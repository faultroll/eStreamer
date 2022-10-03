
#include "business_venc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xq_conv.h"

typedef struct _bdl_hnd_s_ {
    int refin;
    int refout;
    xq_bdl_s *vmulout;
} bdl_hnd_s;

typedef struct _inchn_hnd_s_ {
    struct {
        xq_bdl_s *vin;
        xq_chn_s *h_vin;
        xq_bdl_s *vmulout;
        xq_chn_s *h_vmuloutgrp;
    };

    /* struct
    {
        // bdl_hnd_s *parent;
    }; */
} inchn_hnd_s;

typedef struct _outchn_hnd_s_ {
    struct {
        xq_bdl_s *vmulout;
        xq_chn_s *h_vmuloutchn;
    };

    struct {
        // bdl_hnd_s *parent;
        business_venc_outparam_s p_outchn;
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
static int __outchn_enccb_transfun(void *opaque, xq_chn_s *chn, /* const */ DL_PUB_DATAINFO_S *data);
static int __outchn_rawcb_transfun(void *opaque, xq_chn_s *chn, /* const */ DL_PUB_DATAINFO_S *data);

int business_venc_init(void)
{
    return 0;
}
int business_venc_fini(void)
{
    return 0;
}
xq_bdl_s *business_venc_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_BUSINESS_VENC;
    c1->priv = h1;

    c1->new_inchn = __new_inchn;
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refin = 0;
    h1->refout = 0;
    h1->vmulout = bundle_new(XQ_ADVANCED_VMULOUT, NULL);

    return c1;
}
void business_venc_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    xquant_delete(h1->vmulout);

    free(h1);
    free(c1);
}
const xq_init_s g_business_venc = {business_venc_init, business_venc_fini, business_venc_new, business_venc_delete};

static xq_chn_s *__new_inchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refin) {
        h1->refin++;

        xq_chn_s *c2 = malloc(sizeof(xq_chn_s));
        inchn_hnd_s *h2 = malloc(sizeof(inchn_hnd_s));

        c2->dir = XQ_CHN_IN;
        c2->type = XQ_CHN_LOCAL;
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

        // h2->parent = h1;
        h2->vmulout = h1->vmulout;

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

        // h2->parent = h1;
        h2->vmulout = h1->vmulout;

        business_venc_outparam_s *p = (business_venc_outparam_s *)param;
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
    xq_vin_param_s *p_vin = (xq_vin_param_s *)param;
    xq_vmulgrp_param_s p_vmulout;
    xq_conv(DL_VIN, p_vin, DL_VMUL, &p_vmulout);
    // advanced_vin
    advanced_vin_outparam_s p_vin_init;
    p_vin_init.type = XQ_CHN_LOCAL;
    h1->vin = bundle_new(XQ_ADVANCED_VIN, NULL);
    h1->h_vin = h1->vin->new_outchn(h1->vin, &p_vin_init);
    ret |= h1->h_vin->create_chn(h1->h_vin, p_vin);
    // advanced_vmulout
    h1->h_vmuloutgrp = h1->vmulout->new_inchn(h1->vmulout, NULL);
    ret |= h1->h_vmuloutgrp->create_chn(h1->h_vmuloutgrp, &p_vmulout);
    xquant_link(h1->h_vin, h1->h_vmuloutgrp);

    c1->link_chn = h1->h_vmuloutgrp;
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
    xquant_unlink(h1->h_vin, h1->h_vmuloutgrp);
    ret |= h1->h_vmuloutgrp->destroy_chn(h1->h_vmuloutgrp);
    h1->vmulout->delete_chn(h1->vmulout, h1->h_vmuloutgrp);
    ret |= h1->h_vin->destroy_chn(h1->h_vin);
    h1->vin->delete_chn(h1->vin, h1->h_vin);
    xquant_delete(h1->vin);

    return ret;
}

static int __inchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    xq_vin_param_s *p_vin = (xq_vin_param_s *)param;
    xq_vmulgrp_param_s p_vmulout;
    xq_conv(DL_VIN, p_vin, DL_VMUL, &p_vmulout);
    ret |= h1->h_vmuloutgrp->set_param(h1->h_vmuloutgrp, &p_vmulout);
    ret |= h1->h_vin->set_param(h1->h_vin, p_vin);

    return ret;
}

static int __inchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    xq_vin_param_s *p_vin = (xq_vin_param_s *)param;
    ret |= h1->h_vin->get_param(h1->h_vin, p_vin);

    return ret;
}

static int __inchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    xq_vin_status_s *p_vin = (xq_vin_status_s *)param;
    ret |= h1->h_vin->get_status(h1->h_vin, p_vin);

    return ret;
}

static int __inchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    switch (oper) {
        case XQ_OPER_USRPIC_SET:
        case XQ_OPER_USRPIC_SWITCH: {
            ret |= h1->h_vin->custom_ctrl(h1->h_vin, oper, param);
        }
        break;
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
    business_venc_outparam_s p_vmulout_init = h1->p_outchn;
    if (XQ_CHN_NET == h1->p_outchn.type) {
        if (p_vmulout_init.net.enc_cb.dataprocfun != NULL) {
            p_vmulout_init.net.enc_cb.opaque = c1;
            p_vmulout_init.net.enc_cb.dataprocfun = __outchn_enccb_transfun;
        }
        if (p_vmulout_init.net.raw_cb.dataprocfun != NULL) {
            p_vmulout_init.net.raw_cb.opaque = c1;
            p_vmulout_init.net.raw_cb.dataprocfun = __outchn_rawcb_transfun;
        }
    }
    h1->h_vmuloutchn = h1->vmulout->new_outchn(h1->vmulout, &p_vmulout_init);
    ret |= h1->h_vmuloutchn->create_chn(h1->h_vmuloutchn, param);

    c1->link_chn = h1->h_vmuloutchn;
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

    ret |= h1->h_vmuloutchn->destroy_chn(h1->h_vmuloutchn);
    h1->vmulout->delete_chn(h1->vmulout, h1->h_vmuloutchn);

    return ret;
}

static int __outchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_vmuloutchn->set_param(h1->h_vmuloutchn, param);

    return ret;
}

static int __outchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_vmuloutchn->get_param(h1->h_vmuloutchn, param);

    return ret;
}

static int __outchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_vmuloutchn->get_status(h1->h_vmuloutchn, param);

    return ret;
}

static int __outchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    switch (oper) {
        case XQ_OPER_UPDATE_OSD:
        case XQ_OPER_FORCE_IFRAME: {
            ret |= h1->h_vmuloutchn->custom_ctrl(h1->h_vmuloutchn, oper, param);
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
