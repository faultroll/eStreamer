
#ifndef _XQ_CONV_H_
#define _XQ_CONV_H_

#include "dlink/dlink.h" // DL_MOD_E

#if defined(__cplusplus)
extern "C" {
#endif

// VIN
typedef struct xq_vin_param xq_vin_param_s;
typedef struct xq_vin_status xq_vin_status_s;
extern int xq_vin_create_chn(int *pVinChn, const xq_vin_param_s *pstVinChnParam);
extern int xq_vin_destroy_chn(int VinChn);
extern int xq_vin_get_chnparam(int VinChn, xq_vin_param_s *pstVinChnParam);
extern int xq_vin_set_chnparam(int VinChn, const xq_vin_param_s *pstVinChnParam);
extern int xq_vin_get_chnstatus(int VinChn, xq_vin_status_s *pstVinStatus);
// VOUT
typedef struct xq_vout_param xq_vout_param_s;
typedef struct xq_vout_status xq_vout_status_s;
extern int xq_vout_create_chn(int *pVoutChn, const xq_vout_param_s *pstVoutChnParam);
extern int xq_vout_destroy_chn(int VoutChn);
extern int xq_vout_get_chnparam(int VoutChn, xq_vout_param_s *pstVoutChnParam);
extern int xq_vout_set_chnparam(int VoutChn, const xq_vout_param_s *pstVoutChnParam);
extern int xq_vout_get_chnstatus(int VoutChn, xq_vout_status_s *pstVoutStatus);


int xq_conv(DL_MOD_E src_mod, const void *src_param, DL_MOD_E dst_mod, void *dst_param);

#if defined(__cplusplus)
}
#endif

#endif /* _XQ_CONV_H_ */
