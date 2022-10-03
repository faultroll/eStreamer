
#include "basic_vin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dlink/dlink_module.h"
#include "nbds/map.h"
#include "xq_conv.h"

typedef struct _bdl_hnd_s_ {
    int refout;
} bdl_hnd_s;

typedef struct _outchn_hnd_s_ {
    struct {
        DL_ID_S vin;
        DL_ID_S in;
        // DL_ID_S target;
    };

    struct {
        // xq_vin_param_s p_vin; // should store static-param diff dynamic-param
        // XQ_OPER_USRPIC
        xq_usrpic_set_param_s op1;
        xq_usrpic_switch_param_s op2;
        uint64_t pts;
        uint64_t interval;
        DL_VFRAMEINFO_S usrpic;
    };
} outchn_hnd_s;

// bdl
static xq_chn_s *__new_outchn(xq_bdl_s *self, const void *param);
static void __delete_chn(xq_bdl_s *self, xq_chn_s *chn);
// bdl_chn out
static int __outchn_create_chn(xq_chn_s *self, const void *param);
static int __outchn_destroy_chn(xq_chn_s *self);
static int __outchn_set_param(xq_chn_s *self, const void *param);
static int __outchn_get_param(xq_chn_s *self, void *param);
static int __outchn_get_status(xq_chn_s *self, void *param);
static int __outchn_custom_ctrl(xq_chn_s *self, const int oper, void *param);
// callback
static int __outchn_incb_outfun(DL_ID_S id, DL_DATA_T data);
static int __outchn_incb_freefun(DL_ID_S id, DL_DATA_T data);

static map_t *g_chn_map = NULL;
int basic_vin_init(void)
{
    g_chn_map = map_alloc(&MAP_IMPL_LL, NULL);

    return 0;
}
int basic_vin_fini(void)
{
    map_free(g_chn_map);
    g_chn_map = NULL;

    return 0;
}
xq_bdl_s *basic_vin_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_BASIC_VIN; // bundle do this, think it's bundle's attribute
    c1->priv = h1;

    c1->new_inchn = NULL; // no inchn
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refout = 0;

    return c1;
}
void basic_vin_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    free(h1);
    free(c1);
}
const xq_init_s g_basic_vin = {basic_vin_init, basic_vin_fini, basic_vin_new, basic_vin_delete};

static xq_chn_s *__new_outchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refout) {
        h1->refout++; // TODO(lgY): atomic_add (basic_vin: 1-bdl-1-outchn)

        xq_chn_s *c2 = malloc(sizeof(xq_chn_s));
        outchn_hnd_s *h2 = malloc(sizeof(outchn_hnd_s));

        c2->dir = XQ_CHN_OUT;
        c2->type = XQ_CHN_LOCAL;
        c2->priv = h2;

        c2->create_chn  = __outchn_create_chn;
        c2->destroy_chn = __outchn_destroy_chn;
        c2->set_param   = __outchn_set_param;
        c2->get_param   = __outchn_get_param;
        c2->get_status  = __outchn_get_status;
        c2->custom_ctrl = __outchn_custom_ctrl;

        c2->link_chn = NULL;
        c2->link_id = (DL_ID_S)DL_ID_EMPTY;
        c2->target_chn = NULL;
        c2->target_id = (DL_ID_S)DL_ID_EMPTY;

        c2->des.mod  = DL_OUTPUT;
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_BASIC_VIN));
        c2->des.indatatype  = DL_TYPE_BUTT;
        c2->des.indatafun   = NULL;
        c2->des.outdatatype = DL_VFRAME;
        c2->des.outdatafun  = __outchn_incb_outfun;
        c2->des.freedatafun = __outchn_incb_freefun;

        h2->op2.enable = false; // default disable
        h2->usrpic = (DL_VFRAMEINFO_S)DL_DATA_EMPTY;

        return c2;
    } else {
        return NULL;
    }
}
static void __delete_chn(xq_bdl_s *self, xq_chn_s *chn)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;
    xq_chn_s *c2 = (xq_chn_s *)chn;
    outchn_hnd_s *h2 = (outchn_hnd_s *)c2->priv;

    if (h1->refout > 0) {
        h1->refout--;

        free(h2);
        free(c2);
    } else {
        ;
    }
}

static int __outchn_create_chn(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    // vin
    int vinchn;
    xq_vin_param_s *p = (xq_vin_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_vin);
    xq_vin_create(&vinchn, p);
    // LOCK_MTX_UNLOCK(g_mtx_vin);
    h1->vin = (DL_ID_S) {
        .mod = DL_VIN,
        .grp = -1,
        .chn = vinchn,
    };
    // memcpy(&h1->p_vin, p, sizeof(h1->p_vin));
    // input
    int inchn;
    DL_PROC_F outdata = __outchn_incb_outfun;
    DL_PROC_F freedata = __outchn_incb_freefun;
    DL_TYPE_E outtype = DL_VFRAME;
    inchn = dl_create_inchn(outdata, outtype, freedata);
    h1->in = (DL_ID_S) {
        .mod = DL_INPUT,
        .grp = -1,
        .chn = inchn,
    };
    // add to map
    map_val_t val = (map_val_t)c1;
    map_add(g_chn_map, (map_key_t)h1->in.chn, val);

    c1->link_id = h1->in;

    return 0;
}
static int __outchn_destroy_chn(xq_chn_s *self)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    c1->link_id = (DL_ID_S)DL_ID_EMPTY;
    c1->link_chn = NULL;

    // remove from map
    map_remove(g_chn_map, (map_key_t)h1->in.chn);
    // input
    dl_destroy_chn(h1->in.chn);
    // vin
    // LOCK_MTX_LOCK(g_mtx_vin);
    xq_vin_destroy(h1->vin.chn);
    // LOCK_MTX_UNLOCK(g_mtx_vin);
    // usrpic
    DL_PRIV_DATAINFO_U stPrivDataInfo;
    stPrivDataInfo.stVFrameInfo = h1->usrpic;
    dlink_cvt_release(&stPrivDataInfo);

    return 0;
}
static int __outchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_vin_param_s *p = (xq_vin_param_s *)param;
    // NOTE(lgY): all params should be 'dynamic' (inner destroy-create)
#if 0
    if (1) { // static-param changes
        // 1. unlink vin-target
        dl_unlink(c1->link_id, c1->target_id);
        // 2. destroy vin
        c1->destroy_chn(c1);
        // 3. create vin
        c1->create_chn(c1, p);
        // 4. link vin-target
        dl_link(c1->link_id, c1->target_id);
    } else { // dynamic-param changes
        // LOCK_MTX_UNLOCK(g_mtx_vin);
        xq_vin_setparam(h1->vin.chn, p);
        // LOCK_MTX_UNLOCK(g_mtx_vin);
    }
#else // 0
    // 1. unlink target
    dl_unlink(c1->link_id, c1->target_id);
    // 2. set param
    xq_vin_setparam(h1->vin.chn, p);
    // 3. link target
    dl_link(c1->link_id, c1->target_id);
#endif // 0

    return 0;
}
static int __outchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_vin_param_s *p = (xq_vin_param_s *)param;
    // memcpy(p, &h1->p_vin, sizeof(h1->p_vin)); // static-param
    // LOCK_MTX_LOCK(g_mtx_vin);
    xq_vin_getparam(h1->vin.chn, p); // dynamic-param
    // LOCK_MTX_UNLOCK(g_mtx_vin);

    return 0;
}
static int __outchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_vin_status_s *p = (xq_vin_status_s *)param;
    // LOCK_MTX_LOCK(g_mtx_vin);
    xq_vin_getstatus(h1->vin.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_vin);

    return 0;
}
static int __outchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    switch (oper) {
        case XQ_OPER_USRPIC_SET: {
            // release whatever
            DL_PRIV_DATAINFO_U stPrivDataInfo1;
            stPrivDataInfo1.stVFrameInfo = h1->usrpic;
            dlink_cvt_release(&stPrivDataInfo1);
            xq_usrpic_set_param_s *p = (xq_usrpic_set_param_s *)param;
            /* p->stUsrPic.enDataType = DL_VFRAME;
            p->stUsrPic.u64Pts = 0;
            p->stUsrPic.u32Seq = 0; */ // let user do this
            memcpy(&h1->op1, p, sizeof(h1->op1));
            // timestamp
            h1->pts = info_timeus();
            h1->interval = (uint64_t)(1000 * 1000 * 1 / h1->op1.frame_rate);
            // usrin
            DL_PRIV_DATAINFO_U stPrivDataInfo2;
            dlink_cvt_import(&h1->op1.stUsrPic, &stPrivDataInfo2);
            h1->usrpic = stPrivDataInfo2.stVFrameInfo;
        }
        break;
        case XQ_OPER_USRPIC_SWITCH: {
            xq_usrpic_switch_param_s *p = (xq_usrpic_switch_param_s *)param;
            memcpy(&h1->op2, p, sizeof(h1->op2));
        }
        break;
        default: // not support
            break;
    }

    return 0;
}

static int __outchn_incb_outfun(DL_ID_S id, DL_DATA_T data)
{
    map_val_t val;
    if ((val = map_get(g_chn_map, (map_key_t)id.chn)) == DOES_NOT_EXIST)
        return -1;

    xq_chn_s *c1 = (xq_chn_s *)val;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;
    DL_VFRAMEINFO_S *s = (DL_VFRAMEINFO_S *)data;

    if (h1->op2.enable) {
        // generate usr pic
        uint64_t u64CurPts = info_timeus();
        if ((u64CurPts - h1->pts) < h1->interval) {
            return -1;
        } else {
            h1->pts = u64CurPts;
            memcpy(s, &h1->usrpic, sizeof(h1->usrpic));

            return 0;
        }
    } else {
        return wrapper_vin_outdata(h1->vin, s);
    }
}
static int __outchn_incb_freefun(DL_ID_S id, DL_DATA_T data)
{
    map_val_t val;
    if ((val = map_get(g_chn_map, (map_key_t)id.chn)) == DOES_NOT_EXIST)
        return -1;

    xq_chn_s *c1 = (xq_chn_s *)val;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;
    DL_VFRAMEINFO_S *s = (DL_VFRAMEINFO_S *)data;

    // cannot change directly
    // if |set_param| changes after |infun| before |freefun|, user |outdata| from VIN but not freed
    /* if (h1->op2.enable)
    {
        // do nothing
        return 0;
    }
    else */
    {
        return wrapper_vin_freedata(h1->vin, s);
    }
}
