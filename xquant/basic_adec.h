
#include "xquant.h"

#ifndef _BASIC_ADEC_H_
#define _BASIC_ADEC_H_

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _basic_adec_outparam_s_ {
    bool playback;
    xq_callback_s dec_cb;
} basic_adec_outparam_s;

#if defined(__cplusplus)
}
#endif

#endif /* _BASIC_ADEC_H_ */
