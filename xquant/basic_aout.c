
#include "basic_aout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dlink/dlink_module.h"
#include "xq_conv.h"

typedef struct _bdl_hnd_s_ {
    int refin;
} bdl_hnd_s;

typedef struct _inchn_hnd_s_ {
    struct {
        DL_ID_S aout;
        // DL_ID_S target;
    };

    struct {
        // xq_aout_param_s p_aout;
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

int basic_aout_init(void)
{
    return 0;
}
int basic_aout_fini(void)
{
    return 0;
}
xq_bdl_s *basic_aout_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_BASIC_AOUT;
    c1->priv = h1;

    c1->new_inchn = __new_inchn;
    c1->new_outchn = NULL; // no outchn
    c1->delete_chn = __delete_chn;

    h1->refin = 0;

    return c1;
}
void basic_aout_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    free(h1);
    free(c1);
}
const xq_init_s g_basic_aout = {basic_aout_init, basic_aout_fini, basic_aout_new, basic_aout_delete};

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

        c2->des.mod  = DL_INPUT;
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_BASIC_AOUT));
        c2->des.indatatype  = DL_AFRAME;
        c2->des.indatafun   = wrapper_aout_indata;
        c2->des.outdatatype = DL_TYPE_BUTT;
        c2->des.outdatafun  = NULL;
        c2->des.freedatafun = NULL;

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
        h1->refin--;

        free(h2);
        free(c2);
    } else {
        ;
    }
}

static int __inchn_create_chn(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    // aout
    int aoutchn;
    xq_aout_param_s *p = (xq_aout_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_aout);
    xq_aout_create(&aoutchn, p);
    // LOCK_MTX_UNLOCK(g_mtx_aout);
    h1->aout = (DL_ID_S) {
        .mod = DL_AOUT,
        .grp = -1,
        .chn = aoutchn,
    };
    // memcpy(&h1->p_aout, p, sizeof(h1->p_aout));

    c1->link_id = h1->aout;

    return 0;
}

static int __inchn_destroy_chn(xq_chn_s *self)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    c1->link_id = (DL_ID_S)DL_ID_EMPTY;
    c1->link_chn = NULL;

    // aout
    // LOCK_MTX_LOCK(g_mtx_aout);
    xq_aout_destroy(h1->aout.chn);
    // LOCK_MTX_UNLOCK(g_mtx_aout);

    return 0;
}

static int __inchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    xq_aout_param_s *p = (xq_aout_param_s *)param;
    // 1. unlink target
    dl_unlink(c1->link_id, c1->target_id);
    // 2. set param
    xq_aout_setparam(h1->aout.chn, p);
    // 3. link target
    dl_link(c1->link_id, c1->target_id);

    return 0;
}

static int __inchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    xq_aout_param_s *p = (xq_aout_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_aout);
    xq_aout_getparam(h1->aout.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_aout);
    // memcpy(p, &h1->p_aout, sizeof(h1->p_aout));

    return 0;
}

static int __inchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    xq_aout_status_s *p = (xq_aout_status_s *)param;
    // LOCK_MTX_LOCK(g_mtx_aout);
    xq_aout_getstatus(h1->aout.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_aout);

    return 0;
}

static int __inchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    return 0;
}
