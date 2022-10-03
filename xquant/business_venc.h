
#include "advanced_vin.h"
#include "advanced_vmulout.h"

#ifndef _BUSINESS_VENC_H_
#define _BUSINESS_VENC_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* typedef struct _business_venc_outparam_s_
{
    advanced_vmulout_outparam_s ;
} business_venc_outparam_s; */
// simple?
// typedef advanced_vin_outparam_s business_venc_inparam_s;
typedef advanced_vmulout_outparam_s business_venc_outparam_s;

// param
// inchn-xq_vin_param_s(local)
// outchn-xq_vout_param_s(local)/xq_venc_param_s(net)

#if defined(__cplusplus)
}
#endif

#endif /* _BUSINESS_VENC_H_ */
