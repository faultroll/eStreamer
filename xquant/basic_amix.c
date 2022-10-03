
#include "basic_amix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dlink/dlink_module.h"
#include "xq_conv.h"

typedef struct _bdl_hnd_s_ {
    int refin;
    int refout;
    DL_ID_S amixgrp;
} bdl_hnd_s;

typedef struct _inchn_hnd_s_ {
    struct {
        DL_ID_S amixgrp;
        DL_ID_S amixchn;
        // DL_ID_S target;
    };

    struct {
        bdl_hnd_s *parent;
        // xq_amixchn_param_s p_amix;
    };
} inchn_hnd_s;

typedef struct _outchn_hnd_s_ {
    struct {
        DL_ID_S amixgrp;
        // DL_ID_S target;
    };

    struct {
        bdl_hnd_s *parent;
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

int basic_amix_init(void)
{
    return 0;
}
int basic_amix_fini(void)
{
    return 0;
}
xq_bdl_s *basic_amix_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_BASIC_AMIX;
    c1->priv = h1;

    c1->new_inchn = __new_inchn;
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refin = 0;
    h1->refout = 0;
    h1->amixgrp = (DL_ID_S)DL_ID_EMPTY;

    return c1;
}
void basic_amix_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    free(h1);
    free(c1);
}
const xq_init_s g_basic_amix = {basic_amix_init, basic_amix_fini, basic_amix_new, basic_amix_delete};

static xq_chn_s *__new_inchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refout) {
        return NULL;
    } else {
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

        c2->des.mod  = DL_INPUT;
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_BASIC_AMIX));
        c2->des.indatatype  = DL_AFRAME;
        c2->des.indatafun   = wrapper_amix_indata;
        c2->des.outdatatype = DL_TYPE_BUTT;
        c2->des.outdatafun  = NULL;
        c2->des.freedatafun = NULL;

        h2->parent = h1;

        return c2;
    }
}

static xq_chn_s *__new_outchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refout) {
        h1->refout++;

        xq_chn_s *c2 = malloc(sizeof(xq_chn_s));
        outchn_hnd_s *h2 = malloc(sizeof(outchn_hnd_s));

        c2->dir = XQ_CHN_OUT;
        c2->type = XQ_CHN_BYPASS;
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
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_BASIC_AMIX));
        c2->des.indatatype  = DL_TYPE_BUTT;
        c2->des.indatafun   = NULL;
        c2->des.outdatatype = DL_AFRAME;
        c2->des.outdatafun  = wrapper_amix_outdata;
        c2->des.freedatafun = wrapper_amix_freedata;

        h2->parent = h1;

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

    if (XQ_CHN_OUT == c2->dir) {
        if (h1->refin > 0) {
            return;
        }

        outchn_hnd_s *h2 = (outchn_hnd_s *)c2->priv;

        free(h2);
        free(c2);

        h1->refout--;
    } else if (XQ_CHN_IN == c2->dir) {
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

    h1->amixgrp = h1->parent->amixgrp;
    // amix-chn
    int amixgrp = h1->amixgrp.grp;
    int amixchn;
    xq_amixchn_param_s *p = (xq_amixchn_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_amix);
    xq_amixchn_create(amixgrp, &amixchn, p);
    // LOCK_MTX_UNLOCK(g_mtx_amix);
    h1->amixchn = (DL_ID_S) {
        .mod = DL_AMIX,
        .grp = amixgrp,
        .chn = amixchn,
    };
    // memcpy(&h1->p_amix, p, sizeof(h1->p_amix));

    c1->link_id = h1->amixchn;

    return 0;
}

static int __inchn_destroy_chn(xq_chn_s *self)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    c1->link_id = (DL_ID_S)DL_ID_EMPTY;
    c1->link_chn = NULL;

    // amix-grp
    // LOCK_MTX_LOCK(g_mtx_amix);
    xq_amixchn_destroy(h1->amixgrp.grp, h1->amixchn.chn);
    // LOCK_MTX_UNLOCK(g_mtx_amix);

    return 0;
}

static int __inchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    xq_amixchn_param_s *p = (xq_amixchn_param_s *)param;
    // 1. unlink target
    dl_unlink(c1->link_id, c1->target_id);
    // 2. set param
    xq_amixchn_setparam(h1->amixgrp.grp, h1->amixchn.chn, p);
    // 3. link target
    dl_link(c1->link_id, c1->target_id);

    return 0;
}

static int __inchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    xq_amixchn_param_s *p = (xq_amixchn_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_amix);
    xq_amixchn_getparam(h1->amixgrp.grp, h1->amixchn.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_amix);
    // memcpy(p, &h1->p_amix, sizeof(h1->p_amix));

    return 0;
}

static int __inchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    xq_amixchn_status_s *p = (xq_amixchn_status_s *)param;
    // LOCK_MTX_LOCK(g_mtx_amix);
    xq_amixchn_getstatus(h1->amixgrp.grp, h1->amixchn.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_amix);

    return 0;
}

static int __inchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    inchn_hnd_s *h1 = (inchn_hnd_s *)c1->priv;

    return 0;
}

static int __outchn_create_chn(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    // amix-grp
    int amixgrp;
    xq_amixgrp_param_s *p = (xq_amixgrp_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_amix);
    xq_amixgrp_create(&amixgrp, p);
    // LOCK_MTX_UNLOCK(g_mtx_amix);
    h1->amixgrp = (DL_ID_S) {
        .mod = DL_AMIX,
        .grp = amixgrp,
        .chn = -1,
    };
    h1->parent->amixgrp = h1->amixgrp;

    c1->link_id = h1->amixgrp;

    return 0;
}

static int __outchn_destroy_chn(xq_chn_s *self)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    c1->link_id = (DL_ID_S)DL_ID_EMPTY;
    c1->link_chn = NULL;

    h1->parent->amixgrp = (DL_ID_S)DL_ID_EMPTY;
    // amix-grp
    // LOCK_MTX_LOCK(g_mtx_amix);
    xq_amixgrp_destroy(h1->amixgrp.grp);
    // LOCK_MTX_UNLOCK(g_mtx_amix);

    return 0;
}

static int __outchn_set_param(xq_chn_s *self, const void *param)
{
    // xq_chn_s *c1 = (xq_chn_s *)self;
    // outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    return 0;
}

static int __outchn_get_param(xq_chn_s *self, void *param)
{
    // xq_chn_s *c1 = (xq_chn_s *)self;
    // outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    return 0;
}

static int __outchn_get_status(xq_chn_s *self, void *param)
{
    // xq_chn_s *c1 = (xq_chn_s *)self;
    // outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    return 0;
}

static int __outchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    return 0;
}
