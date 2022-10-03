
#include "advanced_vin.h"
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
                xq_bdl_s *vin;
                xq_chn_s *h_vin;
            } local;
            struct {
                xq_bdl_s *vdec;
                xq_chn_s *h_vdec;
            } net;
            struct {
                DL_ID_S out;
                // DL_ID_S target;
            } bypass;
        };
    };

    struct {
        // bdl_hnd_s *parent;
        advanced_vin_outparam_s p_outchn;
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

int advanced_vin_init(void)
{
    return 0;
}
int advanced_vin_fini(void)
{
    return 0;
}
xq_bdl_s *advanced_vin_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_ADVANCED_VIN;
    c1->priv = h1;

    c1->new_inchn = NULL;
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refout = 0;

    return c1;
}
void advanced_vin_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    free(h1);
    free(c1);
}
const xq_init_s g_advanced_vin = {advanced_vin_init, advanced_vin_fini, advanced_vin_new, advanced_vin_delete};

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

        /* c2->des.mod  = DL_OUTPUT;
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_ADVANCED_VIN));
        c2->des.indatatype  = DL_TYPE_BUTT;
        c2->des.indatafun   = NULL;
        c2->des.outdatatype = DL_VFRAME;
        c2->des.outdatafun  = __chn_cb_infun;
        c2->des.freedatafun = __chn_cb_freefun; */

        advanced_vin_outparam_s *p = (advanced_vin_outparam_s *)param;
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
            xq_vin_param_s *p_vin = (xq_vin_param_s *)param;
            h1->local.vin = bundle_new(XQ_BASIC_VIN, NULL);
            h1->local.h_vin = h1->local.vin->new_outchn(h1->local.vin, NULL); // &h1->p_outchn.local
            ret |= h1->local.h_vin->create_chn(h1->local.h_vin, p_vin);

            c1->link_chn = h1->local.h_vin;
        }
        break;
        case XQ_CHN_NET: {
            xq_vdec_param_s *p_vdec = (xq_vdec_param_s *)param;
            basic_vdec_outparam_s p_vdec_init = h1->p_outchn.net;
            if (p_vdec_init.dec_cb.dataprocfun != NULL) {
                p_vdec_init.dec_cb.opaque = c1;
                p_vdec_init.dec_cb.dataprocfun = __outchn_deccb_transfun;
            }
            h1->net.vdec = bundle_new(XQ_BASIC_VDEC, NULL);
            h1->net.h_vdec = h1->net.vdec->new_outchn(h1->net.vdec, &p_vdec_init); // &h1->p_outchn.net
            ret |= h1->net.h_vdec->create_chn(h1->net.h_vdec, p_vdec);

            c1->link_chn = h1->net.h_vdec;
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
            ret |= h1->local.h_vin->destroy_chn(h1->local.h_vin);
            h1->local.vin->delete_chn(h1->local.vin, h1->local.h_vin);
            xquant_delete(h1->local.vin);
        }
        break;
        case XQ_CHN_NET: {
            ret |= h1->net.h_vdec->destroy_chn(h1->net.h_vdec);
            h1->net.vdec->delete_chn(h1->net.vdec, h1->net.h_vdec);
            xquant_delete(h1->net.vdec);
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
            xq_vin_param_s *p_vin = (xq_vin_param_s *)param;
            ret |= h1->local.h_vin->set_param(h1->local.h_vin, p_vin);
        }
        break;
        case XQ_CHN_NET: {
            xq_vdec_param_s *p_vdec = (xq_vdec_param_s *)param;
            ret |= h1->net.h_vdec->set_param(h1->net.h_vdec, p_vdec);
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
            xq_vin_param_s *p_vin = (xq_vin_param_s *)param;
            ret |= h1->local.h_vin->get_param(h1->local.h_vin, p_vin);
        }
        break;
        case XQ_CHN_NET: {
            xq_vdec_param_s *p_vdec = (xq_vdec_param_s *)param;
            ret |= h1->net.h_vdec->get_param(h1->net.h_vdec, p_vdec);
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
            xq_vin_status_s *p_vin = (xq_vin_status_s *)param;
            ret |= h1->local.h_vin->get_status(h1->local.h_vin, p_vin);
        }
        break;
        case XQ_CHN_NET: {
            xq_vdec_status_s *p_vdec = (xq_vdec_status_s *)param;
            ret |= h1->net.h_vdec->get_status(h1->net.h_vdec, p_vdec);
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
    // NOTE(lgY): parallel bundle, only choose one, no need to dispatch
    /* switch (oper)
    {
        case XQ_OPER_USRPIC_SET:
        case XQ_OPER_USRPIC_SWITCH:
        {
            // only net support this
            if (XQ_CHN_LOCAL == h1->p_outchn.type)
            {
                ret |= h1->local.h_vin->custom_ctrl(h1->local.h_vin, oper, param);
            }
            else
            {
                ret |= -1;
            }
        }
        break;
        default:
        {
            ret |= -1;
        }
        break;
    } */
    switch (h1->p_outchn.type) {
        case XQ_CHN_LOCAL: {
            ret |= h1->local.h_vin->custom_ctrl(h1->local.h_vin, oper, param);
        }
        break;
        case XQ_CHN_NET: {
            ret |= h1->net.h_vdec->custom_ctrl(h1->net.h_vdec, oper, param);
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
