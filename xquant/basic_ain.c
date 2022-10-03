
#include "basic_ain.h"
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
        DL_ID_S ain;
        // DL_ID_S target;
    };

    struct {
        // xq_ain_param_s p_ain;
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

int basic_ain_init(void)
{
    return 0;
}
int basic_ain_fini(void)
{
    return 0;
}
xq_bdl_s *basic_ain_new(const void *param)
{
    xq_bdl_s *c1 = malloc(sizeof(xq_bdl_s));
    bdl_hnd_s *h1 = malloc(sizeof(bdl_hnd_s));

    // c1->type = XQ_BASIC_AIN;
    c1->priv = h1;

    c1->new_inchn = NULL; // no inchn
    c1->new_outchn = __new_outchn;
    c1->delete_chn = __delete_chn;

    h1->refout = 0;

    return c1;
}
void basic_ain_delete(xq_bdl_s *self)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    free(h1);
    free(c1);
}
const xq_init_s g_basic_ain = {basic_ain_init, basic_ain_fini, basic_ain_new, basic_ain_delete};

static xq_chn_s *__new_outchn(xq_bdl_s *self, const void *param)
{
    xq_bdl_s *c1 = (xq_bdl_s *)self;
    bdl_hnd_s *h1 = (bdl_hnd_s *)c1->priv;

    if (0 == h1->refout) {
        h1->refout++;

        xq_chn_s *c2 = malloc(sizeof(xq_chn_s));
        outchn_hnd_s *h2 = malloc(sizeof(outchn_hnd_s));

        c2->dir = XQ_CHN_OUT;
        c2->type = XQ_CHN_LOCAL;
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
        snprintf(c2->des.name, sizeof(c2->des.name), __STRING(XQ_BASIC_AIN));
        c2->des.indatatype  = DL_TYPE_BUTT;
        c2->des.indatafun   = NULL;
        c2->des.outdatatype = DL_AFRAME;
        c2->des.outdatafun  = wrapper_ain_outdata;
        c2->des.freedatafun = wrapper_ain_freedata;

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
        h1->refout--;

        free(h2);
        free(c2);
    } else {
        ;
    }
}

static int __outchn_create_chn(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    // ain
    int ainchn;
    xq_ain_param_s *p = (xq_ain_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_ain);
    xq_ain_create(&ainchn, p);
    // LOCK_MTX_UNLOCK(g_mtx_ain);
    h1->ain = (DL_ID_S) {
        .mod = DL_AIN,
        .grp = -1,
        .chn = ainchn,
    };
    // memcpy(&h1->p_ain, p, sizeof(h1->p_ain));

    c1->link_id = h1->ain;

    return 0;
}

static int __outchn_destroy_chn(xq_chn_s *self)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    c1->link_id = (DL_ID_S)DL_ID_EMPTY;
    c1->link_chn = NULL;

    // ain
    // LOCK_MTX_LOCK(g_mtx_ain);
    xq_ain_destroy(h1->ain.chn);
    // LOCK_MTX_UNLOCK(g_mtx_ain);

    return 0;
}

static int __outchn_set_param(xq_chn_s *self, const void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_ain_param_s *p = (xq_ain_param_s *)param;
    // 1. unlink target
    dl_unlink(c1->link_id, c1->target_id);
    // 2. set param
    xq_ain_setparam(h1->ain.chn, p);
    // 3. link target
    dl_link(c1->link_id, c1->target_id);

    return 0;
}

static int __outchn_get_param(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_ain_param_s *p = (xq_ain_param_s *)param;
    // LOCK_MTX_LOCK(g_mtx_ain);
    xq_ain_getparam(h1->ain.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_ain);
    // memcpy(p, &h1->p_ain, sizeof(h1->p_ain));

    return 0;
}

static int __outchn_get_status(xq_chn_s *self, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    xq_ain_status_s *p = (xq_ain_status_s *)param;
    // LOCK_MTX_LOCK(g_mtx_ain);
    xq_ain_getstatus(h1->ain.chn, p);
    // LOCK_MTX_UNLOCK(g_mtx_ain);

    return 0;
}

static int __outchn_custom_ctrl(xq_chn_s *self, const int oper, void *param)
{
    xq_chn_s *c1 = (xq_chn_s *)self;
    outchn_hnd_s *h1 = (outchn_hnd_s *)c1->priv;

    return 0;
}
