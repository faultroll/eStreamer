
#include "basic_vin.h"
#include "basic_vdec.h"

#ifndef _ADVANCED_VIN_H_
#define _ADVANCED_VIN_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* typedef struct _advanced_vin_bdlparam_s_
{
    bool bypass;
} advanced_vin_bdlparam_s; */

// used in new_outchn
typedef struct _advanced_vin_outparam_s_ {
    xq_chn_type_e type; // outchn type
    // union
    // basic_vin_outparam_s local;
    basic_vdec_outparam_s net;
} advanced_vin_outparam_s;

#if defined(__cplusplus)
}
#endif

#endif /* _ADVANCED_VIN_H_ */
