
#include "basic_vmul.h"
#include "basic_venc.h"
#include "basic_vout.h"

#ifndef _ADVANCED_VMULOUT_H_
#define _ADVANCED_VMULOUT_H_

#if defined(__cplusplus)
extern "C" {
#endif

/* typedef struct _advanced_vmulout_bdlparam_s_
{
    bool bypass; // bypass basic_vmul, 1-to-1
} advanced_vmulout_bdlparam_s; */

// used in new_outchn
typedef struct _advanced_vmulout_outparam_s_ {
    xq_chn_type_e type; // outchn type
    // union
    // basic_vout_inparam_s local;
    basic_venc_inparam_s net;
    struct {
        xq_callback_s raw_cb;
    } bypass;
} advanced_vmulout_outparam_s;

#if defined(__cplusplus)
}
#endif

#endif /* _ADVANCED_VMULOUT_H_ */
