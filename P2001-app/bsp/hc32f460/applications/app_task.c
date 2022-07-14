#include "app_task.h"
#include "flash.h"
#include <rtthread.h>
#include "gpio.h"
#include "main.h"
#include "app_display.h"
#include "app_key.h"
#include "app_IOT2Board.h"
#include "app_error.h"
#include "board.h"


 /*私有变量-------------------------------------------------------*/


static SysTaskDef SysTaskState;
/*---------------------------------------------------------------*/


/*私有函数-------------------------------------------------------*/
static void hw_cpu_reset(void);							//MCU重启 往CPU寄存器写入0x05FA0004 具体看M4内核手册
static void Task_vTaskHandler_Entry(void* parameter);

/*---------------------------------------------------------------*/



int Task_iTaskHandler(void)
{
	rt_thread_t tid1;
	/* 创建Task_vTaskHandler_entry程 */
	tid1 = rt_thread_create("Task_vTaskHandler_entry",
		Task_vTaskHandler_Entry,
		RT_NULL,
		1024,
		9,
		20);
	/* 创建成功则启动线程 */
	if (tid1 != RT_NULL)rt_thread_startup(tid1);

	return 0;
}
INIT_APP_EXPORT(Task_iTaskHandler);


static void Task_vTaskHandler_Entry(void* parameter)
{
	while (1)
	{
		if (TMR_bIsTimeExpired(TMR_BULE_LED))
		{
			TMR_vSetTime_100msValue(TMR_BULE_LED, 5);		//500ms周期  系统蓝灯500ms闪烁
			BLUE_TOGGLE();
		}
		if (TMR_bIsTimeExpired(TMR_RED_LED)&&(Error_GetInfo()!=0L))
		{
			TMR_vSetTime_100msValue(TMR_RED_LED, 5);		//500ms周期  系统红灯500ms闪烁
			RED_TOGGLE();
		}
		if (TMR_bIsTimeExpired(TMR_SYS_USART_SEND)&&(SysTaskState > SYS_TASK_INIT))
		{
			TMR_vSetTime_100msValue(TMR_SYS_USART_SEND, 200);	//200*100ms=20s
			IOT_vTransmitMessage(IOT_MSG_INFO_REPORT);
		}
		switch (SysTaskState)
		{
		case SYS_TASK_INIT:
		{
			BRUSH_ENABLE();				//盘刷电机使能
			SUCTION_ENABLE();			//吸风电机使能
			POW_ENABLE();				//打开电源使能口
			if ((Display_ptGetInfo()->tTaskState == DISPLAY_TASK_INIT_DONE) && \
				(Flash_ptGetInfo()->tTaskState == FLASH_TASK_READ_DONE))
			{
				SysTaskState = SYS_TASK_START;
			}
			break;
		}
		case SYS_TASK_START:
		{

			Display_ptGetInfo()->tTaskState = DISPLAY_TASK_START_UP;
			SysTaskState = SYS_TASK_IDLE;
			break;
		}
		case SYS_TASK_IDLE:
		{
			if (Key_ptGetInfo()->Off_Key_Flag == 1) //有关机标志位
			{
				Flash_ptGetInfo()->tTaskState = FLASH_TASK_SAVE;
				TMR_vSetTime_100msValue(TMR_SYS_CLOSE_DELAY, 15);	//15*100ms=1.5s
				IOT_vTransmitMessage(IOT_MSG_INFO_REPORT);			//关机发送一条指令给IOT
				SysTaskState = SYS_TASK_CLOSE;
			}
			else if ((Display_ptGetInfo()->tTaskState == DISPLAY_TASK_START_UP_DONE)||\
							(Display_ptGetInfo()->tTaskState == DISPLAY_TASK_ERROR)||\
							(Display_ptGetInfo()->tTaskState ==DISPLAY_TASK_RUN))
			{
				SysTaskState = STS_TASK_RUN;
			}
			break;
		}
		case STS_TASK_RUN:
		{
			if (Error_GetInfo())
			{
				SysTaskState = SYS_TASK_ERROR;
			}
			else
			{
				Display_ptGetInfo()->tTaskState = DISPLAY_TASK_RUN;
				SysTaskState = SYS_TASK_IDLE;
			}
			break;
		}
		case SYS_TASK_ERROR:
		{
			Display_ptGetInfo()->tTaskState = DISPLAY_TASK_ERROR;
			if (Key_ptGetInfo()->Off_Key_Flag == 1) //有关机标志位
			{
				Flash_ptGetInfo()->tTaskState = FLASH_TASK_SAVE;
				TMR_vSetTime_100msValue(TMR_SYS_CLOSE_DELAY, 15);	//15*100ms=1.5s
				IOT_vTransmitMessage(IOT_MSG_INFO_REPORT);			//关机发送一条指令给IOT
				SysTaskState = SYS_TASK_CLOSE;
			}
			if (Error_GetInfo() == 0)
			{
				SysTaskState = SYS_TASK_IDLE;
			}
			break;
		}
		case SYS_TASK_CLOSE:
		{
			Display_ptGetInfo()->tTaskState = DISPLAY_TASK_CLOSE;
			if (Flash_ptGetInfo()->tTaskState == FLASH_TASK_SAVE_DONE)
			{
				if (TMR_bIsTimeExpired(TMR_SYS_CLOSE_DELAY))
				{
					BRUSH_DISABLE();				//盘刷电机失能
					SUCTION_DISABLE();				//吸风电机失能
					POW_DISABLE();					//断开电源使能口
				}

			}
			if (Key_ptGetInfo()->Off_Key_Flag == 0) //无关机标志位
			{
				Display_ptGetInfo()->tTaskState = DISPLAY_TASK_INIT;
				Flash_ptGetInfo()->tTaskState = FLASH_TASK_INIT;
				SysTaskState = SYS_TASK_INIT;
			}
			break;
		}
		case SYS_TASK_OTA:
		{
			if ((Flash_ptGetInfo()->tTaskState == FLASH_TASK_SAVE_DONE)&&\
				(Display_ptGetInfo()->tTaskState == DISPLAY_TASK_OTA))
			{
				hw_cpu_reset();
			}
			break;
		}


		default:break;

		}
		rt_thread_mdelay(10);
	}
}
static void hw_cpu_reset(void)
{
#define SCB_AIRCR       (*(volatile unsigned long *)0xE000ED0C)  /* Reset control Address Register */
#define SCB_RESET_VALUE 0x05FA0004                               /* Reset value, write to SCB_AIRCR can reset cpu */

	SCB_AIRCR = SCB_RESET_VALUE;
}





void App_Task(SysTaskDef status)
{
	SysTaskState = status;
}







