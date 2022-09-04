
#include "dlink.h"
#include "dlink_module.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// map
#include "nbds/map.h" // lock-free map
// atomic
#include "atomic_c.h" // c11 atomic wrapper
// threads
#include "thread_c.h" // c11 thread


// TODO(lgY): macros to function
#define DL_NEW(_map, _key, _data, _type) \
    do { \
        _type *__val = (_type *)malloc(sizeof(_type)); \
        if (NULL == __val) \
            return -1; \
        memcpy(__val, (_data), sizeof(_type)); \
        if (map_add((_map), (_key), (map_val_t)__val) != DOES_NOT_EXIST) \
        {  free((void *)__val); \
            return -1; \
        } \
    } while (0)
// TODO(lgY): combine these two into one macro
#define DL_DEL(_map, _key) \
    do { \
        map_val_t __val; \
        if ((__val = map_get((_map), (_key))) == DOES_NOT_EXIST) \
            return -1; \
        if (__val != (intptr_t)NULL) \
            free((void *)__val); \
        if (map_remove((_map), (_key)) == DOES_NOT_EXIST) \
            return -1; \
    } while (0)
#define DL_DEL_WAIT(_map, _key, _flag) \
    do { \
        map_val_t __val; \
        if ((__val = map_get((_map), (_key))) == DOES_NOT_EXIST) \
            return -1; \
        map_remove((_map), (_key)); \
        const int __flag = (_flag); \
        do { \
            thrd_usleep(1000 * 10); \
        } while (__flag == (_flag)); \
        if (__val != (intptr_t)NULL) \
            free((void *)__val); \
    } while (0)

typedef struct _dl_param_s_ {
    DL_ID_S     from;
    DL_ID_S     to;
    DL_PROC_F   indatafun; // for convenient
    DL_GEN_F    outdatafun;
    DL_PROC_F   freedatafun;
} DL_PARAM_S;

#define US_FAST_EXPECT      (100*1)     // 100us
#define US_NORMAL_EXPECT    (1000*1)    // 1ms
#define US_SLOW_EXPECT      (1000*10)   // 10ms

#define DL_KEY_ERROR (0)

static volatile bool __g_running = false;
static volatile int __g_access_fast_cnt   = 0;
static volatile int __g_access_normal_cnt = 0;
static volatile int __g_access_slow_cnt   = 0;
static thrd_t __g_thrd_fast_tp      = 0;
static thrd_t __g_thrd_normal_tp    = 0;
static thrd_t __g_thrd_slow_tp      = 0;
static map_t *__g_mod_cmap          = NULL;
static map_t *__g_link_fast_cmap    = NULL;
static map_t *__g_link_normal_cmap  = NULL;
static map_t *__g_link_slow_cmap    = NULL;

static int __cal_mod_key(void)
{
    static ATOMIC_VAR(int) __g_mod_key = ATOMIC_VAR_INIT(DL_INNER_END + 1);

    int key;
    do {
        key = ATOMIC_VAR_LOAD(&__g_mod_key);
        if ((key <= DL_INNER_END) && (key >= 0)) {
            ATOMIC_VAR_STOR(&__g_mod_key, DL_INNER_END + 1);
            continue;
        }
    } while (!(ATOMIC_VAR_CAS(&__g_mod_key, key, key + 1) == key));

    return key;
}

// in/out channel
int dl_create_inchn(DL_GEN_F outdata, DL_TYPE_E outtype, DL_PROC_F freedata)
{
    DL_DES_S inchn_des;
    inchn_des.mod = DL_INPUT;
    inchn_des.indatafun     = NULL;
    inchn_des.indatatype    = DL_TYPE_BUTT;
    inchn_des.outdatafun    = outdata;
    inchn_des.outdatatype   = outtype;
    inchn_des.freedatafun   = freedata;

    if (DL_VFRAME == outtype || DL_VSTREAM == outtype) {
        inchn_des.outspeed = DL_NORMAL;
    } else if (DL_AFRAME == outtype || DL_ASTREAM == outtype) {
        inchn_des.outspeed = DL_FAST;
    } else {
        printf("[%s:%d] outtype %d is error!\n", __func__, __LINE__, outtype);
        return DL_KEY_ERROR;
    }

    int key = __cal_mod_key();
    snprintf(inchn_des.name,  sizeof(inchn_des.name), "DL_INPUT_%d", key);
    DL_NEW(__g_mod_cmap, key, &inchn_des, DL_DES_S);

    return key;
}

int dl_create_outchn(DL_PROC_F indata, DL_TYPE_E intype)
{
    DL_DES_S outchn_des;
    outchn_des.mod = DL_OUTPUT;
    outchn_des.indatafun    = indata;
    outchn_des.indatatype   = intype;
    outchn_des.outdatafun   = NULL;
    outchn_des.outdatatype  = DL_TYPE_BUTT;
    outchn_des.freedatafun  = NULL;

    if (DL_VFRAME == intype || DL_VSTREAM == intype) {
        outchn_des.inspeed = DL_NORMAL;
    } else if (DL_AFRAME == intype || DL_ASTREAM == intype) {
        outchn_des.inspeed = DL_FAST;
    } else {
        printf("[%s:%d] outtype %d is error!\n", __func__, __LINE__, intype);
        return DL_KEY_ERROR;
    }

    int key = __cal_mod_key();
    snprintf(outchn_des.name, sizeof(outchn_des.name), "DL_OUPUT_%d", key);
    DL_NEW(__g_mod_cmap, key, &outchn_des, DL_DES_S);

    return key;
}

int dl_create_userchn(DL_DES_S *des)
{
    if (NULL == des) {
        printf("[%s:%d] param is null!\n", __func__, __LINE__);
        return -1;
    }

    if (DL_INPUT == des->mod) {
        if (NULL == des->outdatafun || NULL == des->freedatafun) {
            printf("[%s:%d] param is error!\n", __func__, __LINE__);
            return -1;
        }
        des->indatafun     = NULL;
        des->indatatype    = DL_TYPE_BUTT;
    } else if (DL_OUTPUT == des->mod) {
        if (NULL == des->indatafun) {
            printf("[%s:%d] param is error!\n", __func__, __LINE__);
            return -1;
        }
        des->outdatafun    = NULL;
        des->freedatafun   = NULL;
        des->outdatatype   = DL_TYPE_BUTT;
    } else if (DL_INOUT == des->mod) {
        if (NULL == des->outdatafun || NULL == des->freedatafun || NULL == des->indatafun) {
            printf("[%s:%d] param is error!\n", __func__, __LINE__);
            return -1;
        }
    }

    int key = __cal_mod_key();
    DL_NEW(__g_mod_cmap, key, des, DL_DES_S);

    return key;
}

int dl_destroy_chn(int chn)
{
    if ((chn <= DL_INNER_END) && (chn >= 0)) {
        return -1;
    }
    DL_DEL(__g_mod_cmap, chn);

    return 0;
}

static inline void thrd_usleep(int us)
{
    const struct timespec duration = {
        .tv_sec = 0,
        .tv_nsec = 1000 * us,
    };
    thrd_sleep(&duration, NULL);
}

static uint64_t __excute_task(map_iter_t *pmapbegin, DL_DATA_T pstInfo)
{
    int         ret   = 0;
    map_val_t   pval = 0;
    map_key_t   pkey  = 0;
    DL_PARAM_S  *pdl_chn = NULL;
    uint64_t    pts_begin, pts_end;

    map_iter_begin(pmapbegin, DOES_NOT_EXIST);

    pts_begin = info_timeus();

    while ((pval = map_iter_next(pmapbegin, &pkey)) != DOES_NOT_EXIST) {
        pdl_chn = (DL_PARAM_S *)pval;

        ret = pdl_chn->outdatafun(pdl_chn->from, pstInfo);
        if (ret != 0) {
            continue; // TODO(lgY): skip if failed and no success for some count
        }
        pdl_chn->indatafun(pdl_chn->to, pstInfo);
        pdl_chn->freedatafun(pdl_chn->from, pstInfo);
    }

    // cannot use 'pts_begin = pts_end;' for having 'sleep' behind
    pts_end = info_timeus();

    return (pts_end - pts_begin);
}

// TODO(lgY): use args to identify fast/normal/slow
static int __loop_fast_chn(void *arg)
{
    uint64_t    us_cost;
    map_iter_t *pmapbegin = NULL;
    // map_t      *link_cmap = (map_t *)arg;

    /* char thread_name[128];
    snprintf(thread_name, 128, "link_loop_fast");
    thrd_set_name(thread_name); // prctl(PR_SET_NAME, thread_name);
    printf("[%s:%d] start thread:%s\n", __func__, __LINE__, thread_name); */

    DL_DATA_T pstInfo = (DL_DATA_T)malloc(info_maxsize());
    if (NULL == pstInfo) {
        return -1;
    }
    pmapbegin = map_iter_alloc(__g_link_fast_cmap);
    while (__g_running) {
        us_cost = __excute_task(pmapbegin, pstInfo);
        __g_access_fast_cnt++; // no atomic need
        // calc used time
        if (us_cost < US_FAST_EXPECT/* us_expect */) {
            thrd_usleep(US_FAST_EXPECT/* (us_expect - us_cost) */); // reduce calc
        } else {
            thrd_yield();
        }
    }

    map_iter_free(pmapbegin);
    free((void *)pstInfo);

    return 0;

}

static int __loop_normal_chn(void *arg)
{
    uint64_t    us_cost;
    map_iter_t *pmapbegin = NULL;
    // map_t      *link_cmap = (map_t *)arg;

    // set thread name
    /* char thread_name[128];
    snprintf(thread_name, 128, "link_loop_normal");
    thrd_set_name(thread_name); // prctl(PR_SET_NAME, thread_name);
    printf("[%s:%d] start thread:%s\n", __func__, __LINE__, thread_name); */

    DL_DATA_T pstInfo = (DL_DATA_T)malloc(info_maxsize());
    if (NULL == pstInfo) {
        return -1;
    }
    pmapbegin = map_iter_alloc(__g_link_normal_cmap);
    while (__g_running) {
        us_cost = __excute_task(pmapbegin, pstInfo);
        __g_access_normal_cnt++;
        // calc used time
        if (us_cost < US_NORMAL_EXPECT/* us_expect */) {
            thrd_usleep(US_NORMAL_EXPECT/* (us_expect - us_cost) */);
        } else {
            thrd_yield();
        }
    }
    map_iter_free(pmapbegin);
    free((void *)pstInfo);

    return 0;
}

static int __loop_slow_chn(void *arg)
{
    uint64_t    us_cost;
    map_iter_t *pmapbegin = NULL;
    // map_t      *link_cmap = (map_t *)arg;

    // set thread name
    /* char thread_name[128];
    snprintf(thread_name, 128, "link_loop_slow");
    thrd_set_name(thread_name); // prctl(PR_SET_NAME, thread_name);
    printf("[%s:%d] start thread:%s\n", __func__, __LINE__, thread_name); */

    DL_DATA_T pstInfo = (DL_DATA_T)malloc(info_maxsize());
    if (NULL == pstInfo) {
        return -1;
    }
    pmapbegin = map_iter_alloc(__g_link_slow_cmap);
    while (__g_running) {
        us_cost = __excute_task(pmapbegin, pstInfo);
        __g_access_slow_cnt++;
        // calc used time
        if (us_cost < US_SLOW_EXPECT/* us_expect */) {
            thrd_usleep(US_SLOW_EXPECT/* (us_expect - us_cost) */);
        } else {
            thrd_yield();
        }
    }
    map_iter_free(pmapbegin);
    free((void *)pstInfo);

    return 0;
}

// pattern
typedef enum _dl_search_e {
    ONE_EQUAL = 0,
    BOTH_EQUAL,
} DL_SEARCH_E;

// find whether from and to are in task map
static int __state_judgement(DL_ID_S *from, DL_ID_S *to, map_t *pmap, DL_SEARCH_E search_pattern)
{
    int retval = 0;
    int from_exist = 0, to_exist = 0;
    if ((DL_VMUL != from->mod) && (DL_VMIX != from->mod) && \
        (DL_AMUL != from->mod) && (DL_AMIX != from->mod)) {
        from->grp = -1;
    }
    if ((DL_VMUL != to->mod) && (DL_VMIX != to->mod) && \
        (DL_AMUL != to->mod) && (DL_AMIX != to->mod)) {
        to->grp = -1;
    }

    /***********************check whether it's in map***************************/
    map_iter_t *pmapbegin = NULL;
    map_val_t pval = 0;
    map_key_t pkey = 0;
    DL_PARAM_S *pdl_params_s = NULL;

    pmapbegin = map_iter_alloc(pmap);
    map_iter_begin(pmapbegin, DOES_NOT_EXIST);
    while ((pval = map_iter_next(pmapbegin, &pkey)) != DOES_NOT_EXIST) {
        pdl_params_s = (DL_PARAM_S *)pval;
        if (NULL == pdl_params_s) {
            retval = -1;
            break;
        }

        if (0 == memcmp(&(pdl_params_s->from), from, sizeof(DL_ID_S))) {
            if (ONE_EQUAL == search_pattern) {
                retval = -1;
                break;
            }
            if (BOTH_EQUAL == search_pattern) {
                from_exist = 1;
            }
        }

        if (0 == memcmp(&(pdl_params_s->to), to, sizeof(DL_ID_S))) {
            if (ONE_EQUAL == search_pattern) {
                retval = -1;
                break;
            }
            if (BOTH_EQUAL == search_pattern) {
                to_exist = 1;
            }
        }

        if ((1 == from_exist) && (1 == to_exist)) {
            retval = (int)pkey;
            break;
        }

    }

    if (BOTH_EQUAL == search_pattern) {
        if ((0 == from_exist) || (0 == to_exist)) {
            retval = -1;
        }
    }

    map_iter_free(pmapbegin);

    return retval;
}

// NOTE(lgY): currently, from and to is not in use, it just like __cal_mod_key
// TODO(lgY): if we use <mod,chn,grp> to gen key, link logic will be simpled
static int __cal_link_key(DL_ID_S from, DL_ID_S to)
{
    static ATOMIC_VAR(int) __g_link_key = ATOMIC_VAR_INIT(0 + 1);

    // key cannot be 0 in nbds_map
    int key;
    do {
        key = ATOMIC_VAR_LOAD(&__g_link_key);
    } while (!((ATOMIC_VAR_CAS(&__g_link_key, key, key + 1) == key) && (key != DL_KEY_ERROR)));

    return key;
}

static DL_DES_S *__get_mod_from_id(DL_ID_S *pstId)
{
    map_val_t  pval = 0;

    if ((pstId->mod > DL_INNER_BEGIN) && (pstId->mod < DL_INNER_END)) {
        pval = map_get(__g_mod_cmap, pstId->mod);
        if (DOES_NOT_EXIST == pval) {
            printf("[%s] ModType from(%d) error!\n", __func__, pstId->mod);
            return NULL;
        }
    } else if ((pstId->mod < DL_INNER_BEGIN) && (pstId->mod > 0)) {
        pval = map_get(__g_mod_cmap, pstId->chn);
        if (DOES_NOT_EXIST == pval) {
            printf("[%s] UsrChn from(%d) error!\n", __func__, pstId->chn);
            return NULL;
        }
    } else {
        printf("[%s] mod id error!, mod:%d\n", __func__, pstId->mod);
        return NULL;
    }

    return (DL_DES_S *)pval;
}

static int __check_param(DL_ID_S from, DL_ID_S to)
{

#define ID_ISOK(_id) \
    (((DL_INPUT == _id.mod ) && (_id.chn > DL_INNER_END))\
     || ((DL_OUTPUT == _id.mod) && (_id.chn > DL_INNER_END)) \
     || ((DL_INOUT == _id.mod) && (_id.chn > DL_INNER_END)) \
     || ((DL_VIN == _id.mod) && (_id.chn >= 0)) \
     || ((DL_VENC == _id.mod) && (_id.chn >= 0)) \
     || ((DL_VDEC == _id.mod) && (_id.chn >= 0)) \
     || ((DL_VMUL == _id.mod) && (_id.grp >= 0)) \
     || ((DL_VMIX == _id.mod) && (_id.grp >= 0)) \
     || ((DL_VOUT == _id.mod) && (_id.chn >= 0)) \
     || ((DL_AIN == _id.mod) && (_id.chn >= 0)) \
     || ((DL_AENC == _id.mod) && (_id.chn >= 0)) \
     || ((DL_ADEC == _id.mod) && (_id.chn >= 0)) \
     || ((DL_AMUL == _id.mod) && (_id.grp >= 0)) \
     || ((DL_AMIX == _id.mod) && (_id.grp >= 0)) \
     || ((DL_AOUT == _id.mod) && (_id.chn >= 0)))

    if (
        (
            ID_ISOK(from) &&
            !(((DL_VMUL == from.mod) && (from.chn < 0)) ||
              ((DL_AMUL == from.mod) && (from.chn < 0)))
        ) &&
        (
            ID_ISOK(to) &&
            !(((DL_VMIX == to.mod) && (to.chn < 0)) ||
              ((DL_AMIX == to.mod) && (to.chn < 0)))
        )
    ) {
        return 0;
    } else {
        return -1;
    }
}

// 1. check whether it is in map; 2. if not, add node
int dl_link(DL_ID_S from, DL_ID_S to)
{
    if (!__g_running) {
        printf("please init first!\n");
        return -1;
    }

    int ret = -1;
    /***********************check if it can be linked***************************/
    DL_PARAM_S  datatrans;
    DL_DES_S    *pdl_from_s = NULL;
    DL_DES_S    *pdl_to_s   = NULL;
    int         key         = 0;
    DL_SPEED_E  eLinkSpeed  = DL_SPEED_BUTT;
    map_t      *link_cmap   = NULL;

    ret = __check_param(from, to);
    if (ret < 0) {
        printf("[%s] ID error: src(%d, %d, %d), dst(%d, %d, %d).\n",
               __func__,
               from.mod, from.grp, from.chn,
               to.mod, to.grp, to.chn);
        return -1;
    }

    pdl_from_s     = __get_mod_from_id(&from);
    if (NULL == pdl_from_s) {
        printf("[%s] from id is not create!\n", __func__);
        return -1;
    }

    pdl_to_s = __get_mod_from_id(&to);
    if (NULL == pdl_to_s) {
        printf("[%s] to id is not create!\n", __func__);
        return -1;
    }

    if (NULL == pdl_from_s->outdatafun &&
        NULL == pdl_from_s->freedatafun &&
        NULL == pdl_to_s->indatafun) {
        printf("[%s] LinkFunction is nil!\n", __func__);
        return -1;
    }

    if (pdl_from_s->outdatatype != pdl_to_s->indatatype) {
        printf("[%s] LinkType (%d, %d) not match!\n", __func__,
               pdl_from_s->outdatatype, pdl_to_s->indatatype);
        return -1;
    }

    /***********************check whether it is in map**********************/
    eLinkSpeed = pdl_to_s->inspeed > pdl_from_s->outspeed ? pdl_from_s->outspeed : pdl_to_s->inspeed; /* 使用from和to中的最小speed */
    if (DL_NORMAL == eLinkSpeed) {
        link_cmap = __g_link_normal_cmap;
    } else if (DL_FAST == eLinkSpeed) {
        link_cmap = __g_link_fast_cmap;
    } else if (DL_SLOW == eLinkSpeed) {
        link_cmap = __g_link_slow_cmap;
    }

    ret = __state_judgement(&from, &to, link_cmap, ONE_EQUAL);
    if (ret < 0) {
        printf("[%s] ID is exist.\n", __func__);
        return -1;
    }

    /***************************Link param**************************/
    datatrans.from = from;
    datatrans.outdatafun  = pdl_from_s->outdatafun;
    datatrans.freedatafun = pdl_from_s->freedatafun;

    datatrans.to = to;
    datatrans.indatafun = pdl_to_s->indatafun;

    /*************************add to oper map************************/
    key = __cal_link_key(from, to);
    DL_NEW(link_cmap, (map_key_t)key, &datatrans, DL_PARAM_S);

    printf("[%s] spd(%d): src(%s, %d, %d, %d), dst(%s, %d, %d, %d).\n",
           __func__, eLinkSpeed,
           pdl_from_s->name, from.mod, from.grp, from.chn,
           pdl_to_s->name, to.mod, to.grp, to.chn);
    return 0;
}

// 1. check whether it is in map; 2. if so, remove node
int dl_unlink(DL_ID_S from, DL_ID_S to)
{
    if (!__g_running) {
        printf("please init first!\n");
        return -1;
    }

    int ret = -1;
    /***********************check if it can be unlink***************************/
    map_key_t  pkey        = 0;
    DL_DES_S    *pdl_from_s = NULL;
    DL_DES_S    *pdl_to_s   = NULL;
    DL_SPEED_E  eLinkSpeed  = DL_SPEED_BUTT;

    ret = __check_param(from, to);
    if (ret < 0) {
        printf("[%s] ID error: src(%d, %d, %d), dst(%d, %d, %d).\n",
               __func__,
               from.mod, from.grp, from.chn,
               to.mod, to.grp, to.chn);
        return -1;
    }

    pdl_from_s     = __get_mod_from_id(&from);
    if (NULL == pdl_from_s) {
        printf("[%s] from id is not create!\n", __func__);
        return -1;
    }

    pdl_to_s = __get_mod_from_id(&to);
    if (NULL == pdl_to_s) {
        printf("[%s] to id is not create!\n", __func__);
        return -1;
    }

    eLinkSpeed = pdl_to_s->inspeed > pdl_from_s->outspeed ? pdl_from_s->outspeed : pdl_to_s->inspeed; /* 使用from和to中的最小speed */
    if (DL_NORMAL == eLinkSpeed) {
        pkey = (map_key_t)__state_judgement(&from, &to, __g_link_normal_cmap, BOTH_EQUAL);
        if (pkey > 0) {
            DL_DEL_WAIT(__g_link_normal_cmap, pkey, __g_access_normal_cnt);
        } else {
            printf("[%s] in DL_NORMAL, The ID is unexist.\n", __func__);
            return -1;
        }
    } else if (DL_FAST == eLinkSpeed) {
        pkey = (map_key_t)__state_judgement(&from, &to, __g_link_fast_cmap, BOTH_EQUAL);
        if (pkey > 0) {
            DL_DEL_WAIT(__g_link_fast_cmap, pkey, __g_access_fast_cnt);
        } else {
            printf("[%s] in DL_FAST, The ID is unexist.\n", __func__);
            return -1;
        }
    } else if (DL_SLOW == eLinkSpeed) {
        pkey = (map_key_t)__state_judgement(&from, &to, __g_link_slow_cmap, BOTH_EQUAL);
        if (pkey > 0) {
            DL_DEL_WAIT(__g_link_slow_cmap, pkey, __g_access_slow_cnt);
        } else {
            printf("[%s] IN DL_SLOW, The ID is unexist.\n", __func__);
            return -1;
        }
    }

    printf("[%s] spd(%d): src(%s, %d, %d, %d), dst(%s, %d, %d, %d).\n",
           __func__, eLinkSpeed,
           pdl_from_s->name, from.mod, from.grp, from.chn,
           pdl_to_s->name, to.mod, to.grp, to.chn);
    return 0;
}

static const DL_DES_S __g_dl_mod[] = {
    {/* VIN */
        DL_VIN, __STRING(DL_VIN), \
        { DL_NORMAL, DL_TYPE_BUTT, NULL},
        { DL_NORMAL, DL_VFRAME, wrapper_vin_outdata, wrapper_vin_freedata}
    },

    {/* VENC */
        DL_VENC, __STRING(DL_VENC), \
        { DL_NORMAL, DL_VFRAME, wrapper_venc_indata},
        { DL_NORMAL, DL_VSTREAM, wrapper_venc_outdata, wrapper_venc_freedata}
    },

    {/* VDEC */
        DL_VDEC, __STRING(DL_VDEC), \
        { DL_NORMAL, DL_VSTREAM, wrapper_vdec_indata},
        { DL_NORMAL, DL_VFRAME, wrapper_vdec_outdata, wrapper_vdec_freedata}
    },

    {/* VMUL */
        DL_VMUL, __STRING(DL_VMUL), \
        { DL_NORMAL, DL_VFRAME, wrapper_vmul_indata}, \
        { DL_NORMAL, DL_VFRAME, wrapper_vmul_outdata, wrapper_vmul_freedata}
    },

    {/* VMIX */
        DL_VMIX, __STRING(DL_VMIX), \
        { DL_NORMAL, DL_VFRAME, wrapper_vmix_indata}, \
        { DL_NORMAL, DL_VFRAME, wrapper_vmix_outdata, wrapper_vmix_freedata}
    },

    {/* VOUT */
        DL_VOUT, __STRING(DL_VOUT), \
        { DL_NORMAL, DL_VFRAME, wrapper_vout_indata}, \
        { DL_NORMAL, DL_TYPE_BUTT, NULL, NULL}
    },

    {/* AIN */
        DL_AIN, __STRING(DL_AIN), \
        { DL_FAST, DL_TYPE_BUTT, NULL}, \
        { DL_FAST, DL_AFRAME, wrapper_ain_outdata, wrapper_ain_freedata}
    },

    {/* AENC */
        DL_AENC, __STRING(DL_AENC), \
        { DL_FAST, DL_AFRAME, wrapper_aenc_indata}, \
        { DL_FAST, DL_ASTREAM, wrapper_aenc_outdata, wrapper_aenc_freedata}
    },

    {/* ADEC */
        DL_ADEC, __STRING(DL_ADEC), \
        { DL_FAST, DL_ASTREAM, wrapper_adec_indata}, \
        { DL_FAST, DL_AFRAME, wrapper_adec_outdata, wrapper_adec_freedata}
    },

    {/* AMUL */
        DL_AMUL, __STRING(DL_AMUL), \
        { DL_FAST, DL_AFRAME, wrapper_amul_indata}, \
        { DL_FAST, DL_AFRAME, wrapper_amul_outdata, wrapper_amul_freedata}
    },

    {/* AMIX */
        DL_AMIX, __STRING(DL_AMIX), \
        { DL_FAST, DL_AFRAME, wrapper_amix_indata}, \
        { DL_FAST, DL_AFRAME, wrapper_amix_outdata, wrapper_amix_freedata}
    },

    {/* AOUT */
        DL_AOUT, __STRING(DL_AOUT), \
        { DL_FAST, DL_AFRAME, wrapper_aout_indata}, \
        { DL_FAST, DL_TYPE_BUTT, NULL, NULL}
    }
};

int dl_init()
{
    if (__g_running) {
        printf("already inited!\n");
        return 0;
    }

    size_t i = 0;

    // create map
    __g_mod_cmap            = map_alloc(&MAP_IMPL_LL, NULL);
    __g_link_fast_cmap      = map_alloc(&MAP_IMPL_LL, NULL);
    __g_link_normal_cmap    = map_alloc(&MAP_IMPL_LL, NULL);
    __g_link_slow_cmap      = map_alloc(&MAP_IMPL_LL, NULL);

    for (i = 0; i < (sizeof(__g_dl_mod) / sizeof(__g_dl_mod[0])); i++) {
        DL_NEW(__g_mod_cmap, __g_dl_mod[i].mod, &__g_dl_mod[i], DL_DES_S);
    }

    // TODO(lgY): use lock-free threadpool
    __g_running = true;
    thrd_create(&__g_thrd_fast_tp, __loop_fast_chn, NULL);
    thrd_create(&__g_thrd_normal_tp, __loop_normal_chn, NULL);
    thrd_create(&__g_thrd_slow_tp, __loop_slow_chn, NULL);

    return 0;
}

int dl_fini()
{
    if (!__g_running) {
        printf("please init first!\n");
        return 0;
    }

    size_t i = 0;

    __g_running = false;
    if (__g_thrd_slow_tp != 0) {
        thrd_join(__g_thrd_slow_tp, NULL);
        __g_thrd_slow_tp = 0;
    }
    if (__g_thrd_normal_tp != 0) {
        thrd_join(__g_thrd_normal_tp, NULL);
        __g_thrd_normal_tp = 0;
    }
    if (__g_thrd_fast_tp != 0) {
        thrd_join(__g_thrd_fast_tp, NULL);
        __g_thrd_fast_tp = 0;
    }

    for (i = 0; i < (sizeof(__g_dl_mod) / sizeof(__g_dl_mod[0])); i++) {
        DL_DEL(__g_mod_cmap, __g_dl_mod[i].mod);
    }

    // destroy map
    if (__g_link_slow_cmap != NULL) {
        map_free(__g_link_slow_cmap);
        __g_link_slow_cmap = NULL;
    }
    if (__g_link_normal_cmap != NULL) {
        map_free(__g_link_normal_cmap);
        __g_link_normal_cmap = NULL;
    }
    if (__g_link_fast_cmap != NULL) {
        map_free(__g_link_fast_cmap);
        __g_link_fast_cmap = NULL;
    }
    if (__g_mod_cmap != NULL) {
        map_free(__g_mod_cmap);
        __g_mod_cmap = NULL;
    }

    return 0;
}

static void __get_status(map_t *map)
{
    map_val_t pval = 0;
    map_key_t pkey = 0;
    DL_PARAM_S *pdl_chn_s = NULL;
    map_iter_t *pmapbegin;

    pmapbegin = map_iter_alloc(map);

    map_iter_begin(pmapbegin, DOES_NOT_EXIST);
    while ((pval = map_iter_next(pmapbegin, &pkey)) != DOES_NOT_EXIST) {
        DL_DES_S *pfrom = NULL;
        DL_DES_S *pto = NULL;

        pdl_chn_s = (DL_PARAM_S *)pval;

        pfrom   = __get_mod_from_id(&pdl_chn_s->from);
        if (NULL == pfrom) {
            printf("[%s] from id is not create!\n", __func__);
            return;
        }

        pto     = __get_mod_from_id(&pdl_chn_s->to);
        if (NULL == pto) {
            printf("[%s] to id is not create!\n", __func__);
            return;
        }

        printf("src(%s, %d, %d), dst(%s, %d, %d)\n",
               pfrom->name, pdl_chn_s->from.grp, pdl_chn_s->from.chn,
               pto->name, pdl_chn_s->to.grp, pdl_chn_s->to.chn);

    }

    map_iter_free(pmapbegin);
}
int dl_status(void)
{
    if (!__g_running) {
        printf("please init first!\n");
        return -1;
    }

    printf("****************fast task status****************.\n");
    __get_status(__g_link_fast_cmap);
    printf("****************normal task status****************.\n");
    __get_status(__g_link_normal_cmap);
    printf("****************slow task status****************.\n");
    __get_status(__g_link_slow_cmap);
    printf("**********************end**********************.\n");

    return 0;
}

