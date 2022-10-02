
#ifndef _XQ_CONV_H_
#define _XQ_CONV_H_

#include "dlink/dlink.h" // DL_MOD_E

#if defined(__cplusplus)
extern "C" {
#endif

// DL_VIN
typedef struct xq_vin_param xq_vin_param_s;
typedef struct xq_vin_status xq_vin_status_s;
extern int xq_vin_create_chn(int *pVinChn, const xq_vin_param_s *pstVinChnParam);
extern int xq_vin_destroy_chn(int VinChn);
extern int xq_vin_get_chnparam(int VinChn, xq_vin_param_s *pstVinChnParam);
extern int xq_vin_set_chnparam(int VinChn, const xq_vin_param_s *pstVinChnParam);
extern int xq_vin_get_chnstatus(int VinChn, xq_vin_status_s *pstVinStatus);

int xq_conv(DL_MOD_E src_mod, const void *src_param, DL_MOD_E dst_mod, void *dst_param);

#if defined(__cplusplus)
}
#endif

#endif /* _XQ_CONV_H_ */
