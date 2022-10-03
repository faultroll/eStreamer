
#include "basic_amul.h"
#include "basic_aenc.h"
#include "basic_aout.h"

#ifndef _ADVANCED_AMULOUT_H_
#define _ADVANCED_AMULOUT_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* typedef struct _advanced_amulout_bdlparam_s_
{
    bool bypass;
} advanced_amulout_bdlparam_s; */

typedef struct _advanced_amulout_outparam_s_ {
    xq_chn_type_e type; // outchn type
    // union
    // basic_aout_inparam_s local;
    basic_aenc_inparam_s net;
} advanced_amulout_outparam_s;

#if defined(__cplusplus)
}
#endif

#endif /* _ADVANCED_AMULOUT_H_ */
