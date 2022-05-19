#ifndef _MAIN_H_
#define _MAIN_H_
#include "hc32_common.h"
/*������ʱ��(100����)���룩���֣���ʱ��*/
typedef struct {
	uint32_t systick_100ms;
	uint32_t systick_s;
	uint32_t systick_min;
	uint32_t systick_hrs;
}TtimeInfo;

typedef enum {
	TMR_BULE_LED,
	TMR_DISPLAY_START_VERSION,
	TMR_DISPLAY_START_LOGO,
	TMR_SYS_CLOSE_DELAY,
	TMR_NUM,
}TSWTimerIndex;
typedef enum {
	SYS_TASK_INIT,
	SYS_TASK_START,
	SYS_TASK_IDLE,
	STS_TASK_RUN,
	SYS_TASK_ERROR,
	SYS_TASK_CLOSE
}SysTaskDef;

/*���⺯��-------------------------------------------------------*/

extern TtimeInfo* Main_ptGetInfo(void);
extern uint8_t TMR_bIsTimeExpired(TSWTimerIndex tTimerIndex);

extern void TMR_vSetTime_100msValue(TSWTimerIndex tTimerIndex, uint32_t ulValue);

/*---------------------------------------------------------------*/

#endif // !_MAIN_H_
