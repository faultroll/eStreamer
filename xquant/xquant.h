
#ifndef _XQUANT_H_
#define _XQUANT_H_

#include "dlink/dlink.h" // DL_ID_S
#include "dlink/dlink_cvt.h" // DL_PUB_DATAINFO_S

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum _xq_chn_dir_e_ {
    XQ_CHN_IN, //= 1 << 0, // bitmask, can a channel have two types?
    XQ_CHN_OUT,//= 1 << 1,
} xq_chn_dir_e;
typedef enum _xq_chn_type_e_ {
    XQ_CHN_LOCAL,  // nearend, direct to local screen; a/vframe
    XQ_CHN_NET,    // farend, encode to net; a/vstream
    XQ_CHN_BYPASS, // ; a/vframe, a/vstream
} xq_chn_type_e;
typedef enum _xq_chn_oper_e_ {
    XQ_OPER_UPDATE_OSD,
    XQ_OPER_FORCE_IFRAME,
    XQ_OPER_USRPIC_SET,
    XQ_OPER_USRPIC_SWITCH,
    // XQ_OPER_CAPTURE_YUV,
    // XQ_OPER_EPTZ_CTRL,
    XQ_OPER_BUTT,
} xq_chn_oper_e;
// typedef struct _xq_inchn_hnd_s_ xq_inchn_hnd_s;
// typedef struct _xq_outchn_hnd_s_ xq_outchn_hnd_s;
typedef struct _xq_chn_s_ xq_chn_s;
struct _xq_chn_s_ {
    xq_chn_dir_e dir;
    xq_chn_type_e type;
    int (*create_chn)(xq_chn_s *self, const void *param); // self, this pointer
    int (*destroy_chn)(xq_chn_s *self);
    int (*set_param)(xq_chn_s *self, const void *param);
    int (*get_param)(xq_chn_s *self, void *param);
    int (*get_status)(xq_chn_s *self, void *param);
    int (*custom_ctrl)(xq_chn_s *self, const int oper, void *param); // xq-oper

    xq_chn_s *bind_chn;
    DL_ID_S bind_id;
    xq_chn_s *target_chn;
    DL_ID_S target_id;
    DL_DES_S des; // indata(send), outdata(get), freedata(release)

    // xq_bdl_s *parent;

    void *priv; // priv, private data, xq_inchn_hnd_s/xq_outchn_hnd_s
};

typedef enum _xq_type_e_ {
    XQ_BASIC_BEGIN = 1,
    // video
    XQ_BASIC_VIN,
    XQ_BASIC_VOUT,
    XQ_BASIC_VMUL,
    XQ_BASIC_VMIX,
    XQ_BASIC_VENC,
    XQ_BASIC_VDEC,
    // audio
    XQ_BASIC_AIN,
    XQ_BASIC_AOUT,
    XQ_BASIC_AMUL,
    XQ_BASIC_AMIX,
    XQ_BASIC_AENC,
    XQ_BASIC_ADEC,

    XQ_ADVANCED_BEGIN = 101,
    // video
    XQ_ADVANCED_VIN,
    XQ_ADVANCED_VMULOUT, // XQ_ADVANCED_VOUT,
    // audio
    XQ_ADVANCED_AIN,
    XQ_ADVANCED_AMULOUT, // XQ_ADVANCED_AOUT,

    // only for demo/test
    XQ_BUSINESS_BEGIN = 201,
    // video
    XQ_BUSINESS_VENC,
    XQ_BUSINESS_VDEC,
    // audio
    XQ_BUSINESS_AENC,
    XQ_BUSINESS_ADEC,

} xq_type_e;
// typedef struct _xq_bdl_hnd_s_ xq_bdl_hnd_s;
typedef struct _xq_bdl_s_ xq_bdl_s;
struct _xq_bdl_s_ {
    xq_type_e type;
    xq_chn_s *(*new_inchn)(xq_bdl_s *self, const void *param); // self, this pointer
    xq_chn_s *(*new_outchn)(xq_bdl_s *self, const void *param);
    void (*delete_chn)(xq_bdl_s *self, xq_chn_s *chn);

    void *priv; // priv, private data, xq_bdl_hnd_s
};

// TODO(lgY): make this private
typedef struct _xq_init_s_ {
    // xq_type_e type;
    int (*init_xquant)(void);
    int (*fini_xquant)(void);
    xq_bdl_s *(*new_xquant)(const void *);
    void (*delete_xquant)(xq_bdl_s *);
} xq_init_s;

int xquant_init(void);
int xquant_fini(void);
xq_bdl_s *xquant_new(xq_type_e type, const void *param);
void xquant_delete(xq_bdl_s *bdl);
void xquant_bind(xq_chn_s *chn1, xq_chn_s *chn2);
void xquant_unbind(xq_chn_s *chn1, xq_chn_s *chn2);

#if defined(__cplusplus)
}
#endif

// common param
typedef struct _xq_callback_s_ {
    void *opaque;
    int (*dataprocfun)(void *opaque, xq_chn_s *chn, /* const */ DL_PUB_DATAINFO_S *data);
} xq_callback_s;

// module param
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

#endif // _XQUANT_H_
