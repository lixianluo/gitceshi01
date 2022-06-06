/*
 * Copyright (C) 2021, lizhengyang
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2021-09-02      lizhengyang     first version
 */
#include "board.h"
#include <rtthread.h>
#include <rtdevice.h>
#include "hc32_common.h"
#include "gpio.h"
#include "timea.h"
#include "main.h"
#include "flash.h"
#include "app_display.h"
#include "app_key.h"
#include "app_IOT2Board.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>



/*私有变量-------------------------------------------------------*/
static TtimeInfo time_tInfo;

static volatile uint32_t TMR_ulTimer[TMR_NUM] = { 0 };

static SysTaskDef SysTaskState;
/*---------------------------------------------------------------*/


/*私有函数-------------------------------------------------------*/
static  rt_timer_t one_msecond_timer;
static  rt_timer_t hundreds_millisecond_timer;
static void one_msecond_timeout(void* parameter);
static void hundreds_millisecond_timeout(void* parameter);
static void TMR_vTickUpdate(void);


/*---------------------------------------------------------------*/






int32_t main(void)
{
	LOG_I("main fun is running");


	/*1s定时器*/
	one_msecond_timer = rt_timer_create("one_second", one_msecond_timeout , 
									RT_NULL, 100, 
									RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER); 
	if (one_msecond_timer != RT_NULL)rt_timer_start(one_msecond_timer);
	
	/*100ms定时器*/
	hundreds_millisecond_timer = rt_timer_create("hundreds_millisecond", hundreds_millisecond_timeout,
		RT_NULL, 10,
		RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
	if (hundreds_millisecond_timer != RT_NULL)rt_timer_start(hundreds_millisecond_timer);

	while(1)
	{
		if (TMR_bIsTimeExpired(TMR_BULE_LED))
		{
			TMR_vSetTime_100msValue(TMR_BULE_LED, 5);		//500ms周期
			BLUE_TOGGLE();
		}
		if (TMR_bIsTimeExpired(TMR_SYS_USART_SEND))
		{
			TMR_vSetTime_100msValue(TMR_SYS_USART_SEND, 200);	//200*100ms=20s
			IOT_vTransmitMessage(IOT_MSG_INFO_REPORT);
		}
		switch (SysTaskState)
		{
			case SYS_TASK_INIT:
			{
				PORT_SetBits(PWM_EN_PORT, PWM_EN_PIN);	    //打开电机使能口
				PORT_SetBits(POWEN_PORT, POWEN_PIN);		//打开电源使能口
				if ((Display_ptGetInfo()->tTaskState == DISPLAY_TASK_INIT_DONE) && \
					(Flash_ptGetInfo()->tTaskState== FLASH_TASK_READ_DONE))
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
				/*加入是否有错误标志-------------
				if有 则跳转到SYS_TASK_ERROR
				--------------------------------*/
				if (Key_ptGetInfo()->Off_key_Flag == 1) //有关机标志位
				{
					Flash_ptGetInfo()->tTaskState = FLASH_TASK_SAVE;
					TMR_vSetTime_100msValue(TMR_SYS_CLOSE_DELAY, 15);	//15*100ms=1.5s
					SysTaskState = SYS_TASK_CLOSE;
				}
				else if(Display_ptGetInfo()->tTaskState == DISPLAY_TASK_START_UP_DONE)
				{
					SysTaskState = STS_TASK_RUN;
				}
				break;
			}
			case STS_TASK_RUN:
			{
				Display_ptGetInfo()->tTaskState = DISPLAY_TASK_RUN;
				SysTaskState = SYS_TASK_IDLE;
				break;
			}
			case SYS_TASK_ERROR:
			{

				break;
			}
			case SYS_TASK_CLOSE:
			{
				Display_ptGetInfo()->tTaskState = DISPLAY_TASK_CLOSE;
				if (Flash_ptGetInfo()->tTaskState == FLASH_TASK_SAVE_DONE)
				{
					if (TMR_bIsTimeExpired(TMR_SYS_CLOSE_DELAY))
					{
						PORT_ResetBits(PWM_EN_PORT, PWM_EN_PIN);	    //断开电机使能口
						PORT_ResetBits(POWEN_PORT, POWEN_PIN);		    //断开电源使能口
					}
					
				}
				if (Key_ptGetInfo()->Off_key_Flag == 0) //无关机标志位
				{
					Display_ptGetInfo()->tTaskState = DISPLAY_TASK_INIT;
					Flash_ptGetInfo()->tTaskState = FLASH_TASK_INIT;
					SysTaskState = SYS_TASK_INIT;
				}
				break;
			}
			default:break;
		
		}
		rt_thread_mdelay(10);
	}
}

static void one_msecond_timeout(void* parameter)
{
	time_tInfo.systick_s++;
	time_tInfo.IOT_systick_min = time_tInfo.systick_hrs * 60 + time_tInfo.systick_min;
	if ((time_tInfo.systick_s % 60) == 0)
	{
		time_tInfo.systick_s = 0;
		time_tInfo.systick_min++;
		if ((time_tInfo.systick_min % 60) == 0)
		{
			time_tInfo.systick_min = 0;
			time_tInfo.systick_hrs++;
			if (time_tInfo.systick_hrs == 10000000)	//LCD显示位数最大值
			{
				time_tInfo.systick_hrs = 0;
				time_tInfo.IOT_systick_min = 0;
				time_tInfo.systick_min = 0;
			}
		}
	}
	
}

/*100ms进行一次中断回调函数*/
static void hundreds_millisecond_timeout(void* parameter)
{
	TMR_vTickUpdate();
	time_tInfo.systick_100ms++;
	if (time_tInfo.systick_100ms >= 10)time_tInfo.systick_100ms = 0;
}


static void TMR_vTickUpdate(void)
{
	uint8_t ucIndex;

	for (ucIndex = 0; ucIndex < TMR_NUM; ucIndex++)
	{
		if (TMR_ulTimer[ucIndex])	//counter > 0
		{
			TMR_ulTimer[ucIndex]--;
		}
	}
}

uint8_t TMR_bIsTimeExpired(TSWTimerIndex tTimerIndex)
{
	return (0 == TMR_ulTimer[tTimerIndex]);
}


void TMR_vSetTime_100msValue(TSWTimerIndex tTimerIndex, uint32_t ulValue)
{
	TMR_ulTimer[tTimerIndex] = ulValue;
}


TtimeInfo* Main_ptGetInfo(void)
{
	return &time_tInfo;
}








