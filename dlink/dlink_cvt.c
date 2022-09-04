
#include "dlink_cvt.h"
#include "dlink_cvt_module.h"
#include <stdio.h>
#include <stdlib.h> // malloc/free
#include <string.h> // memcpy

// check
#define DL_CHECK(condition)                         \
    do {                                            \
        if (!(condition)) {                         \
            printf("check failed: " #condition);    \
            return -1;                              \
        }                                           \
    } while (0)
// flag
#define DL_SETBIT(_flag, _bit) ((_flag) |= (_bit))
#define DL_CLRBIT(_flag, _bit) ((_flag) &= (~(_bit)))
#define DL_HASBIT(_flag, _bit) ((_flag) & (_bit))
#define DL_SETMAGIC(_ptr, _magic) ((*(int *)(_ptr)) = (_magic))
#define DL_CLRMAGIC(_ptr) ((*(int *)(_ptr)) = (0xdeadbeaf))
#define DL_HASMAGIC(_ptr, _magic) ((_magic) == (*(int *)(_ptr)))


int dlink_cvt_import(/* const */ DL_PUB_DATAINFO_S *src, DL_PRIV_DATAINFO_U *dst)
{
    DL_CHECK(src);
    DL_CHECK(dst);

    int ret;

    if (DL_VFRAME == src->enDataType) {
        DL_OPAQUE_VFRAMEINFO_S *opaque = malloc(sizeof(DL_OPAQUE_VFRAMEINFO_S));
        DL_CHECK(opaque);
        DL_SETMAGIC(opaque, MAGIC_VFRAME);
        opaque->enMod = DL_USER;
        ret = wrapper_import_vframe(src, opaque);
        if (0 != ret) {
            printf("wrapper_import_vframe failed with ret(%#x).\n", ret);
            free(opaque);
            dst->stVFrameInfo.pOpaque = NULL;
            return -1;
        }
        DL_SETBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED);
        DL_CLRBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        dst->stVFrameInfo.pOpaque = opaque;
    } else if (DL_VSTREAM == src->enDataType) {
        DL_OPAQUE_VSTREAMINFO_S *opaque = malloc(sizeof(DL_OPAQUE_VSTREAMINFO_S));
        DL_CHECK(opaque);
        DL_SETMAGIC(opaque, MAGIC_VSTREAM);
        opaque->enMod = DL_USER;
        opaque->private.enEncType = src->enEncType;
        ret = wrapper_import_vstream(src, opaque);
        if (0 != ret) {
            printf("wrapper_import_vstream failed with ret(%#x).\n", ret);
            free(opaque);
            dst->stVStreamInfo.pOpaque = NULL;
            return -1;
        }
        DL_SETBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED);
        DL_CLRBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        dst->stVStreamInfo.pOpaque = opaque;
    } else if (DL_AFRAME == src->enDataType) {
        DL_OPAQUE_AFRAMEINFO_S *opaque = malloc(sizeof(DL_OPAQUE_AFRAMEINFO_S));
        DL_CHECK(opaque);
        DL_SETMAGIC(opaque, MAGIC_AFRAME);
        opaque->enMod = DL_USER;
        ret = wrapper_import_aframe(src, opaque);
        if (0 != ret) {
            printf("wrapper_import_aframe failed with ret(%#x).\n", ret);
            free(opaque);
            dst->stAFrameInfo.pOpaque = NULL;
            return -1;
        }
        DL_SETBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED);
        DL_CLRBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        dst->stAFrameInfo.pOpaque = opaque;
    } else if (DL_ASTREAM == src->enDataType) {
        DL_OPAQUE_ASTREAMINFO_S *opaque = malloc(sizeof(DL_OPAQUE_ASTREAMINFO_S));
        DL_CHECK(opaque);
        DL_SETMAGIC(opaque, MAGIC_ASTREAM);
        opaque->enMod = DL_USER;
        opaque->private.enEncType = src->enEncType;
        ret = wrapper_import_astream(src, opaque);
        if (0 != ret) {
            printf("wrapper_import_astream failed with ret(%#x).\n", ret);
            free(opaque);
            dst->stAStreamInfo.pOpaque = NULL;
            return -1;
        }
        DL_SETBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED);
        DL_CLRBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        dst->stAStreamInfo.pOpaque = opaque;
    } else {
        printf("datatype (%d) not support!\n", src->enDataType);
        return -1;
    }

    return 0;
}

int dlink_cvt_release(DL_PRIV_DATAINFO_U *dst)
{
    // printf("magic is (%#x)\n", **(int **)dst);
    DL_CHECK(dst);
    DL_CHECK((void *)(*(uintptr_t *)dst));
    DL_CHECK(dst->stVFrameInfo.pOpaque); // NOTE(lgY): Check pointer of |pMagic|

    int ret;
    int *pMagic = *(int **)dst;

    // NOTE(lgY): pMagic is a pointer exact the same as pOpaque
    // printf("pMagic is (%p), opaque is (%p)\n", pMagic, dst->stVFrameInfo.pOpaque);
    if (DL_HASMAGIC(pMagic, MAGIC_VFRAME)) {
        DL_OPAQUE_VFRAMEINFO_S *opaque = (DL_OPAQUE_VFRAMEINFO_S *)dst->stVFrameInfo.pOpaque;
        DL_CHECK(opaque);
        if (DL_HASBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED)) {
            if (opaque->public.enDataType != DL_VFRAME) {
                return -1; // cannot happen
            }
            ret = wrapper_freepub_vframe(&opaque->public);
            if (0 != ret) {
                printf("wrapper_freepub_vframe failed with ret(%#x).\n", ret);
                return -1;
            }
            opaque->public.ipPhyAddr = (uintptr_t)NULL;
            opaque->public.pVirAddr = NULL;
            DL_CLRBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        }
        if (DL_HASBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED)) {
            if (opaque->enMod != DL_USER) {
                return 0; // no need to free priv if not import(eg. directly get from modules)
            }
            ret = wrapper_freepriv_vframe(opaque);
            if (0 != ret) {
                printf("wrapper_freepriv_vframe failed with ret(%#x).\n", ret);
                return -1;
            }
            // DL_CLRBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED);
            // DL_CLRMAGIC(opaque);
            free(opaque);
            dst->stVFrameInfo.pOpaque = NULL;
        }
    } else if (DL_HASMAGIC(pMagic, MAGIC_VSTREAM)) {
        DL_OPAQUE_VSTREAMINFO_S *opaque = (DL_OPAQUE_VSTREAMINFO_S *)dst->stVStreamInfo.pOpaque;
        DL_CHECK(opaque);
        if (DL_HASBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED)) {
            if (opaque->public.enDataType != DL_VSTREAM) {
                return -1;
            }
            ret = wrapper_freepub_vstream(&opaque->public);
            if (0 != ret) {
                printf("wrapper_freepub_vstream failed with ret(%#x).\n", ret);
                return -1;
            }
            opaque->public.ipPhyAddr = (uintptr_t)NULL;
            opaque->public.pVirAddr = NULL;
            DL_CLRBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        }

        if (DL_HASBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED)) {
            if (opaque->enMod != DL_USER) {
                return 0;
            }
            ret = wrapper_freepriv_vstream(opaque);
            if (0 != ret) {
                printf("wrapper_freepriv_vstream failed with ret(%#x).\n", ret);
                return -1;
            }
            // DL_CLRBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED);
            // DL_CLRMAGIC(opaque);
            free(opaque);
            dst->stVStreamInfo.pOpaque = NULL;
        }
    } else if (DL_HASMAGIC(pMagic, MAGIC_AFRAME)) {
        DL_OPAQUE_AFRAMEINFO_S *opaque = (DL_OPAQUE_AFRAMEINFO_S *)dst->stAFrameInfo.pOpaque;
        DL_CHECK(opaque);
        if (DL_HASBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED)) {
            if (opaque->public.enDataType != DL_AFRAME) {
                return -1;
            }
            ret = wrapper_freepub_aframe(&opaque->public);
            if (0 != ret) {
                printf("wrapper_freepub_aframe failed with ret(%#x).\n", ret);
                return -1;
            }
            opaque->public.ipPhyAddr = (uintptr_t)NULL;
            opaque->public.pVirAddr = NULL;
            DL_CLRBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        }
        if (DL_HASBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED)) {
            if (opaque->enMod != DL_USER) {
                return 0;
            }
            ret = wrapper_freepriv_aframe(opaque);
            if (0 != ret) {
                printf("wrapper_freepriv_aframe failed with ret(%#x).\n", ret);
                return -1;
            }
            // DL_CLRBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED);
            // DL_CLRMAGIC(opaque);
            free(opaque);
            dst->stAFrameInfo.pOpaque = NULL;
        }
    } else if (DL_HASMAGIC(pMagic, MAGIC_ASTREAM)) {
        DL_OPAQUE_ASTREAMINFO_S *opaque = (DL_OPAQUE_ASTREAMINFO_S *)dst->stAStreamInfo.pOpaque;
        DL_CHECK(opaque);
        if (DL_HASBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED)) {
            if (opaque->public.enDataType != DL_ASTREAM) {
                return -1;
            }
            ret = wrapper_freepub_astream(&opaque->public);
            if (0 != ret) {
                printf("wrapper_freepub_astream failed with ret(%#x).\n", ret);
                return -1;
            }
            opaque->public.ipPhyAddr = (uintptr_t)NULL;
            opaque->public.pVirAddr = NULL;
            DL_CLRBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        }
        if (DL_HASBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED)) {
            if (opaque->enMod != DL_USER) {
                return 0;
            }
            ret = wrapper_freepriv_astream(opaque);
            if (0 != ret) {
                printf("wrapper_freepriv_astream failed with ret(%#x).\n", ret);
                return -1;
            }
            // DL_CLRBIT(opaque->enStat, QPAQUE_PRIVATE_FILLED);
            // DL_CLRMAGIC(opaque);
            free(opaque);
            dst->stAStreamInfo.pOpaque = NULL;
        }
    } else {
        return -1;
    }

    // NOTE(lgY): pMagic is same as pOpaque, so we cannot clear the free-ed pointer (use after free)
    // DL_CLRMAGIC(pMagic);

    return 0;
}

int dlink_cvt_export(/* const */ DL_PRIV_DATAINFO_U *src, DL_PUB_DATAINFO_S *dst)
{
    // printf("magic is (%#x)\n", **(int **)src);
    DL_CHECK(src);
    DL_CHECK((void *)(*(uintptr_t *)src));
    DL_CHECK(src->stVFrameInfo.pOpaque); // Check pointer of |pMagic|
    DL_CHECK(dst);

    int ret;
    int *pMagic = *(int **)src;

    if (DL_HASMAGIC(pMagic, MAGIC_VFRAME)) {
        DL_OPAQUE_VFRAMEINFO_S *opaque = (DL_OPAQUE_VFRAMEINFO_S *)src->stVFrameInfo.pOpaque;
        DL_CHECK(opaque);
        if (DL_HASBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED)) {
            memcpy(dst, &opaque->public, sizeof(DL_PUB_DATAINFO_S)); // already export
        } else {
            dst->enDataType = DL_VFRAME;
            dst->enEncType = DL_VIDEO_YUV;
            ret = wrapper_export_vframe(opaque, dst);
            if (0 != ret) {
                printf("wrapper_export_vframe failed with ret(%#x).\n", ret);
                return -1;
            }
            memcpy(&opaque->public, dst, sizeof(DL_PUB_DATAINFO_S)); // store export
            DL_SETBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        }
    } else if (DL_HASMAGIC(pMagic, MAGIC_VSTREAM)) {
        DL_OPAQUE_VSTREAMINFO_S *opaque = (DL_OPAQUE_VSTREAMINFO_S *)src->stVStreamInfo.pOpaque;
        DL_CHECK(opaque);
        if (DL_HASBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED)) {
            memcpy(dst, &opaque->public, sizeof(DL_PUB_DATAINFO_S));
        } else {
            dst->enDataType = DL_VSTREAM;
            dst->enEncType = opaque->private.enEncType;
            ret = wrapper_export_vstream(opaque, dst);
            if (0 != ret) {
                printf("wrapper_export_vstream failed with ret(%#x).\n", ret);
                return -1;
            }
            memcpy(&opaque->public, dst, sizeof(DL_PUB_DATAINFO_S));
            DL_SETBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        }
    } else if (DL_HASMAGIC(pMagic, MAGIC_AFRAME)) {
        DL_OPAQUE_AFRAMEINFO_S *opaque = (DL_OPAQUE_AFRAMEINFO_S *)src->stAFrameInfo.pOpaque;
        DL_CHECK(opaque);
        if (DL_HASBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED)) {
            memcpy(dst, &opaque->public, sizeof(DL_PUB_DATAINFO_S));
        } else {
            dst->enDataType = DL_AFRAME;
            dst->enEncType = DL_AUDIO_PCM;
            ret = wrapper_export_aframe(opaque, dst);
            if (0 != ret) {
                printf("wrapper_export_aframe failed with ret(%#x).\n", ret);
                return -1;
            }
            memcpy(&opaque->public, dst, sizeof(DL_PUB_DATAINFO_S));
            DL_SETBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        }
    } else if (DL_HASMAGIC(pMagic, MAGIC_ASTREAM)) {
        DL_OPAQUE_ASTREAMINFO_S *opaque = (DL_OPAQUE_ASTREAMINFO_S *)src->stAStreamInfo.pOpaque;
        DL_CHECK(opaque);
        if (DL_HASBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED)) {
            memcpy(dst, &opaque->public, sizeof(DL_PUB_DATAINFO_S));
        } else {
            dst->enDataType = DL_ASTREAM;
            dst->enEncType = opaque->private.enEncType;
            ret = wrapper_export_astream(opaque, dst);
            if (0 != ret) {
                printf("wrapper_export_astream failed with ret(%#x).\n", ret);
                return -1;
            }
            memcpy(&opaque->public, dst, sizeof(DL_PUB_DATAINFO_S));
            DL_SETBIT(opaque->enStat, QPAQUE_PUBLIC_FILLED);
        }
    } else {
        return -1;
    }

    return 0;
}
