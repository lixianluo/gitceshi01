#ifndef _APP_DISPLAY_H_
#define _APP_DISPLAY_H_

#include "myiic.h"


typedef enum {
	DISPLAY_TASK_INIT = 0,
	DISPLAY_TASK_IDLE,
	DISPLAY_TASK_RUN_SPEED,
	DISPLAY_TASK_RUN_BATTERY,
	DISPLAY_TASK_RUN_ERROR,
}TDisplayTaskDef;



typedef struct {
	TDisplayTaskDef tDisplayTaskState;
	uint8_t			Speed_Level;

}TDisplayInfoDef;


extern TDisplayInfoDef* Display_tGetInfo(void);

#endif // !_APP_DISPLAY_H_
