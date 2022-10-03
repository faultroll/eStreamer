
#include "advanced_amulout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xq_conv.h"

typedef struct _bdl_hnd_s_ {
    int refin;
    int refout;
    xq_bdl_s *amul;
} bdl_hnd_s;

typedef struct _inchn_hnd_s_ {
    struct {
        xq_bdl_s *amul;
        xq_chn_s *h_amulgrp;
    };

    /* struct
    {
        // bdl_hnd_s *parent;
    }; */
} inchn_hnd_s;

typedef struct _outchn_hnd_s_ {
    struct {
        xq_bdl_s *amul;
        xq_chn_s *h_amulchn;

        // xq_chn_type_e type; // in p_outchn
        union {
            struct {
                xq_bdl_s *aout;
                xq_chn_s *h_aout;
            } local;
            struct {
                xq_bdl_s *aenc;
                xq_chn_s *h_aenc;
            } net;
            struct {
                DL_ID_S in;
                // DL_ID_S target;
            } bypass;
        };
    };

    struct {
        // bdl_hnd_s *parent;
        advanced_amulout_outparam_s p_outchn;
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

int advanced_amulout_init(void)
{
    return 0;
}
int advanced_amulout_fini(void)
{
    return 0;
}
xq_bdl_s *advanced_amulout_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_ADVANCED_AMULOUT;
    c1->priv = h1;

    c1->new_inchn = __new_inchn;
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refin = 0;
    h1->refout = 0;
    h1->amul = bundle_new(XQ_BASIC_AMUL, NULL);

    return c1;
}
void advanced_amulout_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    xquant_delete(h1->amul);

    free(h1);
    free(c1);
}
const xq_init_s g_advanced_amulout = {advanced_amulout_init, advanced_amulout_fini, advanced_amulout_new, advanced_amulout_delete};

static xq_chn_s *__new_inchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refin) {
        h1->refin++;

        xq_chn_s *c2 = malloc(sizeof(xq_chn_s));
        inchn_hnd_s *h2 = malloc(sizeof(inchn_hnd_s));

        c2->dir = XQ_CHN_IN;
        c2->type = XQ_CHN_BYPASS;
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
        h2->amul = h1->amul;

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
        h2->amul = h1->amul;

        advanced_amulout_outparam_s *p = (advanced_amulout_outparam_s *)param;
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
    h1->h_amulgrp = h1->amul->new_inchn(h1->amul, NULL);
    ret |= h1->h_amulgrp->create_chn(h1->h_amulgrp, param);

    c1->link_chn = h1->h_amulgrp;
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
    ret |= h1->h_amulgrp->destroy_chn(h1->h_amulgrp);
    h1->amul->delete_chn(h1->amul, h1->h_amulgrp);

    return ret;
}

static int __inchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_amulgrp->set_param(h1->h_amulgrp, param);

    return ret;
}

static int __inchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_amulgrp->get_param(h1->h_amulgrp, param);

    return ret;
}

static int __inchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    ret |= h1->h_amulgrp->get_status(h1->h_amulgrp, param);

    return ret;
}

static int __inchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    // xq_chn_s *c1 = (xq_chn_s *)self;
    // inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    int ret = 0;
    switch (oper) {
        // ret |= h1->h_amulgrp->custom_ctrl(h1->h_amulgrp, oper, param);
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
            xq_amulchn_param_s p_amul;
            xq_aout_param_s *p_aout = (xq_aout_param_s *)param;
            // xq_conv(DL_AOUT, p_aout, DL_AMUL, &p_amul);
            h1->h_amulchn = h1->amul->new_outchn(h1->amul, NULL);
            ret |= h1->h_amulchn->create_chn(h1->h_amulchn, &p_amul);
            h1->local.aout = bundle_new(XQ_BASIC_AOUT, NULL);
            h1->local.h_aout = h1->local.aout->new_inchn(h1->local.aout, NULL); // &h1->p_outchn.local
            ret |= h1->local.h_aout->create_chn(h1->local.h_aout, p_aout);
            xquant_link(h1->h_amulchn, h1->local.h_aout);

            c1->link_chn = h1->local.h_aout;
        }
        break;
        case XQ_CHN_NET: {
            xq_amulchn_param_s p_amul;
            xq_aenc_param_s *p_aenc = (xq_aenc_param_s *)param;
            basic_aenc_inparam_s p_aenc_init = h1->p_outchn.net;
            if (p_aenc_init.enc_cb.dataprocfun != NULL) {
                p_aenc_init.enc_cb.opaque = c1;
                p_aenc_init.enc_cb.dataprocfun = __outchn_enccb_transfun;
            }
            xq_conv(DL_AENC, p_aenc, DL_AMUL, &p_amul);
            h1->h_amulchn = h1->amul->new_outchn(h1->amul, NULL);
            ret |= h1->h_amulchn->create_chn(h1->h_amulchn, &p_amul);
            h1->net.aenc = bundle_new(XQ_BASIC_AENC, NULL);
            h1->net.h_aenc = h1->net.aenc->new_inchn(h1->net.aenc, &p_aenc_init); // &h1->p_outchn.net
            ret |= h1->net.h_aenc->create_chn(h1->net.h_aenc, p_aenc);
            xquant_link(h1->h_amulchn, h1->net.h_aenc);

            c1->link_chn = h1->net.h_aenc;
        }
        break;
        /* case XQ_CHN_BYPASS:
        {
        }
        break; */
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
            xquant_unlink(h1->h_amulchn, h1->local.h_aout);
            ret |= h1->local.h_aout->destroy_chn(h1->local.h_aout);
            h1->local.aout->delete_chn(h1->local.aout, h1->local.h_aout);
            xquant_delete(h1->local.aout);
            ret |= h1->h_amulchn->destroy_chn(h1->h_amulchn);
            h1->amul->delete_chn(h1->amul, h1->h_amulchn);
        }
        break;
        case XQ_CHN_NET: {
            xquant_unlink(h1->h_amulchn, h1->net.h_aenc);
            ret |= h1->net.h_aenc->destroy_chn(h1->net.h_aenc);
            h1->net.aenc->delete_chn(h1->net.aenc, h1->net.h_aenc);
            xquant_delete(h1->net.aenc);
            ret |= h1->h_amulchn->destroy_chn(h1->h_amulchn);
            h1->amul->delete_chn(h1->amul, h1->h_amulchn);
        }
        break;
        /* case XQ_CHN_BYPASS:
        {
        }
        break; */
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
            xq_amulchn_param_s p_amul;
            xq_aout_param_s *p_aout = (xq_aout_param_s *)param;
            xq_conv(DL_AOUT, p_aout, DL_AMUL, &p_amul);
            ret |= h1->local.h_aout->set_param(h1->local.h_aout, p_aout);
            ret |= h1->h_amulchn->set_param(h1->h_amulchn, &p_amul);
        }
        break;
        case XQ_CHN_NET: {
            xq_amulchn_param_s p_amul;
            xq_aenc_param_s *p_aenc = (xq_aenc_param_s *)param;
            xq_conv(DL_AENC, p_aenc, DL_AMUL, &p_amul);
            ret |= h1->net.h_aenc->set_param(h1->net.h_aenc, p_aenc);
            ret |= h1->h_amulchn->set_param(h1->h_amulchn, &p_amul);
        }
        break;
        /* case XQ_CHN_BYPASS:
        {
        }
        break; */
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
            xq_aout_param_s *p_aout = (xq_aout_param_s *)param;
            ret |= h1->local.h_aout->get_param(h1->local.h_aout, p_aout);
        }
        break;
        case XQ_CHN_NET: {
            xq_aenc_param_s *p_aenc = (xq_aenc_param_s *)param;
            ret |= h1->net.h_aenc->get_param(h1->net.h_aenc, p_aenc);
        }
        break;
        /* case XQ_CHN_BYPASS:
        {
        }
        break; */
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
            xq_aout_status_s *p_aout = (xq_aout_status_s *)param;
            ret |= h1->local.h_aout->get_status(h1->local.h_aout, p_aout);
        }
        break;
        case XQ_CHN_NET: {
            xq_aenc_status_s *p_aenc = (xq_aenc_status_s *)param;
            ret |= h1->net.h_aenc->get_status(h1->net.h_aenc, p_aenc);
        }
        break;
        /* case XQ_CHN_BYPASS:
        {
        }
        break; */
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

    int ret = 0;
    switch (h1->p_outchn.type) {
        case XQ_CHN_LOCAL: {
            ret |= h1->local.h_aout->custom_ctrl(h1->local.h_aout, oper, param);
        }
        break;
        case XQ_CHN_NET: {
            ret |= h1->net.h_aenc->custom_ctrl(h1->net.h_aenc, oper, param);
        }
        break;
        /* case XQ_CHN_BYPASS:
        {
        }
        break; */
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
