
#ifndef _ELINK_MODULE_H_
#define _ELINK_MODULE_H_

#include <stddef.h> // size_t
#include <stdint.h> // uint64_t
#include "elink.h"  // ELINK_ID_S

#if defined(__cplusplus)
extern "C" {
#endif

// info
extern size_t info_maxsize(void);
extern uint64_t info_timeus(void);


// video
extern int wrapper_vin_out(ELINK_ID_S id,  void *data);
extern int wrapper_vin_release(ELINK_ID_S id, void *data);

extern int wrapper_venc_in(ELINK_ID_S id, void *data);
extern int wrapper_venc_out(ELINK_ID_S id,  void *data);
extern int wrapper_venc_release(ELINK_ID_S id, void *data);

extern int wrapper_vdec_in(ELINK_ID_S id, void *data);
extern int wrapper_vdec_out(ELINK_ID_S id,  void *data);
extern int wrapper_vdec_release(ELINK_ID_S id, void *data);

extern int wrapper_vmul_in(ELINK_ID_S id, void *data);
extern int wrapper_vmul_out(ELINK_ID_S id, void *data);
extern int wrapper_vmul_release(ELINK_ID_S id, void *data);

extern int wrapper_vmix_in(ELINK_ID_S id, void *data);
extern int wrapper_vmix_out(ELINK_ID_S id, void *data);
extern int wrapper_vmix_release(ELINK_ID_S id,  void *data);

extern int wrapper_vout_in(ELINK_ID_S id, void *data);


// audio
extern int wrapper_ain_out(ELINK_ID_S id, void *data);
extern int wrapper_ain_release(ELINK_ID_S id, void *data);

extern int wrapper_aenc_in(ELINK_ID_S id, void *data);
extern int wrapper_aenc_out(ELINK_ID_S id, void *data);
extern int wrapper_aenc_release(ELINK_ID_S id, void *data);

extern int wrapper_adec_in(ELINK_ID_S id, void *data);
extern int wrapper_adec_out(ELINK_ID_S id, void *data);
extern int wrapper_adec_release(ELINK_ID_S id, void *data);

extern int wrapper_amul_in(ELINK_ID_S id, void *data);
extern int wrapper_amul_out(ELINK_ID_S id, void *data);
extern int wrapper_amul_release(ELINK_ID_S id,  void *data);

extern int wrapper_amix_in(ELINK_ID_S id, void *data);
extern int wrapper_amix_out(ELINK_ID_S id, void *data);
extern int wrapper_amix_release(ELINK_ID_S id, void *data);

extern int wrapper_aout_in(ELINK_ID_S id, void *data);

#if defined(__cplusplus)
}
#endif

#endif /* _ELINK_MODULE_H_ */
