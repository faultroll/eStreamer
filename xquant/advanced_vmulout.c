
#include "advanced_vmulout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nbds/map.h"
#include "xq_conv.h"

typedef struct _bdl_hnd_s_ {
    int refin;
    int refout;
    xq_bdl_s *vmul;
} bdl_hnd_s;

typedef struct _inchn_hnd_s_ {
    struct {
        xq_bdl_s *vmul;
        xq_chn_s *h_vmulgrp;
    };

    /* struct
    {
        // bdl_hnd_s *parent;
    }; */
} inchn_hnd_s;

typedef struct _outchn_hnd_s_ {
    struct {
        xq_bdl_s *vmul;
        xq_chn_s *h_vmulchn;

        // xq_chn_type_e type; // in p_outchn
        union {
            struct {
                xq_bdl_s *vout;
                xq_chn_s *h_vout;
            } local;
            struct {
                xq_bdl_s *venc;
                xq_chn_s *h_venc;
            } net;
            struct {
                DL_ID_S in;
                // DL_ID_S target;
            } bypass;
        };
    };

    struct {
        // bdl_hnd_s *parent;
        advanced_vmulout_outparam_s p_outchn;
    };
} outchn_hnd_s;

// bdl
static xq_chn_s *__new_inchn(xq_bdl_s *self, const void *param);
static xq_chn_s *__new_outchn(xq_bdl_s *self, const void *param);
static void __delete_chn(xq_bdl_s *self, xq_chn_s *chn);
// bdl_chn in
static int __inchn_create_chn(xq_chn_s *self, const void *param);
static int __inchn_destroy_chn(xq_chn_s *self);
static int __inchn_set_param(xq_chn_s *self, const void *param);
static int __inchn_get_param(xq_chn_s *self, void *param);
static int __inchn_get_status(xq_chn_s *self, void *param);
static int __inchn_custom_ctrl(xq_chn_s *self, const int oper, void *param);
// bdl_chn out
static int __outchn_create_chn(xq_chn_s *self, const void *param);
static int __outchn_destroy_chn(xq_chn_s *self);
static int __outchn_set_param(xq_chn_s *self, const void *param);
static int __outchn_get_param(xq_chn_s *self, void *param);
static int __outchn_get_status(xq_chn_s *self, void *param);
static int __outchn_custom_ctrl(xq_chn_s *self, const int oper, void *param);
// callback
// TODO(lgY): can we merge these funcs into one?
static int __outchn_enccb_transfun(void *opaque, xq_chn_s *chn, /* const */ DL_PUB_DATAINFO_S *data);
static int __outchn_rawcb_transfun(void *opaque, xq_chn_s *chn, /* const */ DL_PUB_DATAINFO_S *data);
static int __outchn_outcb_infun(DL_ID_S id, DL_DATA_T data);

static map_t *g_chn_map = NULL;
int advanced_vmulout_init(void)
{
    g_chn_map = map_alloc(&MAP_IMPL_LL, NULL);

    return 0;
}
int advanced_vmulout_fini(void)
{
    map_free(g_chn_map);
    g_chn_map = NULL;

    return 0;
}
xq_bdl_s *advanced_vmulout_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_ADVANCED_VMULOUT;
    c1->priv = h1;

    c1->new_inchn = __new_inchn;
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refin = 0;
    h1->refout = 0;
    h1->vmul = bundle_new(XQ_BASIC_VMUL, NULL);

    return c1;
}
void advanced_vmulout_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    xquant_delete(h1->vmul);

    free(h1);
    free(c1);
}
const xq_init_s g_advanced_vmulout = {advanced_vmulout_init, advanced_vmulout_fini, advanced_vmulout_new, advanced_vmulout_delete};

static xq_chn_s *__new_inchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refin) {
        h1->refin++;

        xq_chn_s *c2 = malloc(sizeof(xq_chn_s));
        inchn_hnd_s *h2 = malloc(sizeof(inchn_hnd_s));

        c2->dir = XQ_CHN_IN;
        c2->type = XQ_CHN_BYPASS; // TODO(lgY): use basic's type (same as des)
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

        // using h1->h_vmulgrp.des in __inchn_create_chn
        /* c2->des.mod  = DL_INPUT;
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_ADVANCED_VMULOUT));
        c2->des.indatatype  = DL_VFRAME;
        c2->des.indatafun   = wrapper_vmul_in;
        c2->des.outdatatype = DL_TYPE_BUTT;
        c2->des.outdatafun  = NULL;
        c2->des.freedatafun = NULL; */

        // h2->parent = h1;
        h2->vmul = h1->vmul;

        return c2;
    } else {
        return NULL;
    }
}

static xq_chn_s *__new_outchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refin) {
        return NULL;
    } else {
        h1->refout++;

        xq_chn_s *c2 = malloc(sizeof(xq_chn_s));
        outchn_hnd_s *h2 = malloc(sizeof(outchn_hnd_s));

        c2->dir = XQ_CHN_OUT;
        // c2->type = XQ_CHN_NET;
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

        // using h1->h_venc.des in __outchn_create_chn
        /* c2->des.mod  = DL_OUTPUT;
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_ADVANCED_VMULOUT));
        c2->des.indatatype  = DL_TYPE_BUTT;
        c2->des.indatafun   = NULL;
        c2->des.outdatatype = DL_VFRAME;
        c2->des.outdatafun  = wrapper_venc_out;
        c2->des.freedatafun = wrapper_venc_release; */

        // h2->parent = h1;
        h2->vmul = h1->vmul;

        advanced_vmulout_outparam_s *p = (advanced_vmulout_outparam_s *)param;
        memcpy(&h2->p_outchn, p, sizeof(h2->p_outchn));
        c2->type = h2->p_outchn.type;

        return c2;
    }
}

static void __delete_chn(xq_bdl_s *self, xq_chn_s *chn)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;
    xq_chn_s *c2 = (xq_chn_s *)chn;

    if (XQ_CHN_OUT == c2->dir) {
        outchn_hnd_s *h2 = (outchn_hnd_s *)c2->priv;

        free(h2);
        free(c2);

        h1->refout--;
    } else if (XQ_CHN_IN == c2->dir) {
        if (h1->refout > 0) {
            return;
        }

        inchn_hnd_s *h2 = (inchn_hnd_s *)c2->priv;

        free(h2);
        free(c2);

        h1->refin--;
    }
}

static int __inchn_create_chn(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    h1->h_vmulgrp = h1->vmul->new_inchn(h1->vmul, NULL);
    ret |= h1->h_vmulgrp->create_chn(h1->h_vmulgrp, param);

    c1->link_chn = h1->h_vmulgrp;
    c1->link_id = c1->link_chn->link_id;
    memcpy(&c1->des, &c1->link_chn->des, sizeof(c1->des));

    return ret;
}

static int __inchn_destroy_chn(xq_chn_s *self)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    c1->link_id = (DL_ID_S)DL_ID_EMPTY;
    c1->link_chn = NULL;

    int ret = 0;
    ret |= h1->h_vmulgrp->destroy_chn(h1->h_vmulgrp);
    h1->vmul->delete_chn(h1->vmul, h1->h_vmulgrp);

    return ret;
}

static int __inchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_vmulgrp->set_param(h1->h_vmulgrp, param);

    return ret;
}

static int __inchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_vmulgrp->get_param(h1->h_vmulgrp, param);

    return ret;
}

static int __inchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_vmulgrp->get_status(h1->h_vmulgrp, param);

    return ret;
}

static int __inchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    // xq_chn_s *c1 = (xq_chn_s *)self;
    // inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    switch (oper) {
        // ret |= h1->h_vmulgrp->custom_ctrl(h1->h_vmulgrp, oper, param);
        default: {
            return -1;
        }
        break;
    }

    return ret;
}

static int __outchn_create_chn(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    switch (h1->p_outchn.type) {
        case XQ_CHN_LOCAL: {
            xq_vmulchn_param_s p_vmul;
            xq_vout_param_s *p_vout = (xq_vout_param_s *)param;
            xq_conv(DL_VOUT, p_vout, DL_VMUL, &p_vmul);
            h1->h_vmulchn = h1->vmul->new_outchn(h1->vmul, NULL);
            ret |= h1->h_vmulchn->create_chn(h1->h_vmulchn, &p_vmul);
            h1->local.vout = bundle_new(XQ_BASIC_VOUT, NULL);
            h1->local.h_vout = h1->local.vout->new_inchn(h1->local.vout, NULL); // &h1->p_outchn.local
            ret |= h1->local.h_vout->create_chn(h1->local.h_vout, p_vout);
            xquant_link(h1->h_vmulchn, h1->local.h_vout);

            c1->link_chn = h1->local.h_vout;
        }
        break;
        case XQ_CHN_NET: {
            xq_vmulchn_param_s p_vmul;
            xq_venc_param_s *p_venc = (xq_venc_param_s *)param;
            basic_venc_inparam_s p_venc_init = h1->p_outchn.net;
            // must use transparent hnd/func, handle is not same
            if (p_venc_init.enc_cb.dataprocfun != NULL) {
                p_venc_init.enc_cb.opaque = c1;
                p_venc_init.enc_cb.dataprocfun = __outchn_enccb_transfun;
            }
            if (p_venc_init.raw_cb.dataprocfun != NULL) {
                p_venc_init.raw_cb.opaque = c1;
                p_venc_init.raw_cb.dataprocfun = __outchn_rawcb_transfun;
            }
            xq_conv(DL_VENC, p_venc, DL_VMUL, &p_vmul);
            h1->h_vmulchn = h1->vmul->new_outchn(h1->vmul, NULL);
            ret |= h1->h_vmulchn->create_chn(h1->h_vmulchn, &p_vmul);
            h1->net.venc = bundle_new(XQ_BASIC_VENC, NULL);
            h1->net.h_venc = h1->net.venc->new_inchn(h1->net.venc, &p_venc_init); // &h1->p_outchn.net
            ret |= h1->net.h_venc->create_chn(h1->net.h_venc, p_venc);
            xquant_link(h1->h_vmulchn, h1->net.h_venc);

            c1->link_chn = h1->net.h_venc;
        }
        break;
        case XQ_CHN_BYPASS: {
            xq_vmulchn_param_s *p_vmul = (xq_vmulchn_param_s *)param;
            h1->h_vmulchn = h1->vmul->new_outchn(h1->vmul, NULL);
            ret |= h1->h_vmulchn->create_chn(h1->h_vmulchn, p_vmul);
            if (h1->p_outchn.bypass.raw_cb.dataprocfun != NULL) {
                int outchn;
                /* DL_PROC_F indata = __outchn_outcb_infun;
                DL_TYPE_E intype = DL_VFRAME;
                outchn = dl_create_outchn(indata, intype); */
                DL_DES_S p_des = {
                    .mod = DL_OUTPUT,
                    // .name = "BDLA_VMULOUT_BYPASS", // TODO(lgY): too long, find a better way (snprintf)
                    .inspeed = DL_SLOW,
                    .indatatype = DL_VFRAME,
                    .indatafun = __outchn_outcb_infun,
                };
                snprintf(p_des.name, sizeof(p_des.name), "BDLA_VMULOUT_BYPASS");
                outchn = dl_create_chn(&p_des);
                h1->bypass.in = (DL_ID_S) {
                    .mod = DL_OUTPUT,
                    .grp = -1,
                    .chn = outchn,
                };
                // link
                dl_link(h1->h_vmulchn->link_id, h1->bypass.in);
                // add to map
                map_val_t val = (map_val_t)c1;
                map_add(g_chn_map, (map_key_t)h1->bypass.in.chn, val);
            }

            c1->link_chn = h1->h_vmulchn;
        }
        break;
        default: {
            return -1;
        }
        break;
    }

    c1->link_id = c1->link_chn->link_id;
    memcpy(&c1->des, &c1->link_chn->des, sizeof(c1->des));

    return ret;
}

static int __outchn_destroy_chn(xq_chn_s *self)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    c1->link_id = (DL_ID_S)DL_ID_EMPTY;
    c1->link_chn = NULL;

    int ret = 0;
    switch (h1->p_outchn.type) {
        case XQ_CHN_LOCAL: {
            xquant_unlink(h1->h_vmulchn, h1->local.h_vout);
            ret |= h1->local.h_vout->destroy_chn(h1->local.h_vout);
            h1->local.vout->delete_chn(h1->local.vout, h1->local.h_vout);
            xquant_delete(h1->local.vout);
            ret |= h1->h_vmulchn->destroy_chn(h1->h_vmulchn);
            h1->vmul->delete_chn(h1->vmul, h1->h_vmulchn);
        }
        break;
        case XQ_CHN_NET: {
            xquant_unlink(h1->h_vmulchn, h1->net.h_venc);
            ret |= h1->net.h_venc->destroy_chn(h1->net.h_venc);
            h1->net.venc->delete_chn(h1->net.venc, h1->net.h_venc);
            xquant_delete(h1->net.venc);
            ret |= h1->h_vmulchn->destroy_chn(h1->h_vmulchn);
            h1->vmul->delete_chn(h1->vmul, h1->h_vmulchn);
        }
        break;
        case XQ_CHN_BYPASS: {
            if (h1->p_outchn.bypass.raw_cb.dataprocfun != NULL) {
                map_remove(g_chn_map, (map_key_t)h1->bypass.in.chn);
                dl_unlink(h1->h_vmulchn->link_id, h1->bypass.in);
                dl_destroy_chn(h1->bypass.in.chn);
            }
            ret |= h1->h_vmulchn->destroy_chn(h1->h_vmulchn);
            h1->vmul->delete_chn(h1->vmul, h1->h_vmulchn);
        }
        break;
        default: {
            return -1;
        }
        break;
    }

    return ret;
}

static int __outchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    switch (h1->p_outchn.type) {
        case XQ_CHN_LOCAL: {
            xq_vmulchn_param_s p_vmul;
            xq_vout_param_s *p_vout = (xq_vout_param_s *)param;
            xq_conv(DL_VOUT, p_vout, DL_VMUL, &p_vmul);
            ret |= h1->local.h_vout->set_param(h1->local.h_vout, p_vout);
            ret |= h1->h_vmulchn->set_param(h1->h_vmulchn, &p_vmul);
        }
        break;
        case XQ_CHN_NET: {
            xq_vmulchn_param_s p_vmul;
            xq_venc_param_s *p_venc = (xq_venc_param_s *)param;
            xq_conv(DL_VENC, p_venc, DL_VMUL, &p_vmul);
            ret |= h1->net.h_venc->set_param(h1->net.h_venc, p_venc);
            ret |= h1->h_vmulchn->set_param(h1->h_vmulchn, &p_vmul);
        }
        break;
        case XQ_CHN_BYPASS: {
            xq_vmulchn_param_s *p_vmul = (xq_vmulchn_param_s *)param;
            ret |= h1->h_vmulchn->set_param(h1->h_vmulchn, p_vmul);
        }
        break;
        default: {
            return -1;
        }
        break;
    }

    return ret;
}

static int __outchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    switch (h1->p_outchn.type) {
        case XQ_CHN_LOCAL: {
            xq_vout_param_s *p_vout = (xq_vout_param_s *)param;
            ret |= h1->local.h_vout->get_param(h1->local.h_vout, p_vout);
        }
        break;
        case XQ_CHN_NET: {
            xq_venc_param_s *p_venc = (xq_venc_param_s *)param;
            ret |= h1->net.h_venc->get_param(h1->net.h_venc, p_venc);
        }
        break;
        case XQ_CHN_BYPASS: {
            xq_vmulchn_param_s *p_vmul = (xq_vmulchn_param_s *)param;
            ret |= h1->h_vmulchn->get_param(h1->h_vmulchn, p_vmul);
        }
        break;
        default: {
            return -1;
        }
        break;
    }

    return ret;
}

static int __outchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    switch (h1->p_outchn.type) {
        case XQ_CHN_LOCAL: {
            xq_vout_status_s *p_vout = (xq_vout_status_s *)param;
            ret |= h1->local.h_vout->get_status(h1->local.h_vout, p_vout);
        }
        break;
        case XQ_CHN_NET: {
            xq_venc_status_s *p_venc = (xq_venc_status_s *)param;
            ret |= h1->net.h_venc->get_status(h1->net.h_venc, p_venc);
        }
        break;
        case XQ_CHN_BYPASS: {
            xq_vmulchn_status_s *p_vmul = (xq_vmulchn_status_s *)param;
            ret |= h1->h_vmulchn->get_status(h1->h_vmulchn, p_vmul);
        }
        break;
        default: {
            return -1;
        }
        break;
    }

    return ret;
}

static int __outchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    // TODO(lgY): adv bundle only dispatch oper, maybe we need a table
    // TODO(lgY): if business bundle add an oper, advanced/basic bundle both need to add an oper, sucks
    // NOTE(lgY): see advanced_amulout.c, maybe it's a good idea if we don't need to ctrl basic_vmul
    int ret = 0;
    switch (oper) {
        case XQ_OPER_UPDATE_OSD:
        case XQ_OPER_FORCE_IFRAME: {
            // only net support this
            if (XQ_CHN_NET == h1->p_outchn.type) {
                ret |= h1->net.h_venc->custom_ctrl(h1->net.h_venc, oper, param);
            } else {
                ret |= -1;
            }
        }
        break;
        default: {
            return -1;
        }
        break;
    }

    return ret;
}

static int __outchn_enccb_transfun(void *opaque, xq_chn_s *chn, /* const */ DL_PUB_DATAINFO_S *data)
{
    xq_chn_s *c1 = (xq_chn_s *)opaque;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    return h1->p_outchn.net.enc_cb.dataprocfun(h1->p_outchn.net.enc_cb.opaque, c1, data);
}

static int __outchn_rawcb_transfun(void *opaque, xq_chn_s *chn, /* const */ DL_PUB_DATAINFO_S *data)
{
    xq_chn_s *c1 = (xq_chn_s *)opaque;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    return h1->p_outchn.net.raw_cb.dataprocfun(h1->p_outchn.net.raw_cb.opaque, c1, data);
}

static int __outchn_outcb_infun(DL_ID_S id, DL_DATA_T data)
{
    map_val_t val;
    if ((val = map_get(g_chn_map, (map_key_t)id.chn)) == DOES_NOT_EXIST)
        return -1;

    xq_chn_s *c1 = (xq_chn_s *)val;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;
    DL_VFRAMEINFO_S *s = (DL_VFRAMEINFO_S *)data;

    DL_PRIV_DATAINFO_U stPrivDataInfo;
    DL_PUB_DATAINFO_S stPubDataInfo;
    memcpy(&stPrivDataInfo.stVFrameInfo, s, sizeof(stPrivDataInfo.stVFrameInfo));
    dlink_cvt_export(&stPrivDataInfo, &stPubDataInfo);
    return h1->p_outchn.bypass.raw_cb.dataprocfun(h1->p_outchn.bypass.raw_cb.opaque, c1, &stPubDataInfo);
}
