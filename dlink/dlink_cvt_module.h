
#ifndef _DLINK_CVT_MODULE_H
#define _DLINK_CVT_MODULE_H

#include "dlink.h"
#include "dlink_cvt.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define MAGIC_VFRAME   ('v' << 24 | 'f' << 16 | 'm' << 8 | '\0' << 0)
#define MAGIC_VSTREAM  ('v' << 24 | 's' << 16 | 'm' << 8 | '\0' << 0)
#define MAGIC_AFRAME   ('a' << 24 | 'f' << 16 | 'm' << 8 | '\0' << 0)
#define MAGIC_ASTREAM  ('a' << 24 | 's' << 16 | 'm' << 8 | '\0' << 0)

typedef enum OPAQUE_STAT {
    QPAQUE_PRIVATE_FILLED = 1 << 0,
    QPAQUE_PUBLIC_FILLED  = 1 << 1,
} DL_OPAQUE_STAT_E;

// DL_DATATYPE_VFRAME
typedef struct OPAQUE_VFRAMEINFO {
    int                 s32Magic;
    DL_MOD_E            enMod;
    DL_OPAQUE_STAT_E    enStat;
    struct {
        union {
            // module structs
        };
        struct {
            // user infos
        };
    } private;
    DL_PUB_DATAINFO_S public;
} DL_OPAQUE_VFRAMEINFO_S;
extern int wrapper_import_vframe(DL_PUB_DATAINFO_S *in, DL_OPAQUE_VFRAMEINFO_S *out);
extern int wrapper_freepriv_vframe(DL_OPAQUE_VFRAMEINFO_S *out);
extern int wrapper_export_vframe(DL_OPAQUE_VFRAMEINFO_S *in, DL_PUB_DATAINFO_S *out);
extern int wrapper_freepub_vframe(DL_PUB_DATAINFO_S *out);
// DL_DATATYPE_VSTREAM
typedef struct OPAQUE_VSTREAMINFO {
    int                 s32Magic;
    DL_MOD_E            enMod;
    DL_OPAQUE_STAT_E    enStat;
    struct {
        DL_ENCTYPE_E    enEncType;
        union {
            // module structs
        };
        struct {
            // user infos
        };
    } private;
    DL_PUB_DATAINFO_S public;
} DL_OPAQUE_VSTREAMINFO_S;
extern int wrapper_import_vstream(DL_PUB_DATAINFO_S *in, DL_OPAQUE_VSTREAMINFO_S *out);
extern int wrapper_freepriv_vstream(DL_OPAQUE_VSTREAMINFO_S *out);
extern int wrapper_export_vstream(DL_OPAQUE_VSTREAMINFO_S *in, DL_PUB_DATAINFO_S *out);
extern int wrapper_freepub_vstream(DL_PUB_DATAINFO_S *out);
// DL_DATATYPE_AFRAME
typedef struct OPAQUE_AFRAMEINFO {
    int                 s32Magic;
    DL_MOD_E            enMod;
    DL_OPAQUE_STAT_E    enStat;
    struct {
        union {
            // module structs
        };
        struct {
            // user infos
        };
    } private;
    DL_PUB_DATAINFO_S public;
} DL_OPAQUE_AFRAMEINFO_S;
extern int wrapper_import_aframe(DL_PUB_DATAINFO_S *in, DL_OPAQUE_AFRAMEINFO_S *out);
extern int wrapper_freepriv_aframe(DL_OPAQUE_AFRAMEINFO_S *out);
extern int wrapper_export_aframe(DL_OPAQUE_AFRAMEINFO_S *in, DL_PUB_DATAINFO_S *out);
extern int wrapper_freepub_aframe(DL_PUB_DATAINFO_S *out);
// DL_DATATYPE_ASTREAM
typedef struct OPAQUE_ASTREAMINFO {
    int                 s32Magic;
    DL_MOD_E            enMod;
    DL_OPAQUE_STAT_E    enStat;
    struct {
        DL_ENCTYPE_E    enEncType;
        union {
            // module structs
        };
        struct {
            // user infos
        };
    } private;
    DL_PUB_DATAINFO_S public;
} DL_OPAQUE_ASTREAMINFO_S;
extern int wrapper_import_astream(DL_PUB_DATAINFO_S *in, DL_OPAQUE_ASTREAMINFO_S *out);
extern int wrapper_freepriv_astream(DL_OPAQUE_ASTREAMINFO_S *out);
extern int wrapper_export_astream(DL_OPAQUE_ASTREAMINFO_S *in, DL_PUB_DATAINFO_S *out);
extern int wrapper_freepub_astream(DL_PUB_DATAINFO_S *out);

#if defined(__cplusplus)
}
#endif

#endif // _DLINK_CVT_MODULE_H
