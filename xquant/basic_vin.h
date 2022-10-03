
#include "xquant.h"

#ifndef _BASIC_VIN_H_
#define _BASIC_VIN_H_

#if defined(__cplusplus)
extern "C" {
#endif

// XQ_OPER_USRPIC_SET
typedef struct _xq_usrpic_set_param_s_
{
    int frame_rate;
    DL_PUB_DATAINFO_S stUsrPic;
} xq_usrpic_set_param_s;

// XQ_OPER_USRPIC_SWITCH
typedef struct _xq_usrpic_switch_param_s_
{
    bool enable;
} xq_usrpic_switch_param_s;

#if defined(__cplusplus)
}
#endif

#endif /* _BASIC_VIN_H_ */
