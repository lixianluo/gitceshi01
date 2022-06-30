/*
 * task.h
 *
 *  Created on: 2018Äê9ÔÂ19ÈÕ
 *      Author: hp
 */

#ifndef _TASK_H_
#define _TASK_H_
#include <stdint.h>

//#include "defines.h"

#define TASK_BASE_TICK		1	//!< 1ms, must modify this based on system ticks
#define TASK_MS2TICK(x)		((x) / TASK_BASE_TICK)
	





typedef struct _TaskStruct{
	uint32_t ulTimestampLast;			//!< last time stample
	uint32_t ulTimePeriod;				//!< task loop period, 0 means task run at every loop
	void (*pvTaskPointer)(void);    //!< task handler pointer
}TaskStruct;



extern void TASK_vScheduleTasks(void);





























#endif /* _TASK_H_ */
