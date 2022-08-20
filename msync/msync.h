
#ifndef _MSYNC_H_
#define _MSYNC_H_

#include "elink/elink.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum _ms_chn_dir_e_ {
    MS_CHN_IN, //= 1 << 0, // bitmask, can a channel have two types?
    MS_CHN_OUT,//= 1 << 1,
} ms_chn_dir_e;
typedef enum _ms_chn_type_e_ {
    MS_CHN_LOCAL,  // nearend, direct to local screen; a/vframe
    MS_CHN_NET,    // farend, encode to net; a/vstream
    MS_CHN_BYPASS, // ; a/vframe, a/vstream
} ms_chn_type_e;
typedef enum _ms_chn_oper_e_ {
    MS_OPER_UPDATE_OSD,
    MS_OPER_FORCE_IFRAME,
    MS_OPER_USRPIC_SET,
    MS_OPER_USRPIC_SWITCH,
    // MS_OPER_CAPTURE_YUV,
    // MS_OPER_EPTZ_CTRL,
    MS_OPER_BUTT,
} ms_chn_oper_e;
#define MS_ID_EMPTY    { .mod = -1, .grp = -1, .chn = -1, }
// typedef struct _ms_inchn_hnd_s_ ms_inchn_hnd_s;
// typedef struct _ms_outchn_hnd_s_ ms_outchn_hnd_s;
typedef struct _ms_chn_s_ ms_chn_s;
struct _ms_chn_s_ {
    ms_chn_dir_e dir;
    ms_chn_type_e type;
    int (*create_chn)(ms_chn_s *self, const void *param); // self, this pointer
    int (*destroy_chn)(ms_chn_s *self);
    int (*set_param)(ms_chn_s *self, const void *param);
    int (*get_param)(ms_chn_s *self, void *param);
    int (*get_status)(ms_chn_s *self, void *param);
    int (*custom_ctrl)(ms_chn_s *self, const int oper, void *param); // ms-oper

    ms_chn_s *bind_chn;
    EL_ID_S bind_id;
    ms_chn_s *target_chn;
    EL_ID_S target_id;
    EL_DES_S des; // indata(send), outdata(get), freedata(release)

    // ms_bdl_s *parent;

    void *priv; // priv, private data, ms_inchn_hnd_s/ms_outchn_hnd_s
};

typedef enum _ms_type_e_ {
    MS_BASIC_BEGIN = 1,
    // video
    MS_BASIC_VIN,
    MS_BASIC_VOUT,
    MS_BASIC_VMUL,
    MS_BASIC_VMIX,
    MS_BASIC_VENC,
    MS_BASIC_VDEC,
    // audio
    MS_BASIC_AIN,
    MS_BASIC_AOUT,
    MS_BASIC_AMUL,
    MS_BASIC_AMIX,
    MS_BASIC_AENC,
    MS_BASIC_ADEC,

    MS_ADVANCED_BEGIN = 101,
    // video
    MS_ADVANCED_VIN,
    MS_ADVANCED_VMULOUT, // MS_ADVANCED_VOUT,
    // audio
    MS_ADVANCED_AIN,
    MS_ADVANCED_AMULOUT, // MS_ADVANCED_AOUT,

    // only for demo/test
    MS_BUSINESS_BEGIN = 201,
    // video
    MS_BUSINESS_VENC,
    MS_BUSINESS_VDEC,
    // audio
    MS_BUSINESS_AENC,
    MS_BUSINESS_ADEC,

} ms_type_e;
// typedef struct _ms_bdl_hnd_s_ ms_bdl_hnd_s;
typedef struct _ms_bdl_s_ ms_bdl_s;
struct _ms_bdl_s_ {
    ms_type_e type;
    ms_chn_s *(*new_inchn)(ms_bdl_s *self, const void *param); // self, this pointer
    ms_chn_s *(*new_outchn)(ms_bdl_s *self, const void *param);
    void (*delete_chn)(ms_bdl_s *self, ms_chn_s *chn);

    void *priv; // priv, private data, ms_hnd_s
};

// TODO(lgY): make this private
typedef struct _ms_init_s_ {
    // ms_type_e type;
    int (*init_msync)(void);
    int (*fini_msync)(void);
    ms_bdl_s *(*new_msync)(const void *);
    void (*delete_msync)(ms_bdl_s *);
} ms_init_s;

int msync_init(void);
int msync_fini(void);
ms_bdl_s *msync_new(ms_type_e type, const void *param);
void msync_delete(ms_bdl_s *hnd);
void msync_bind(ms_chn_s *chn1, ms_chn_s *chn2);
void msync_unbind(ms_chn_s *chn1, ms_chn_s *chn2);

#if defined(__cplusplus)
}
#endif

// common params
typedef struct _ms_callback_s_ {
    void *opaque;
    int (*dataprocfun)(void *opaque, ms_chn_s *chn, /* const */ MS_DATAINFO_S *data);
} ms_callback_s;

// params
/* #include "basic_vin.h"
#include "basic_vout.h"
#include "basic_vmul.h"
#include "basic_vmix.h"
#include "basic_venc.h"
#include "basic_vdec.h"
#include "basic_ain.h"
#include "basic_aout.h"
#include "basic_amul.h"
#include "basic_amix.h"
#include "basic_aenc.h"
#include "basic_adec.h"
#include "advanced_vin.h"
#include "advanced_vmulout.h"
#include "advanced_ain.h"
#include "advanced_amulout.h"
#include "business_venc.h"
#include "business_vdec.h"
#include "business_aenc.h"
#include "business_adec.h" */

#endif // _MSYNC_H_
