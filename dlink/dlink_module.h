
#ifndef _DLINK_MODULE_H_
#define _DLINK_MODULE_H_

#include <stddef.h> // size_t
#include <stdint.h> // uint64_t
#include "dlink.h"  // DL_ID_S

#if defined(__cplusplus)
extern "C" {
#endif

// info
extern size_t info_maxsize(void);
extern uint64_t info_timeus(void);


// video
extern int wrapper_vin_outdata(DL_ID_S id,  void *data);
extern int wrapper_vin_freedata(DL_ID_S id, void *data);

extern int wrapper_venc_indata(DL_ID_S id, void *data);
extern int wrapper_venc_outdata(DL_ID_S id,  void *data);
extern int wrapper_venc_freedata(DL_ID_S id, void *data);

extern int wrapper_vdec_indata(DL_ID_S id, void *data);
extern int wrapper_vdec_outdata(DL_ID_S id,  void *data);
extern int wrapper_vdec_freedata(DL_ID_S id, void *data);

extern int wrapper_vmul_indata(DL_ID_S id, void *data);
extern int wrapper_vmul_outdata(DL_ID_S id, void *data);
extern int wrapper_vmul_freedata(DL_ID_S id, void *data);

extern int wrapper_vmix_indata(DL_ID_S id, void *data);
extern int wrapper_vmix_outdata(DL_ID_S id, void *data);
extern int wrapper_vmix_freedata(DL_ID_S id,  void *data);

extern int wrapper_vout_indata(DL_ID_S id, void *data);


// audio
extern int wrapper_ain_outdata(DL_ID_S id, void *data);
extern int wrapper_ain_freedata(DL_ID_S id, void *data);

extern int wrapper_aenc_indata(DL_ID_S id, void *data);
extern int wrapper_aenc_outdata(DL_ID_S id, void *data);
extern int wrapper_aenc_freedata(DL_ID_S id, void *data);

extern int wrapper_adec_indata(DL_ID_S id, void *data);
extern int wrapper_adec_outdata(DL_ID_S id, void *data);
extern int wrapper_adec_freedata(DL_ID_S id, void *data);

extern int wrapper_amul_indata(DL_ID_S id, void *data);
extern int wrapper_amul_outdata(DL_ID_S id, void *data);
extern int wrapper_amul_freedata(DL_ID_S id,  void *data);

extern int wrapper_amix_indata(DL_ID_S id, void *data);
extern int wrapper_amix_outdata(DL_ID_S id, void *data);
extern int wrapper_amix_freedata(DL_ID_S id, void *data);

extern int wrapper_aout_indata(DL_ID_S id, void *data);

#if defined(__cplusplus)
}
#endif

#endif // _DLINK_MODULE_H_
