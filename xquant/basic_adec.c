
#include "basic_adec.h"
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
        DL_ID_S adec;
        DL_ID_S in;
        // DL_ID_S target;
    };

    struct {
        // xq_adec_param_s p_adec;
        basic_adec_outparam_s p_inchn;
        // playback
        bool resend;
        DL_PRIV_DATAINFO_U last;
        void *buf;
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

#define ADEC_BUFSZ  (1 * 1024 * 1024)

static map_t *g_chn_map = NULL;
int basic_adec_init(void)
{
    g_chn_map = map_alloc(&MAP_IMPL_LL, NULL);

    return 0;
}
int basic_adec_fini(void)
{
    map_free(g_chn_map);
    g_chn_map = NULL;

    return 0;
}
xq_bdl_s *basic_adec_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_BASIC_ADEC;
    c1->priv = h1;

    c1->new_inchn = NULL; // no inchn
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refout = 0;

    return c1;
}
void basic_adec_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    free(h1);
    free(c1);
}
const xq_init_s g_basic_adec = {basic_adec_init, basic_adec_fini, basic_adec_new, basic_adec_delete};

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
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_BASIC_ADEC));
        c2->des.indatatype  = DL_TYPE_BUTT;
        c2->des.indatafun   = NULL;
        c2->des.outdatatype = DL_AFRAME;
        c2->des.outdatafun  = __outchn_incb_outfun;
        c2->des.freedatafun = __outchn_incb_freefun;

        basic_adec_outparam_s *p = (basic_adec_outparam_s *)param;
        memcpy(&h2->p_inchn, p, sizeof(h2->p_inchn));
        h2->resend = false;
        h2->last.stAStreamInfo = (DL_ASTREAMINFO_S)DL_DATA_EMPTY;
        h2->buf = malloc(ADEC_BUFSZ);

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

    // adec
    int adecchn;
    xq_adec_param_s *p = (xq_adec_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_adec);
    xq_adec_create(&adecchn, p);
    // LOCK_MTX_UNLOCK(g_mtx_adec);
    h1->adec = (DL_ID_S) {
        .mod = DL_ADEC,
        .grp = -1,
        .chn = adecchn,
    };
    // memcpy(&h1->p_adec, p, sizeof(h1->p_adec));
    // input
    if (h1->p_inchn.dec_cb.dataprocfun != NULL) {
        int inchn;
        DL_PROC_F outdata = __outchn_incb_outfun;
        DL_PROC_F freedata = __outchn_incb_freefun;
        DL_TYPE_E outtype = DL_AFRAME;
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
    } else {
        c1->link_id = h1->adec;
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
    // adec
    // LOCK_MTX_LOCK(g_mtx_adec);
    xq_adec_destroy(h1->adec.chn);
    // LOCK_MTX_UNLOCK(g_mtx_adec);

    return 0;
}

static int __outchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_adec_param_s *p = (xq_adec_param_s *)param;
    // 1. unlink target
    dl_unlink(c1->link_id, c1->target_id);
    // 2. set param
    xq_adec_setparam(h1->adec.chn, p);
    // 3. link target
    dl_link(c1->link_id, c1->target_id);

    return 0;
}

static int __outchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_adec_param_s *p = (xq_adec_param_s *)param;
    // LOCK_MTX_UNLOCK(g_mtx_adec);
    xq_adec_getparam(h1->adec.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_adec);
    // memcpy(p, &h1->p_adec, sizeof(h1->p_adec));

    return 0;
}

static int __outchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_adec_status_s *p = (xq_adec_status_s *)param;
    // LOCK_MTX_LOCK(g_mtx_adec);
    xq_adec_getstatus(h1->adec.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_adec);

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
    if ((val = map_get(g_chn_map, (map_key_t)id.chn)) == DOES_NOT_EXIST)
        return -1;

    xq_chn_s *c1 = (xq_chn_s *)val;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;
    DL_AFRAMEINFO_S *s = (DL_AFRAMEINFO_S *)data;

    if (!h1->p_inchn.playback) {
        DL_PUB_DATAINFO_S stPubDataInfo;
        DL_PRIV_DATAINFO_U stPrivDataInfo;
        stPubDataInfo.enDataType = DL_ASTREAM;
        // stPubDataInfo.stAStreamInfo.enEncType = h1->p_adec.enType;
        stPubDataInfo.pVirAddr = h1->buf;
        stPubDataInfo.ipPhyAddr = (uintptr_t)NULL;
        stPubDataInfo.u32Len = ADEC_BUFSZ;
        if (!h1->p_inchn.dec_cb.dataprocfun(h1->p_inchn.dec_cb.opaque, c1, &stPubDataInfo)) {
            stPubDataInfo.u32Len = (stPubDataInfo.u32Len > ADEC_BUFSZ) ? ADEC_BUFSZ : stPubDataInfo.u32Len;
            dlink_cvt_import(&stPubDataInfo, &stPrivDataInfo);
            wrapper_adec_indata(h1->adec, &stPrivDataInfo.stAStreamInfo);
            dlink_cvt_release(&stPrivDataInfo);
        }
    } else {
        if (!h1->resend) {
            DL_PUB_DATAINFO_S stPubDataInfo;
            DL_PRIV_DATAINFO_U stPrivDataInfo;
            stPubDataInfo.enDataType = DL_ASTREAM;
            // stPubDataInfo.stAStreamInfo.enEncType = h1->p_adec.enType;
            stPubDataInfo.pVirAddr = h1->buf;
            stPubDataInfo.ipPhyAddr = (uintptr_t)NULL;
            stPubDataInfo.u32Len = ADEC_BUFSZ;
            if (!h1->p_inchn.dec_cb.dataprocfun(h1->p_inchn.dec_cb.opaque, c1, &stPubDataInfo)) {
                stPubDataInfo.u32Len = (stPubDataInfo.u32Len > ADEC_BUFSZ) ? ADEC_BUFSZ : stPubDataInfo.u32Len;
                dlink_cvt_import(&stPubDataInfo, &stPrivDataInfo);
                if (!wrapper_adec_indata(h1->adec, &stPrivDataInfo.stAStreamInfo)) {
                    dlink_cvt_release(&stPrivDataInfo);
                    h1->resend = false;
                } else {
                    h1->last = stPrivDataInfo;
                    h1->resend = true;
                }
            }
        } else {
            DL_PRIV_DATAINFO_U stPrivDataInfo;
            stPrivDataInfo = h1->last;
            if (!wrapper_adec_indata(h1->adec, &stPrivDataInfo.stAStreamInfo)) {
                dlink_cvt_release(&stPrivDataInfo);
                h1->last.stAStreamInfo = (DL_ASTREAMINFO_S)DL_DATA_EMPTY;
                h1->resend = false;
            } else {
                // h1->last = stPrivDataInfo;
                // h1->resend = true;
            }
        }
    }

    return wrapper_adec_outdata(h1->adec, s);
}

static int __outchn_incb_freefun(DL_ID_S id, DL_DATA_T data)
{
    map_val_t val;
    if ((val = map_get(g_chn_map, (map_key_t)id.chn))  == DOES_NOT_EXIST)
        return -1;

    xq_chn_s *c1 = (xq_chn_s *)val;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;
    DL_AFRAMEINFO_S *s = (DL_AFRAMEINFO_S *)data;

    return wrapper_adec_freedata(h1->adec, s);
}
