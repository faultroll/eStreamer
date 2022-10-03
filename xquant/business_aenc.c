
#include "business_aenc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xq_conv.h"

typedef struct _bdl_hnd_s_ {
    int refin;
    int refout;
    xq_bdl_s *amulout;
} bdl_hnd_s;

typedef struct _inchn_hnd_s_ {
    struct {
        xq_bdl_s *ain;
        xq_chn_s *h_ain;
        xq_bdl_s *amulout;
        xq_chn_s *h_amuloutgrp;
    };

    /* struct
    {
        // bdl_hnd_s *parent;
    }; */
} inchn_hnd_s;

typedef struct _outchn_hnd_s_ {
    struct {
        xq_bdl_s *amulout;
        xq_chn_s *h_amuloutchn;
    };

    struct {
        // bdl_hnd_s *parent;
        business_aenc_outparam_s p_outchn;
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

int business_aenc_init(void)
{
    return 0;
}
int business_aenc_fini(void)
{
    return 0;
}
xq_bdl_s *business_aenc_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_BUSINESS_AENC;
    c1->priv = h1;

    c1->new_inchn = __new_inchn;
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refin = 0;
    h1->refout = 0;
    h1->amulout = bundle_new(XQ_ADVANCED_AMULOUT, NULL);

    return c1;
}
void business_aenc_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    xquant_delete(h1->amulout);

    free(h1);
    free(c1);
}
const xq_init_s g_business_aenc = {business_aenc_init, business_aenc_fini, business_aenc_new, business_aenc_delete};

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
        h2->amulout = h1->amulout;

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
        h2->amulout = h1->amulout;

        business_aenc_outparam_s *p = (business_aenc_outparam_s *)param;
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
    xq_ain_param_s *p_ain = (xq_ain_param_s *)param;
    xq_amulgrp_param_s p_amulout;
    xq_conv(DL_AIN, p_ain, DL_AMUL, &p_amulout);
    // advanced_ain
    advanced_ain_outparam_s p_ain_init;
    p_ain_init.type = XQ_CHN_LOCAL;
    h1->ain = bundle_new(XQ_ADVANCED_AIN, NULL);
    h1->h_ain = h1->ain->new_outchn(h1->ain, &p_ain_init);
    ret |= h1->h_ain->create_chn(h1->h_ain, p_ain);
    // advanced_amulout
    h1->h_amuloutgrp = h1->amulout->new_inchn(h1->amulout, NULL);
    ret |= h1->h_amuloutgrp->create_chn(h1->h_amuloutgrp, &p_amulout);
    xquant_link(h1->h_ain, h1->h_amuloutgrp);

    c1->link_chn = h1->h_amuloutgrp;
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
    xquant_unlink(h1->h_ain, h1->h_amuloutgrp);
    ret |= h1->h_amuloutgrp->destroy_chn(h1->h_amuloutgrp);
    h1->amulout->delete_chn(h1->amulout, h1->h_amuloutgrp);
    ret |= h1->h_ain->destroy_chn(h1->h_ain);
    h1->ain->delete_chn(h1->ain, h1->h_ain);
    xquant_delete(h1->ain);

    return ret;
}

static int __inchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    xq_ain_param_s *p_ain = (xq_ain_param_s *)param;
    xq_amulgrp_param_s p_amulout;
    xq_conv(DL_AIN, p_ain, DL_AMUL, &p_amulout);
    ret |= h1->h_amuloutgrp->set_param(h1->h_amuloutgrp, &p_amulout);
    ret |= h1->h_ain->set_param(h1->h_ain, p_ain);

    return ret;
}

static int __inchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    xq_ain_param_s *p_ain = (xq_ain_param_s *)param;
    ret |= h1->h_ain->get_param(h1->h_ain, p_ain);

    return ret;
}

static int __inchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    xq_ain_status_s *p_ain = (xq_ain_status_s *)param;
    ret |= h1->h_ain->get_status(h1->h_ain, p_ain);

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
            ret |= h1->h_ain->custom_ctrl(h1->h_ain, oper, param);
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
    business_aenc_outparam_s p_amulout_init = h1->p_outchn;
    if (XQ_CHN_NET == h1->p_outchn.type) {
        if (p_amulout_init.net.enc_cb.dataprocfun != NULL) {
            p_amulout_init.net.enc_cb.opaque = c1;
            p_amulout_init.net.enc_cb.dataprocfun = __outchn_enccb_transfun;
        }
    }
    h1->h_amuloutchn = h1->amulout->new_outchn(h1->amulout, &p_amulout_init);
    ret |= h1->h_amuloutchn->create_chn(h1->h_amuloutchn, param);

    c1->link_chn = h1->h_amuloutchn;
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

    ret |= h1->h_amuloutchn->destroy_chn(h1->h_amuloutchn);
    h1->amulout->delete_chn(h1->amulout, h1->h_amuloutchn);

    return ret;
}

static int __outchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_amuloutchn->set_param(h1->h_amuloutchn, param);

    return ret;
}

static int __outchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_amuloutchn->get_param(h1->h_amuloutchn, param);

    return ret;
}

static int __outchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_amuloutchn->get_status(h1->h_amuloutchn, param);

    return ret;
}

static int __outchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    int ret = 0;
    switch (oper) {
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
