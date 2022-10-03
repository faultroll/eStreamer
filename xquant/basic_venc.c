
#include "basic_venc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dlink/dlink_module.h"
#include "nbds/map.h"
#include "xq_conv.h"

typedef struct _bdl_hnd_s_ {
    int refin;
} bdl_hnd_s;

typedef struct _inchn_hnd_s_ {
    struct {
        DL_ID_S venc;
        DL_ID_S in_venc;
        DL_ID_S out;
        // DL_ID_S target;
    };

    struct {
        // xq_venc_param_s p_venc;
        basic_venc_inparam_s p_inchn;
    };
} inchn_hnd_s;

// bdl
static xq_chn_s *__new_inchn(xq_bdl_s *self, const void *param);
static void __delete_chn(xq_bdl_s *self, xq_chn_s *chn);
// bdl_chn in
static int __inchn_create_chn(xq_chn_s *self, const void *param);
static int __inchn_destroy_chn(xq_chn_s *self);
static int __inchn_set_param(xq_chn_s *self, const void *param);
static int __inchn_get_param(xq_chn_s *self, void *param);
static int __inchn_get_status(xq_chn_s *self, void *param);
static int __inchn_custom_ctrl(xq_chn_s *self, const int oper, void *param);
// callback
static int __inchn_outcb_infun(DL_ID_S id, DL_DATA_T data);
static int __inchn_invenccb_infun(DL_ID_S id, DL_DATA_T data);

static map_t *g_chn_map = NULL;
int basic_venc_init(void)
{
    g_chn_map = map_alloc(&MAP_IMPL_LL, NULL);

    return 0;
}
int basic_venc_fini(void)
{
    map_free(g_chn_map);
    g_chn_map = NULL;

    return 0;
}
xq_bdl_s *basic_venc_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_BASIC_VENC;
    c1->priv = h1;

    c1->new_inchn = __new_inchn;
    c1->new_outchn = NULL; // no outchn
    c1->delete_chn = __delete_chn;

    h1->refin = 0;

    return c1;
}
void basic_venc_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    free(h1);
    free(c1);
}
const xq_init_s g_basic_venc = {basic_venc_init, basic_venc_fini, basic_venc_new, basic_venc_delete};

static xq_chn_s *__new_inchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refin) {
        h1->refin++;

        xq_chn_s *c2 = malloc(sizeof(xq_chn_s));
        inchn_hnd_s *h2 = malloc(sizeof(inchn_hnd_s));

        c2->dir = XQ_CHN_IN;
        c2->type = XQ_CHN_NET;
        c2->priv = h2;

        c2->create_chn  = __inchn_create_chn;
        c2->destroy_chn = __inchn_destroy_chn;
        c2->set_param   = __inchn_set_param;
        c2->get_param   = __inchn_get_param;
        c2->get_status  = __inchn_get_status;
        c2->custom_ctrl = __inchn_custom_ctrl;

        c2->link_chn = NULL;
        c2->link_id = (DL_ID_S)DL_ID_EMPTY;
        c2->target_chn = NULL;
        c2->target_id = (DL_ID_S)DL_ID_EMPTY;

        c2->des.mod  = DL_INOUT; // DL_INPUT;
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_BASIC_VENC));
        c2->des.indatatype  = DL_VFRAME;
        c2->des.indatafun   = wrapper_venc_indata;
        c2->des.outdatatype = DL_VSTREAM;
        c2->des.outdatafun  = wrapper_venc_outdata;
        c2->des.freedatafun = wrapper_venc_freedata;

        basic_venc_inparam_s *p = (basic_venc_inparam_s *)param;
        memcpy(&h2->p_inchn, p, sizeof(h2->p_inchn));

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
    inchn_hnd_s *h2 = (inchn_hnd_s *)c2->priv;

    if (h1->refin > 0) {
        free(h2);
        free(c2);

        h1->refin--;
    } else {
        ;
    }
}

static int __inchn_create_chn(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    // venc
    int vencchn;
    xq_venc_param_s *p = (xq_venc_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_venc);
    xq_venc_create(&vencchn, p);
    // LOCK_MTX_UNLOCK(g_mtx_venc);
    h1->venc = (DL_ID_S) {
        .mod = DL_VENC,
        .grp = -1,
        .chn = vencchn,
    };
    // memcpy(&h1->p_venc, p, sizeof(h1->p_venc));
    c1->link_id = h1->venc; // default
    // output
    if (h1->p_inchn.enc_cb.dataprocfun != NULL) {
        int outchn;
        DL_PROC_F indata = __inchn_outcb_infun;
        DL_TYPE_E intype = DL_VSTREAM;
        outchn = dl_create_outchn(indata, intype);
        h1->out = (DL_ID_S) {
            .mod = DL_OUTPUT,
            .grp = -1,
            .chn = outchn,
        };
        // link
        dl_link(h1->venc, h1->out);
        // add to map
        map_val_t val = (map_val_t)c1;
        map_add(g_chn_map, (map_key_t)h1->out.chn, val);

        // output will not change link_id
    }
    // in_venc
    if (h1->p_inchn.raw_cb.dataprocfun != NULL) {
        int outchn;
        /* DL_PROC_F indata = __inchn_invenccb_infun;
        DL_TYPE_E intype = DL_VFRAME;
        outchn = dl_create_outchn(indata, intype); */
        DL_DES_S p_des = {
            .mod = DL_OUTPUT,
            // .name = "BDLB_VENC_RAW", // B-basic, A-advanced
            .inspeed = DL_SLOW,
            .indatatype = DL_VFRAME,
            .indatafun = __inchn_invenccb_infun,
        };
        // see advanced_vmulout.c __outchn_create_chn
        snprintf(p_des.name, sizeof(p_des.name), "BDLB_VENC_RAW");
        outchn = dl_create_chn(&p_des);
        h1->in_venc = (DL_ID_S) {
            .mod = DL_OUTPUT,
            .grp = -1,
            .chn = outchn,
        };
        map_val_t val = (map_val_t)c1;
        map_add(g_chn_map, (map_key_t)h1->in_venc.chn, val);

        c1->link_id = h1->in_venc; // change to in_venc
        c1->des.indatafun = __inchn_invenccb_infun;
    }

    return 0;
}

static int __inchn_destroy_chn(xq_chn_s *self)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    c1->link_id = (DL_ID_S)DL_ID_EMPTY;
    c1->link_chn = NULL;

    if (h1->p_inchn.raw_cb.dataprocfun != NULL) {
        map_remove(g_chn_map, (map_key_t)h1->in_venc.chn);
        dl_destroy_chn(h1->in_venc.chn);
    }
    if (h1->p_inchn.enc_cb.dataprocfun != NULL) {
        // remove from map
        map_remove(g_chn_map, (map_key_t)h1->out.chn);
        // unlink
        dl_unlink(h1->venc, h1->out);
        // output
        dl_destroy_chn(h1->out.chn);
    }
    // venc
    // LOCK_MTX_LOCK(g_mtx_venc);
    xq_venc_destroy(h1->venc.chn);
    // LOCK_MTX_UNLOCK(g_mtx_venc);

    return 0;
}

static int __inchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    xq_venc_param_s *p = (xq_venc_param_s *)param;
    // 1. unlink target
    dl_unlink(c1->link_id, c1->target_id);
    // 2. set param
    xq_venc_setparam(h1->venc.chn, p);
    // 3. link target
    dl_link(c1->link_id, c1->target_id);

    return 0;
}

static int __inchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    xq_venc_param_s *p = (xq_venc_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_venc);
    xq_venc_getparam(h1->venc.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_venc);
    // memcpy(p, &h1->p_venc, sizeof(h1->p_venc));

    return 0;
}

static int __inchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    xq_venc_status_s *p = (xq_venc_status_s *)param;
    // LOCK_MTX_LOCK(g_mtx_venc);
    xq_venc_getstatus(h1->venc.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_venc);

    return 0;
}

static int __inchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    return 0;
}

static int __inchn_outcb_infun(DL_ID_S id, DL_DATA_T data)
{
    map_val_t val;
    if ((val = map_get(g_chn_map, (map_key_t)id.chn)) == DOES_NOT_EXIST)
        return -1;

    xq_chn_s *c1 = (xq_chn_s *)val;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;
    DL_VSTREAMINFO_S *s = (DL_VSTREAMINFO_S *)data;

    DL_PRIV_DATAINFO_U stPrivDataInfo;
    DL_PUB_DATAINFO_S stPubDataInfo;
    memcpy(&stPrivDataInfo.stVStreamInfo, s, sizeof(stPrivDataInfo.stVStreamInfo));
    dlink_cvt_export(&stPrivDataInfo, &stPubDataInfo);
    // no need to check whether h1->p_inchn.enc_cb.dataprocfun is NULL, this callback will not use if it is NULL
    return h1->p_inchn.enc_cb.dataprocfun(h1->p_inchn.enc_cb.opaque, c1, &stPubDataInfo);
}

static int __inchn_invenccb_infun(DL_ID_S id, DL_DATA_T data)
{
    map_val_t val;
    if ((val = map_get(g_chn_map, (map_key_t)id.chn)) == DOES_NOT_EXIST)
        return -1;

    xq_chn_s *c1 = (xq_chn_s *)val;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;
    DL_VFRAMEINFO_S *s = (DL_VFRAMEINFO_S *)data;

    DL_PRIV_DATAINFO_U stPrivDataInfo;
    DL_PUB_DATAINFO_S stPubDataInfo;
    memcpy(&stPrivDataInfo.stVFrameInfo, s, sizeof(stPrivDataInfo.stVFrameInfo));
    dlink_cvt_export(&stPrivDataInfo, &stPubDataInfo);
    h1->p_inchn.raw_cb.dataprocfun(h1->p_inchn.raw_cb.opaque, c1, &stPubDataInfo);
    return wrapper_venc_indata(h1->venc, s);
}
