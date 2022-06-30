/*
 * task.c
 *
 *  Created on: 2018Äê9ÔÂ19ÈÕ
 *      Author: hp
 */


#include "task.h"
#include "hc32f46x_utility.h"
#include "can.h"
#include "sw_timer.h"
#include "Board2Board.h"
#include "can_iap.h"
#include "tnd_fota.h"
#include "ymodem_ota.h"

static void TASK_vUpdateWorkingTime(void);

static volatile TaskStruct TASK_tTaskTbl[] = {
/** last timestamp,				task running period,					task handler*/
	//{TASK_MS2TICK(0), 			TASK_MS2TICK(1),					TMR_vTickUpdate},
	//{TASK_MS2TICK(0), 			TASK_MS2TICK(0),						CAN_vTaskHandler},
	{TASK_MS2TICK(0), 			TASK_MS2TICK(0),						FOTA_vTaskHandler},

	//{TASK_MS2TICK(0), 			TASK_MS2TICK(0),						can_iap},
	{TASK_MS2TICK(0), 			TASK_MS2TICK(0),						ymodem_ota},

	{TASK_MS2TICK(0), 			TASK_MS2TICK(1000), 					TASK_vUpdateWorkingTime},	//!< 
};


void TASK_vScheduleTasks(void)
{
	uint8_t i = 0;
	uint8_t ucSize = sizeof(TASK_tTaskTbl) / sizeof(TaskStruct);

	for(i=0; i< ucSize; i++)
	{
		if ((SysTick_GetTick() - TASK_tTaskTbl[i].ulTimestampLast) >= TASK_tTaskTbl[i].ulTimePeriod)
		{
			TASK_tTaskTbl[i].ulTimestampLast = SysTick_GetTick();	//!< update timestamp
			TASK_tTaskTbl[i].pvTaskPointer();
		}
	}
}

static void TASK_vUpdateWorkingTime(void)
{
	/**user code put here*/
	static uint32_t systick = 0;

	systick++;
}

