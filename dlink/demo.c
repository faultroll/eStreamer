#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>

#include "dlink.h"

#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(_arr[0]))

int main(void)
{
    typedef struct {
        DL_ID_S from;
        DL_ID_S to;
    } link_tbl[] = {
        {{DL_VDEC, 0, 0}, {DL_VMUL, 0, -1}},
        {{DL_VMUL, 0, 0}, {DL_VOUT, 0, 0}},
        {{DL_VMUL, 0, 1}, {DL_VENC, 0, 0}},
        {{DL_VMUL, 0, 2}, {DL_VENC, 0, 1}},
    };

    dt_init();

    for (i = 0; i < ARRAY_SIZE(link_tbl); ++i) {
        dt_bind(link_tbl[i].from, link_tbl[i].to);
    }

    getchar();

    for (i = 0; i < ARRAY_SIZE(link_tbl); ++i) {
        dt_unbind(link_tbl[i].from, link_tbl[i].to);
    }

    dt_fini();

    return 0;
}


