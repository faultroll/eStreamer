
#include "basic_ain.h"
#include "basic_adec.h"

#ifndef _ADVANCED_AIN_H_
#define _ADVANCED_AIN_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* typedef struct _advanced_ain_bdlparam_s_
{
    bool bypass;
} advanced_ain_bdlparam_s; */

typedef struct _advanced_ain_outparam_s_ {
    xq_chn_type_e type;
    // union
    // basic_ain_outparam_s local;
    basic_adec_outparam_s net;
} advanced_ain_outparam_s;

#if defined(__cplusplus)
}
#endif

#endif /* _ADVANCED_AIN_H_ */
