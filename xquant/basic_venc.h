
#include "xquant.h" // FIX(lgY): header file sequence problem

#ifndef _BASIC_VENC_H_
#define _BASIC_VENC_H_

#if defined(__cplusplus)
extern "C" {
#endif

// TODO(lgY): maybe we can do this in custom_ctrl like XQ_OPER_CALLBACK_SET
// used in new_inchn
typedef struct _basic_venc_inparam_s_ {
    xq_callback_s raw_cb;
    xq_callback_s enc_cb;
} basic_venc_inparam_s;

#if defined(__cplusplus)
}
#endif

#endif /* _BASIC_VENC_H_ */
