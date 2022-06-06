#ifndef _APP_DISPLAY_H_
#define _APP_DISPLAY_H_

#include "stdint.h"


typedef enum {
	DISPLAY_TASK_INIT = 0,		//��ʼ��
	DISPLAY_TASK_INIT_DONE,
	DISPLAY_TASK_START_UP,		//������ʾ
	DISPLAY_TASK_START_UP_DONE,	//������ʾ����
	DISPLAY_TASK_RUN,			//������ʾ
	DISPLAY_TASK_ERROR,			//������ʾ
	DISPLAY_TASK_CLOSE,			//�ػ���ʾ
	DISPLAY_TASK_REBOOT,		//����1602
}TDisplayTaskState;

typedef struct {
	TDisplayTaskState tTaskState;
	uint8_t			  batter_info;
	uint8_t			  wifi_switch;
}DisplayTaskDef;

/*���⺯��---------------------------------------------*/
extern uint32_t APP_Display_read_hrs_time(uint32_t ulData);
extern uint32_t APP_Display_read_min_time(uint32_t ulData);
extern uint32_t APP_Display_save_hrs_time(void);
extern uint32_t APP_Display_save_min_time(void);
DisplayTaskDef *Display_ptGetInfo(void);
/*------------------------------------------------------*/
#endif // !_APP_DISPLAY_H_
