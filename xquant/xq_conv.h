
#ifndef _XQ_CONV_H_
#define _XQ_CONV_H_

#include "dlink/dlink.h" // DL_MOD_E

#if defined(__cplusplus)
extern "C" {
#endif

// VIN
typedef struct xq_vin_param xq_vin_param_s;
typedef struct xq_vin_status xq_vin_status_s;
extern int xq_vin_create(int *pVinChn, const xq_vin_param_s *pstVinChnParam);
extern int xq_vin_destroy(int VinChn);
extern int xq_vin_getparam(int VinChn, xq_vin_param_s *pstVinChnParam);
extern int xq_vin_setparam(int VinChn, const xq_vin_param_s *pstVinChnParam);
extern int xq_vin_getstatus(int VinChn, xq_vin_status_s *pstVinStatus);
// VENC
typedef struct xq_venc_param xq_venc_param_s;
typedef struct xq_venc_status xq_venc_status_s;
extern int xq_venc_create(int *pVencChn, const xq_venc_param_s *pstVencChnParam);
extern int xq_venc_destroy(int VencChn);
extern int xq_venc_getparam(int VencChn, xq_venc_param_s *pstVencChnParam);
extern int xq_venc_setparam(int VencChn, const xq_venc_param_s *pstVencChnParam);
extern int xq_venc_getstatus(int VencChn, xq_venc_status_s *pstVencStatus);
// VDEC
typedef struct xq_vdec_param xq_vdec_param_s;
typedef struct xq_vdec_status xq_vdec_status_s;
extern int xq_vdec_create(int *pVdecChn, const xq_vdec_param_s *pstVdecChnParam);
extern int xq_vdec_destroy(int VdecChn);
extern int xq_vdec_getparam(int VdecChn, xq_vdec_param_s *pstVdecChnParam);
extern int xq_vdec_setparam(int VdecChn, const xq_vdec_param_s *pstVdecChnParam);
extern int xq_vdec_getstatus(int VdecChn, xq_vdec_status_s *pstVdecStatus);
// VMUL
typedef struct xq_vmulgrp_param xq_vmulgrp_param_s;
typedef struct xq_vmulchn_param xq_vmulchn_param_s;
typedef struct xq_vmulchn_status xq_vmulchn_status_s;
extern int xq_vmulgrp_create(int *pVmulGrp, const xq_vmulgrp_param_s *pstVmulGrpParam);
extern int xq_vmulgrp_destroy(int VmulGrp);
extern int xq_vmulchn_create(int VmulGrp, int *pVmulChn, const xq_vmulchn_param_s *pstVmulGrpParam);
extern int xq_vmulchn_destroy(int VmulGrp, int VmulChn);
extern int xq_vmulchn_getparam(int VmulGrp, int VmulChn, xq_vmulchn_param_s *pstVmulGrpParam);
extern int xq_vmulchn_setparam(int VmulGrp, int VmulChn, const xq_vmulchn_param_s *pstVmulGrpParam);
extern int xq_vmulchn_getstatus(int VmulGrp, int VmulChn, xq_vmulchn_status_s *pstVoutStatus);
// VMIX
typedef struct xq_vmixgrp_param xq_vmixgrp_param_s;
typedef struct xq_vmixchn_param xq_vmixchn_param_s;
typedef struct xq_vmixchn_status xq_vmixchn_status_s;
extern int xq_vmixgrp_create(int *pVmixGrp, const xq_vmixgrp_param_s *pstVmixGrpParam);
extern int xq_vmixgrp_destroy(int VmixGrp);
extern int xq_vmixchn_create(int VmixGrp, int *pVmixChn, const xq_vmixchn_param_s *pstVmixGrpParam);
extern int xq_vmixchn_destroy(int VmixGrp, int VmixChn);
extern int xq_vmixchn_getparam(int VmixGrp, int VmixChn, xq_vmixchn_param_s *pstVmixGrpParam);
extern int xq_vmixchn_setparam(int VmixGrp, int VmixChn, const xq_vmixchn_param_s *pstVmixGrpParam);
extern int xq_vmixchn_getstatus(int VmixGrp, int VmixChn, xq_vmixchn_status_s *pstVoutStatus);
// VOUT
typedef struct xq_vout_param xq_vout_param_s;
typedef struct xq_vout_status xq_vout_status_s;
extern int xq_vout_create(int *pVoutChn, const xq_vout_param_s *pstVoutChnParam);
extern int xq_vout_destroy(int VoutChn);
extern int xq_vout_getparam(int VoutChn, xq_vout_param_s *pstVoutChnParam);
extern int xq_vout_setparam(int VoutChn, const xq_vout_param_s *pstVoutChnParam);
extern int xq_vout_getstatus(int VoutChn, xq_vout_status_s *pstVoutStatus);
// AIN
typedef struct xq_ain_param xq_ain_param_s;
typedef struct xq_ain_status xq_ain_status_s;
extern int xq_ain_create(int *pAinChn, const xq_ain_param_s *pstAinChnParam);
extern int xq_ain_destroy(int AinChn);
extern int xq_ain_getparam(int AinChn, xq_ain_param_s *pstAinChnParam);
extern int xq_ain_setparam(int AinChn, const xq_ain_param_s *pstAinChnParam);
extern int xq_ain_getstatus(int AinChn, xq_ain_status_s *pstAinStatus);
// AENC
typedef struct xq_aenc_param xq_aenc_param_s;
typedef struct xq_aenc_status xq_aenc_status_s;
extern int xq_aenc_create(int *pAencChn, const xq_aenc_param_s *pstAencChnParam);
extern int xq_aenc_destroy(int AencChn);
extern int xq_aenc_getparam(int AencChn, xq_aenc_param_s *pstAencChnParam);
extern int xq_aenc_setparam(int AencChn, const xq_aenc_param_s *pstAencChnParam);
extern int xq_aenc_getstatus(int AencChn, xq_aenc_status_s *pstAencStatus);
// ADEC
typedef struct xq_adec_param xq_adec_param_s;
typedef struct xq_adec_status xq_adec_status_s;
extern int xq_adec_create(int *pAdecChn, const xq_adec_param_s *pstAdecChnParam);
extern int xq_adec_destroy(int AdecChn);
extern int xq_adec_getparam(int AdecChn, xq_adec_param_s *pstAdecChnParam);
extern int xq_adec_setparam(int AdecChn, const xq_adec_param_s *pstAdecChnParam);
extern int xq_adec_getstatus(int AdecChn, xq_adec_status_s *pstAdecStatus);
// AMUL
typedef struct xq_amulgrp_param xq_amulgrp_param_s;
typedef struct xq_amulchn_param xq_amulchn_param_s;
typedef struct xq_amulchn_status xq_amulchn_status_s;
extern int xq_amulgrp_create(int *pAmulGrp, const xq_amulgrp_param_s *pstAmulGrpParam);
extern int xq_amulgrp_destroy(int AmulGrp);
extern int xq_amulchn_create(int AmulGrp, int *pAmulChn, const xq_amulchn_param_s *pstAmulGrpParam);
extern int xq_amulchn_destroy(int AmulGrp, int AmulChn);
extern int xq_amulchn_getparam(int AmulGrp, int AmulChn, xq_amulchn_param_s *pstAmulGrpParam);
extern int xq_amulchn_setparam(int AmulGrp, int AmulChn, const xq_amulchn_param_s *pstAmulGrpParam);
extern int xq_amulchn_getstatus(int AmulGrp, int AmulChn, xq_amulchn_status_s *pstAoutStatus);
// AMIX
typedef struct xq_amixgrp_param xq_amixgrp_param_s;
typedef struct xq_amixchn_param xq_amixchn_param_s;
typedef struct xq_amixchn_status xq_amixchn_status_s;
extern int xq_amixgrp_create(int *pAmixGrp, const xq_amixgrp_param_s *pstAmixGrpParam);
extern int xq_amixgrp_destroy(int AmixGrp);
extern int xq_amixchn_create(int AmixGrp, int *pAmixChn, const xq_amixchn_param_s *pstAmixGrpParam);
extern int xq_amixchn_destroy(int AmixGrp, int AmixChn);
extern int xq_amixchn_getparam(int AmixGrp, int AmixChn, xq_amixchn_param_s *pstAmixGrpParam);
extern int xq_amixchn_setparam(int AmixGrp, int AmixChn, const xq_amixchn_param_s *pstAmixGrpParam);
extern int xq_amixchn_getstatus(int AmixGrp, int AmixChn, xq_amixchn_status_s *pstAoutStatus);
// AOUT
typedef struct xq_aout_param xq_aout_param_s;
typedef struct xq_aout_status xq_aout_status_s;
extern int xq_aout_create(int *pAoutChn, const xq_aout_param_s *pstAoutChnParam);
extern int xq_aout_destroy(int AoutChn);
extern int xq_aout_getparam(int AoutChn, xq_aout_param_s *pstAoutChnParam);
extern int xq_aout_setparam(int AoutChn, const xq_aout_param_s *pstAoutChnParam);
extern int xq_aout_getstatus(int AoutChn, xq_aout_status_s *pstAoutStatus);


int xq_conv(DL_MOD_E src_mod, const void *src_param, DL_MOD_E dst_mod, void *dst_param);

#if defined(__cplusplus)
}
#endif

#endif /* _XQ_CONV_H_ */
