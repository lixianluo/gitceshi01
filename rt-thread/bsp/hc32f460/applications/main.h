#ifndef _MAIN_H_
#define _MAIN_H_
#include "hc32_common.h"
/*基础定时间(100毫秒)（秒）（分）（时）*/
typedef struct {
	uint32_t systick_100ms;
	uint32_t systick_s;
	uint32_t systick_min;
	uint32_t systick_hrs;
	uint32_t IOT_systick_min;
}TtimeInfo;

typedef enum {
	TMR_BULE_LED,
	TMR_DISPLAY_START_VERSION,
	TMR_DISPLAY_START_LOGO,
	TMR_DISPLAY_WIFI,
	TMR_SYS_CLOSE_DELAY,
	TMR_SYS_USART_SEND,
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

/*对外函数-------------------------------------------------------*/

extern TtimeInfo* Main_ptGetInfo(void);

extern uint8_t TMR_bIsTimeExpired(TSWTimerIndex tTimerIndex);

extern void TMR_vSetTime_100msValue(TSWTimerIndex tTimerIndex, uint32_t ulValue);

/*---------------------------------------------------------------*/

#endif // !_MAIN_H_
