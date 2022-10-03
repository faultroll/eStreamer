
#include "xquant.h"

#ifndef _BASIC_VDEC_H_
#define _BASIC_VDEC_H_

#if defined(__cplusplus)
extern "C" {
#endif

// used in new_outchn
typedef struct _basic_vdec_outparam_s_ {
    bool playback;
    xq_callback_s dec_cb;
    // xq_callback_s raw_cb; // no need, vdec output raw
    // xq_usrpic_set_param_s ;
} basic_vdec_outparam_s;

#if defined(__cplusplus)
}
#endif

#endif /* _BASIC_VDEC_H_ */
