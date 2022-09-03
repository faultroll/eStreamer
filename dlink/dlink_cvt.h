
#ifndef _DLINK_CVT_H_
#define _DLINK_CVT_H_

#include <stdbool.h>
#include <stdint.h>
#include "dlink/dlink.h" // DL_TYPE_E

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _dl_resolution_s_ {
    uint32_t u32Width;
    uint32_t u32Height;
} DL_RESOLUTION_S;

typedef enum _dl_enctype_e_ {
    // video
    DL_VIDEO_YUV = 0, // not encoded
    DL_VIDEO_H264,
    DL_VIDEO_H265,
    DL_VIDEO_MJPEG, // JPEG using this
    // audio
    DL_AUDIO_PCM = 100, // not encoded
    DL_AUDIO_G711A,
    DL_AUDIO_G711U,
    DL_AUDIO_AAC,
    DL_AUDIO_G722,
    DL_AUDIO_G726,
} DL_ENCTYPE_E;

// TODO(lgY): use fourcc
typedef enum _dl_pixelformat_e_ {
    // video
    DL_PIXELFORMAT_YUV420SP,
    // audio
    DL_PIXELFORMAT_MONO, // left single
    DL_PIXELFORMAT_STEREO, // left-right packed, current not support
} DL_PIXELFORMAT_E;

typedef enum _dl_bitwidth_e_ {
    DL_BITWIDTH_8  = 8,    // current video
    DL_BITWIDTH_10 = 10,   // 10-bit video
    // DL_BITWIDTH_12 = 12,
    // DL_BITWIDTH_14 = 14,
    DL_BITWIDTH_16 = 16,    // current audio
    // DL_BITWIDTH_24 = 24,
    DL_BITWIDTH_32 = 32,    // 32-bit audio
} DL_BITWIDTH_E;

typedef enum _dl_samplerate_e_ {
    // 48 series
    DL_SAMPLERATE_8000   = 8000,        /* 8K samplerate*/
    //DL_SAMPLERATE_12000  = 12000,     /* 12K samplerate*/
    DL_SAMPLERATE_16000  = 16000,       /* 16K samplerate*/
    //DL_SAMPLERATE_24000  = 24000,     /* 24K samplerate*/
    DL_SAMPLERATE_32000  = 32000,       /* 32K samplerate*/
    DL_SAMPLERATE_48000  = 48000,       /* 48K samplerate*/
    //DL_SAMPLERATE_64000  = 64000,     /* 64K samplerate*/
    //DL_SAMPLERATE_96000  = 96000,     /* 96K samplerate*/
    // 44.1 series
    //DL_SAMPLERATE_11025  = 11025,     /* 11.025K samplerate*/
    //DL_SAMPLERATE_22050  = 22050,     /* 22.050K samplerate*/
    DL_SAMPLERATE_44100  = 44100,       /* 44.1K samplerate*/
} DL_SAMPLERATE_E;

typedef struct _dl_pub_datainfo_s_ {
    // buffer
    void        *pVirAddr; // convert to bitwidth (eg. DL_BITWIDTH_16 audio, then use pu16VirAddr)
    uintptr_t    ipPhyAddr;
    uint32_t     u32Len;
    // timestamp
    uint64_t     u64Pts; // if 0, will be generated in some module
    uint32_t     u32Seq; // if 0, will be generated in some module
    // info
    DL_TYPE_E    enDataType;
    DL_ENCTYPE_E enEncType;
    union {
        struct {
            DL_PIXELFORMAT_E       enPixelFmt;
            DL_RESOLUTION_S        stSize;
            DL_BITWIDTH_E          enBitwidth;
        } stVFrameInfo;
        struct {
            bool                   bIsKey;
        } stVStreamInfo;
        struct {
            DL_PIXELFORMAT_E       enSoundmode;
            DL_BITWIDTH_E          enBitwidth;
        } stAFrameInfo;
        struct {
            DL_PIXELFORMAT_E       enSoundmode;
            DL_BITWIDTH_E          enBitwidth;
            DL_SAMPLERATE_E        enSampleRate;
            uint32_t               u32PtNumPerFrm;
        } stAStreamInfo;
    };

} DL_PUB_DATAINFO_S;


typedef struct _dl_vframeinfo_s_ {
    void    *priv;
} DL_VFRAMEINFO_S;
typedef struct _dl_vstreaminfo_s_ {
    void    *priv;
} DL_VSTREAMINFO_S;
typedef struct _dl_aframeinfo_s_ {
    void    *priv;
} DL_AFRAMEINFO_S;
typedef struct _dl_astreaminfo_s_ {
    void    *priv;
} DL_ASTREAMINFO_S;
typedef union _dl_priv_datainfo_s_ {
    DL_VFRAMEINFO_S   stVFrameInfo;
    DL_VSTREAMINFO_S  stVStreamInfo;
    DL_AFRAMEINFO_S   stAFrameInfo;
    DL_ASTREAMINFO_S  stAStreamInfo;
} DL_PRIV_DATAINFO_U;

int dlink_cvt_import(/* const */ DL_PUB_DATAINFO_S *src, DL_PRIV_DATAINFO_U *dst); // get
int dlink_cvt_release(DL_PRIV_DATAINFO_U *dst); // release
int dlink_cvt_export(/* const */ DL_PRIV_DATAINFO_U *src, DL_PUB_DATAINFO_S *dst); // send

#if defined(__cplusplus)
}
#endif

#endif // _DLINK_CVT_H_
