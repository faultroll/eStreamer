
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
// VOUT
typedef struct xq_vout_param xq_vout_param_s;
typedef struct xq_vout_status xq_vout_status_s;
extern int xq_vout_create(int *pVoutChn, const xq_vout_param_s *pstVoutChnParam);
extern int xq_vout_destroy(int VoutChn);
extern int xq_vout_getparam(int VoutChn, xq_vout_param_s *pstVoutChnParam);
extern int xq_vout_setparam(int VoutChn, const xq_vout_param_s *pstVoutChnParam);
extern int xq_vout_getstatus(int VoutChn, xq_vout_status_s *pstVoutStatus);


int xq_conv(DL_MOD_E src_mod, const void *src_param, DL_MOD_E dst_mod, void *dst_param);

#if defined(__cplusplus)
}
#endif

#endif /* _XQ_CONV_H_ */
