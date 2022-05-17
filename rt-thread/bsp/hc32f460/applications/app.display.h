#ifndef _APP_DISPLAY_H_
#define _APP_DISPLAY_H_

#include "stdint.h"


typedef enum {
	DISPLAY_TASK_INIT = 0,
	DISPLAY_TASK_IDLE,
	DISPLAY_TASK_RUN_BATTERY,
	DISPLAY_TASK_RUN_ERROR,
	DISPLAY_TASK_CLOSING,
	DISPLAY_TASK_CLOSE,
}DisplayTaskDef;


extern uint32_t APP_Display_read_hrs_time(uint32_t ulData);
extern uint32_t APP_Display_read_min_time(uint32_t ulData);
extern uint32_t APP_Display_save_hrs_time(void);
extern uint32_t APP_Display_save_min_time(void);

#endif // !_APP_DISPLAY_H_
