
#ifndef _ELINK_MODULE_H_
#define _ELINK_MODULE_H_

#include <stddef.h> // size_t
#include <stdint.h> // uint64_t
#include "elink.h"  // EL_ID_S

#if defined(__cplusplus)
extern "C" {
#endif

// info
extern size_t info_maxsize(void);
extern uint64_t info_timeus(void);


// video
extern int wrapper_vin_out(EL_ID_S id,  void *data);
extern int wrapper_vin_release(EL_ID_S id, void *data);

extern int wrapper_venc_in(EL_ID_S id, void *data);
extern int wrapper_venc_out(EL_ID_S id,  void *data);
extern int wrapper_venc_release(EL_ID_S id, void *data);

extern int wrapper_vdec_in(EL_ID_S id, void *data);
extern int wrapper_vdec_out(EL_ID_S id,  void *data);
extern int wrapper_vdec_release(EL_ID_S id, void *data);

extern int wrapper_vmul_in(EL_ID_S id, void *data);
extern int wrapper_vmul_out(EL_ID_S id, void *data);
extern int wrapper_vmul_release(EL_ID_S id, void *data);

extern int wrapper_vmix_in(EL_ID_S id, void *data);
extern int wrapper_vmix_out(EL_ID_S id, void *data);
extern int wrapper_vmix_release(EL_ID_S id,  void *data);

extern int wrapper_vout_in(EL_ID_S id, void *data);


// audio
extern int wrapper_ain_out(EL_ID_S id, void *data);
extern int wrapper_ain_release(EL_ID_S id, void *data);

extern int wrapper_aenc_in(EL_ID_S id, void *data);
extern int wrapper_aenc_out(EL_ID_S id, void *data);
extern int wrapper_aenc_release(EL_ID_S id, void *data);

extern int wrapper_adec_in(EL_ID_S id, void *data);
extern int wrapper_adec_out(EL_ID_S id, void *data);
extern int wrapper_adec_release(EL_ID_S id, void *data);

extern int wrapper_amul_in(EL_ID_S id, void *data);
extern int wrapper_amul_out(EL_ID_S id, void *data);
extern int wrapper_amul_release(EL_ID_S id,  void *data);

extern int wrapper_amix_in(EL_ID_S id, void *data);
extern int wrapper_amix_out(EL_ID_S id, void *data);
extern int wrapper_amix_release(EL_ID_S id, void *data);

extern int wrapper_aout_in(EL_ID_S id, void *data);

#if defined(__cplusplus)
}
#endif

#endif // _ELINK_MODULE_H_
