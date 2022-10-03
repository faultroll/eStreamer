
#include "advanced_ain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xq_conv.h"

typedef struct _bdl_hnd_s_ {
    int refout;
} bdl_hnd_s;

typedef struct _outchn_hnd_s_ {
    struct {
        // xq_chn_type_e type; // in p_outchn
        union {
            struct {
                xq_bdl_s *ain;
                xq_chn_s *h_ain;
            } local;
            struct {
                xq_bdl_s *adec;
                xq_chn_s *h_adec;
            } net;
            struct {
                DL_ID_S out;
                // DL_ID_S target;
            } bypass;
        };
    };

    struct {
        // bdl_hnd_s *parent;
        advanced_ain_outparam_s p_outchn;
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
static int __outchn_deccb_transfun(void *opaque, xq_chn_s *hnd, /* const */ DL_PUB_DATAINFO_S *data);

int advanced_ain_init(void)
{
    return 0;
}
int advanced_ain_fini(void)
{
    return 0;
}
xq_bdl_s *advanced_ain_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_ADVANCED_AIN;
    c1->priv = h1;

    c1->new_inchn = NULL;
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refout = 0;

    return c1;
}
void advanced_ain_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    free(h1);
    free(c1);
}
const xq_init_s g_advanced_ain = {advanced_ain_init, advanced_ain_fini, advanced_ain_new, advanced_ain_delete};

static xq_chn_s *__new_outchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refout) {
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

        advanced_ain_outparam_s *p = (advanced_ain_outparam_s *)param;
        memcpy(&h2->p_outchn, p, sizeof(h2->p_outchn));
        c2->type = h2->p_outchn.type;

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

    int ret = 0;
    switch (h1->p_outchn.type) {
        case XQ_CHN_LOCAL: {
            xq_ain_param_s *p_ain = (xq_ain_param_s *)param;
            h1->local.ain = bundle_new(XQ_BASIC_AIN, NULL);
            h1->local.h_ain = h1->local.ain->new_outchn(h1->local.ain, NULL); // &h1->p_outchn.local
            ret |= h1->local.h_ain->create_chn(h1->local.h_ain, p_ain);

            c1->link_chn = h1->local.h_ain;
        }
        break;
        case XQ_CHN_NET: {
            xq_adec_param_s *p_adec = (xq_adec_param_s *)param;
            basic_adec_outparam_s p_adec_init = h1->p_outchn.net;
            if (p_adec_init.dec_cb.dataprocfun != NULL) {
                p_adec_init.dec_cb.opaque = c1;
                p_adec_init.dec_cb.dataprocfun = __outchn_deccb_transfun;
            }
            h1->net.adec = bundle_new(XQ_BASIC_ADEC, NULL);
            h1->net.h_adec = h1->net.adec->new_outchn(h1->net.adec, &p_adec_init); // &h1->p_outchn.net
            ret |= h1->net.h_adec->create_chn(h1->net.h_adec, p_adec);

            c1->link_chn = h1->net.h_adec;
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
            ret |= h1->local.h_ain->destroy_chn(h1->local.h_ain);
            h1->local.ain->delete_chn(h1->local.ain, h1->local.h_ain);
            xquant_delete(h1->local.ain);
        }
        break;
        case XQ_CHN_NET: {
            ret |= h1->net.h_adec->destroy_chn(h1->net.h_adec);
            h1->net.adec->delete_chn(h1->net.adec, h1->net.h_adec);
            xquant_delete(h1->net.adec);
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
            xq_ain_param_s *p_ain = (xq_ain_param_s *)param;
            ret |= h1->local.h_ain->set_param(h1->local.h_ain, p_ain);
        }
        break;
        case XQ_CHN_NET: {
            xq_adec_param_s *p_adec = (xq_adec_param_s *)param;
            ret |= h1->net.h_adec->set_param(h1->net.h_adec, p_adec);
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
            xq_ain_param_s *p_ain = (xq_ain_param_s *)param;
            ret |= h1->local.h_ain->get_param(h1->local.h_ain, p_ain);
        }
        break;
        case XQ_CHN_NET: {
            xq_adec_param_s *p_adec = (xq_adec_param_s *)param;
            ret |= h1->net.h_adec->get_param(h1->net.h_adec, p_adec);
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
            xq_ain_status_s *p_ain = (xq_ain_status_s *)param;
            ret |= h1->local.h_ain->get_status(h1->local.h_ain, p_ain);
        }
        break;
        case XQ_CHN_NET: {
            xq_adec_status_s *p_adec = (xq_adec_status_s *)param;
            ret |= h1->net.h_adec->get_status(h1->net.h_adec, p_adec);
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
            ret |= h1->local.h_ain->custom_ctrl(h1->local.h_ain, oper, param);
        }
        break;
        case XQ_CHN_NET: {
            ret |= h1->net.h_adec->custom_ctrl(h1->net.h_adec, oper, param);
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

static int __outchn_deccb_transfun(void *opaque, xq_chn_s *hnd, /* const */ DL_PUB_DATAINFO_S *data)
{
    xq_chn_s *c1 = (xq_chn_s *)opaque;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    return h1->p_outchn.net.dec_cb.dataprocfun(h1->p_outchn.net.dec_cb.opaque, c1, data);
}
