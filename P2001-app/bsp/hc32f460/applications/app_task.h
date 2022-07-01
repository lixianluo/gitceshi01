#ifndef _APP_TASK_H_
#define _APP_TASK_H_


typedef enum {
	SYS_TASK_INIT,
	SYS_TASK_START,
	SYS_TASK_IDLE,
	STS_TASK_RUN,
	SYS_TASK_ERROR,
	SYS_TASK_CLOSE,
	SYS_TASK_OTA,
}SysTaskDef;

/*¶ÔÍâº¯Êý---------------------------------------------*/
extern void App_Task(SysTaskDef status);
/*-----------------------------------------------------*/

#endif // !_APP_TASK_H_
