
#include "xq_conv.h"

extern int __venc_to_vmul(const void *src, void *dst);
extern int __vout_to_vmul(const void *src, void *dst);
extern int __vdec_to_vmul(const void *src, void *dst);
extern int __vin_to_vmul(const void *src, void *dst);
extern int __aenc_to_vmul(const void *src, void *dst);
extern int __aout_to_vmul(const void *src, void *dst);
extern int __adec_to_vmul(const void *src, void *dst);
extern int __ain_to_vmul(const void *src, void *dst);


static const struct {
    DL_MOD_E src_mod;
    DL_MOD_E dst_mod;
    int (*conv_fun)(const void *, void *);
} dsp_conv_tbl[] = {
    {DL_VENC, DL_VMUL, __venc_to_vmul},
    {DL_VOUT, DL_VMUL, __vout_to_vmul},
    {DL_VDEC, DL_VMUL, __vdec_to_vmul},
    {DL_VIN,  DL_VMUL, __vin_to_vmul},
    {DL_AENC, DL_AMUL, __aenc_to_vmul},
    {DL_ADEC, DL_AMUL, __adec_to_vmul},
    {DL_AOUT, DL_AMUL, __aout_to_vmul},
    {DL_AIN,  DL_AMUL, __ain_to_vmul},
};
int dsp_convert(DL_MOD_E src_mod, const void *src_param,
                DL_MOD_E dst_mod, void *dst_param)
{
    int i = 0, len = sizeof(dsp_conv_tbl) / sizeof(dsp_conv_tbl[0]);
    for (; i < len; ++i)
        if ((src_mod == dsp_conv_tbl[i].src_mod) && (dst_mod == dsp_conv_tbl[i].dst_mod))
            return dsp_conv_tbl[i].conv_fun(src_param, dst_param);

    return -1;
}

