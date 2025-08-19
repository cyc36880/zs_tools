#ifndef __PID_H__
#define __PID_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "../zs_tool.h"
#if (ZST_USE_PID == 1)
typedef struct
{
	float P;
	float I;
	float D;
	uint32_t maxVal; //绝对值最值
	ZST_PID_CALC_TYPE last_diff;
	ZST_PID_CALC_TYPE i_val;
}pid_t;

pid_t *pid_init(pid_t *p_pid, uint32_t maxVal, float P, float I, float D);
ZST_PID_CALC_TYPE pid_calculation(pid_t *p_pid, ZST_PID_CALC_TYPE nowVal, ZST_PID_CALC_TYPE wantVal);


#ifdef __cplusplus
}
#endif

#endif /* ZST_USE_PID */
#endif /* __PID_H__ */

