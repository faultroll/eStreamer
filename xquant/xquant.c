
#include "xquant.h"
#include <stddef.h>

extern const xq_init_s g_basic_vin;
extern const xq_init_s g_basic_vout;
extern const xq_init_s g_basic_vmul;
extern const xq_init_s g_basic_vmix;
extern const xq_init_s g_basic_venc;
extern const xq_init_s g_basic_vdec;
extern const xq_init_s g_basic_ain;
extern const xq_init_s g_basic_aout;
extern const xq_init_s g_basic_amul;
extern const xq_init_s g_basic_amix;
extern const xq_init_s g_basic_aenc;
extern const xq_init_s g_basic_adec;
extern const xq_init_s g_advanced_vin;
extern const xq_init_s g_advanced_vmulout;
extern const xq_init_s g_advanced_ain;
extern const xq_init_s g_advanced_amulout;
extern const xq_init_s g_business_venc;
// extern const xq_init_s g_business_vdec;
extern const xq_init_s g_business_aenc;
// extern const xq_init_s g_business_adec;
static const struct {
    xq_type_e type;
    const xq_init_s const *func; // must use pointer here (error: initializer is not constant)
} g_bdl_init_tbl[] = {
    // XQ_BASIC_BEGIN
    {XQ_BASIC_VIN,  &g_basic_vin},
    {XQ_BASIC_VOUT, &g_basic_vout},
    {XQ_BASIC_VMUL, &g_basic_vmul},
    {XQ_BASIC_VMIX, &g_basic_vmix},
    {XQ_BASIC_VENC, &g_basic_venc},
    {XQ_BASIC_VDEC, &g_basic_vdec},
    {XQ_BASIC_AIN,  &g_basic_ain},
    {XQ_BASIC_AOUT, &g_basic_aout},
    {XQ_BASIC_AMUL, &g_basic_amul},
    {XQ_BASIC_AMIX, &g_basic_amix},
    {XQ_BASIC_AENC, &g_basic_aenc},
    {XQ_BASIC_ADEC, &g_basic_adec},
    // XQ_ADVANCED_BEGIN
    {XQ_ADVANCED_VIN,      &g_advanced_vin},
    {XQ_ADVANCED_VMULOUT,  &g_advanced_vmulout},
    {XQ_ADVANCED_AIN,      &g_advanced_ain},
    {XQ_ADVANCED_AMULOUT,  &g_advanced_amulout},
    // XQ_BUSINESS_BEGIN
    {XQ_BUSINESS_VENC,     &g_business_venc},
    // {XQ_BUSINESS_VDEC,   &g_business_vdec},
    {XQ_BUSINESS_AENC,     &g_business_aenc},
    // {XQ_BUSINESS_ADEC,   &g_business_adec},
};

int xquant_init(void)
{
    int ret = 0;

    ret |= dl_init();

    int i = 0, len = sizeof(g_bdl_init_tbl) / sizeof(g_bdl_init_tbl[0]);
    for (; i < len; ++i) {
        ret |= g_bdl_init_tbl[i].func->init_xquant();
    }

    // if (ret != 0) {
    //     xquant_fini();
    // }

    return ret;
}

int xquant_fini(void)
{
    int i = 0, len = sizeof(g_bdl_init_tbl) / sizeof(g_bdl_init_tbl[0]);
    for (; i < len; ++i) {
        g_bdl_init_tbl[i].func->fini_xquant();
    }

    dl_fini();

    return 0;
}

xq_bdl_s *xquant_new(xq_type_e type, const void *param)
{
    int i = 0, len = sizeof(g_bdl_init_tbl) / sizeof(g_bdl_init_tbl[0]);
    for (; i < len; ++i) {
        if (type == g_bdl_init_tbl[i].type) {
            break;
        }
    }
    if (i < len) {
        xq_bdl_s *c1 = g_bdl_init_tbl[i].func->new_xquant(param);
        c1->type = g_bdl_init_tbl[i].type;
        return c1;
    } else {
        return NULL;
    }
}

void xquant_delete(xq_bdl_s *c1)
{
    xq_type_e type = c1->type;

    int i = 0, len = sizeof(g_bdl_init_tbl) / sizeof(g_bdl_init_tbl[0]);
    for (; i < len; ++i) {
        if (type == g_bdl_init_tbl[i].type) {
            break;
        }
    }
    if (i < len) {
        g_bdl_init_tbl[i].func->delete_xquant(c1);
    } else {
        ;
    }
}

void xquant_link(xq_chn_s *chn1, xq_chn_s *chn2)
{
#if 0
    DL_ID_S src = chn1->link_id;
    DL_ID_S dst = chn2->link_id;
    dl_link(src, dst);
    chn1->target_id = dst;
    chn2->target_id = src;
#else // 0
    DL_ID_S src = chn1->link_id;
    DL_ID_S dst = chn2->link_id;
    xq_chn_s *b1 = chn1->link_chn;
    xq_chn_s *b2 = chn2->link_chn;
    if (b1 != NULL || b2 != NULL) {
        b1 = (NULL == b1) ? chn1 : b1;
        b2 = (NULL == b2) ? chn2 : b2;
        xquant_link(b1, b2); // recursion
        b1->target_chn = b2;
        b2->target_chn = b1;
    } else {
        dl_link(src, dst);
    }
    chn1->target_id = dst;
    chn2->target_id = src;
#endif // 0
}

void xquant_unlink(xq_chn_s *chn1, xq_chn_s *chn2)
{
#if 0
    DL_ID_S src = chn1->link_id;
    DL_ID_S dst = chn2->link_id;
    dl_unlink(src, dst);
    chn1->target_id = (DL_ID_S)DL_ID_EMPTY;
    chn2->target_id = (DL_ID_S)DL_ID_EMPTY;
#else // 0
    DL_ID_S src = chn1->link_id;
    DL_ID_S dst = chn2->link_id;
    xq_chn_s *b1 = chn1->link_chn;
    xq_chn_s *b2 = chn2->link_chn;
    if (b1 != NULL || b2 != NULL) {
        b1 = (NULL == b1) ? chn1 : b1;
        b2 = (NULL == b2) ? chn2 : b2;
        xquant_unlink(b1, b2); // recursion
        b1->target_chn = NULL;
        b2->target_chn = NULL;
    } else {
        dl_unlink(src, dst);
    }
    chn1->target_id = (DL_ID_S)DL_ID_EMPTY;
    chn2->target_id = (DL_ID_S)DL_ID_EMPTY;
#endif // 0
}
