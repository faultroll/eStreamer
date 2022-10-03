
#include "basic_vdec.h"
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
        DL_ID_S vdec;
        // DL_ID_S out_vdec;
        DL_ID_S in;
        // DL_ID_S target;
    };

    struct {
        // xq_vdec_param_s p_vdec;
        basic_vdec_outparam_s p_inchn;
        // playback
        bool resend;
        DL_PRIV_DATAINFO_U last;
        void *buf;
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

#define VDEC_BUFSZ  (1920 * 1080 * 3 >> 1)

static map_t *g_chn_map = NULL;
int basic_vdec_init(void)
{
    g_chn_map = map_alloc(&MAP_IMPL_LL, NULL);

    return 0;
}
int basic_vdec_fini(void)
{
    map_free(g_chn_map);
    g_chn_map = NULL;

    return 0;
}
xq_bdl_s *basic_vdec_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_BASIC_VDEC;
    c1->priv = h1;

    c1->new_inchn = NULL; // no inchn
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refout = 0;

    return c1;
}
void basic_vdec_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    free(h1);
    free(c1);
}
const xq_init_s g_basic_vdec = {basic_vdec_init, basic_vdec_fini, basic_vdec_new, basic_vdec_delete};

static xq_chn_s *__new_outchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refout) {
        h1->refout++;

        xq_chn_s *c2 = malloc(sizeof(xq_chn_s));
        outchn_hnd_s *h2 = malloc(sizeof(outchn_hnd_s));

        c2->dir = XQ_CHN_OUT;
        c2->type = XQ_CHN_NET;
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
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_BASIC_VDEC));
        c2->des.indatatype  = DL_TYPE_BUTT;
        c2->des.indatafun   = NULL;
        c2->des.outdatatype = DL_VFRAME;
        c2->des.outdatafun  = __outchn_incb_outfun;
        c2->des.freedatafun = __outchn_incb_freefun;

        basic_vdec_outparam_s *p = (basic_vdec_outparam_s *)param;
        memcpy(&h2->p_inchn, p, sizeof(h2->p_inchn));
        h2->resend = false;
        h2->last.stVStreamInfo = (DL_VSTREAMINFO_S)DL_DATA_EMPTY;
        h2->buf = malloc(VDEC_BUFSZ);
        h2->op2.enable = false;
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
        free(h2->buf);

        free(h2);
        free(c2);

        h1->refout--;
    } else {
        ;
    }
}

static int __outchn_create_chn(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    // vdec
    int vdecchn;
    xq_vdec_param_s *p = (xq_vdec_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_vdec);
    xq_vdec_create(&vdecchn, p);
    // LOCK_MTX_UNLOCK(g_mtx_vdec);
    h1->vdec = (DL_ID_S) {
        .mod = DL_VDEC,
        .grp = -1,
        .chn = vdecchn,
    };
    // memcpy(&h1->p_vdec, p, sizeof(h1->p_vdec));
    c1->link_id = h1->vdec;
    // input
    if (h1->p_inchn.dec_cb.dataprocfun != NULL) {
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

        c1->link_id = h1->in; // change to input
    }

    return 0;
}

static int __outchn_destroy_chn(xq_chn_s *self)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    c1->link_id = (DL_ID_S)DL_ID_EMPTY;
    c1->link_chn = NULL;

    if (h1->p_inchn.dec_cb.dataprocfun != NULL) {
        // remove from map
        map_remove(g_chn_map, (map_key_t)h1->in.chn);
        // input
        dl_destroy_chn(h1->in.chn);
    }
    // vdec
    // LOCK_MTX_LOCK(g_mtx_vdec);
    xq_vdec_destroy(h1->vdec.chn);
    // LOCK_MTX_UNLOCK(g_mtx_vdec);
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

    xq_vdec_param_s *p = (xq_vdec_param_s *)param;
    // 1. unlink target
    dl_unlink(c1->link_id, c1->target_id);
    // 2. set param
    xq_vdec_setparam(h1->vdec.chn, p);
    // 3. link target
    dl_link(c1->link_id, c1->target_id);

    return 0;
}

static int __outchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_vdec_param_s *p = (xq_vdec_param_s *)param;
    // LOCK_MTX_UNLOCK(g_mtx_vdec);
    xq_vdec_getparam(h1->vdec.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_vdec);

    return 0;
}

static int __outchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    // TODO(lgY): return current enctype
    //  1. not using status param, bundle wrap it ownself(current layer)
    //  (in-use)2. return in callback, move this logic into upper layer
    //  (in-use)3. add this status, move this logic into lower layer
    xq_vdec_status_s *p = (xq_vdec_status_s *)param;
    // LOCK_MTX_LOCK(g_mtx_vdec);
    xq_vdec_getstatus(h1->vdec.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_vdec);

    return 0;
}

static int __outchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    return 0;
}

static int __outchn_incb_outfun(DL_ID_S id, DL_DATA_T data)
{
    map_val_t val;
    if ((val = map_get(g_chn_map, (map_key_t)id.chn))  == DOES_NOT_EXIST)
        return -1;

    xq_chn_s *c1 = (xq_chn_s *)val;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;
    DL_VFRAMEINFO_S *s = (DL_VFRAMEINFO_S *)data;

    if (h1->op2.enable) {
        uint64_t u64CurPts;
        u64CurPts = info_timeus();
        if ((u64CurPts - h1->pts) < h1->interval) {
            return -1;
        } else {
            h1->pts = u64CurPts;
            memcpy(s, &h1->usrpic, sizeof(h1->usrpic));

            return 0;
        }
    } else {
        if (!h1->p_inchn.playback) {
            DL_PUB_DATAINFO_S stPubDataInfo;
            DL_PRIV_DATAINFO_U stPrivDataInfo;
            stPubDataInfo.enDataType = DL_VSTREAM;
            // stPubDataInfo.stVStreamInfo.enEncType = h1->p_vdec.enType;
            stPubDataInfo.pVirAddr = h1->buf;
            stPubDataInfo.ipPhyAddr = (uintptr_t)NULL;
            stPubDataInfo.u32Len = VDEC_BUFSZ;
            if (!h1->p_inchn.dec_cb.dataprocfun(h1->p_inchn.dec_cb.opaque, c1, &stPubDataInfo)) {
                // TODO(lgY): handle exceed VDEC_BUFSZ properly
                stPubDataInfo.u32Len = (stPubDataInfo.u32Len > VDEC_BUFSZ) ? VDEC_BUFSZ : stPubDataInfo.u32Len;
                dlink_cvt_import(&stPubDataInfo, &stPrivDataInfo);
                wrapper_vdec_indata(h1->vdec, &stPrivDataInfo.stVStreamInfo);
                dlink_cvt_release(&stPrivDataInfo);
            }
        } else {
            if (!h1->resend) {
                // same as playback
                DL_PUB_DATAINFO_S stPubDataInfo;
                DL_PRIV_DATAINFO_U stPrivDataInfo;
                stPubDataInfo.enDataType = DL_VSTREAM;
                // stPubDataInfo.stVStreamInfo.enEncType = h1->p_vdec.enType;
                stPubDataInfo.pVirAddr = h1->buf;
                stPubDataInfo.ipPhyAddr = (uintptr_t)NULL;
                stPubDataInfo.u32Len = VDEC_BUFSZ;
                if (!h1->p_inchn.dec_cb.dataprocfun(h1->p_inchn.dec_cb.opaque, c1, &stPubDataInfo)) {
                    stPubDataInfo.u32Len = (stPubDataInfo.u32Len > VDEC_BUFSZ) ? VDEC_BUFSZ : stPubDataInfo.u32Len;
                    dlink_cvt_import(&stPubDataInfo, &stPrivDataInfo);
                    if (!wrapper_vdec_indata(h1->vdec, &stPrivDataInfo.stVStreamInfo)) {
                        dlink_cvt_release(&stPrivDataInfo);
                        h1->resend = false;
                    } else {
                        // store current data, not release until resend success
                        h1->last = stPrivDataInfo;
                        h1->resend = true;
                    }
                }
            } else {
                DL_PRIV_DATAINFO_U stPrivDataInfo;
                stPrivDataInfo = h1->last;
                if (!wrapper_vdec_indata(h1->vdec, &stPrivDataInfo.stVStreamInfo)) {
                    // resend success, release current data
                    dlink_cvt_release(&stPrivDataInfo);
                    h1->last.stVStreamInfo = (DL_VSTREAMINFO_S)DL_DATA_EMPTY;
                    h1->resend = false;
                } else {
                    // do nothing
                    // h1->last = stPrivDataInfo;
                    // h1->resend = true;
                }
            }
        }

        // video is frame-mode, pts is proper
        return wrapper_vdec_outdata(h1->vdec, s);
    }
}

static int __outchn_incb_freefun(DL_ID_S id, DL_DATA_T data)
{
    map_val_t val;
    if ((val = map_get(g_chn_map, (map_key_t)id.chn))  == DOES_NOT_EXIST)
        return -1;

    xq_chn_s *c1 = (xq_chn_s *)val;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;
    DL_VFRAMEINFO_S *s = (DL_VFRAMEINFO_S *)data;

    // see |basic_vin.c|
    /* if (h1->op2.enable)
    {
        // do nothing
        return 0;
    }
    else */
    {
        return wrapper_vdec_freedata(h1->vdec, s);
    }
}
