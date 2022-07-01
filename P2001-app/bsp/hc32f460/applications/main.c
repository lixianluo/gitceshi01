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
#include "main.h"




#define DBG_TAG "main"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>



/*私有变量-------------------------------------------------------*/
static TtimeInfo time_tInfo;

static volatile uint32_t TMR_ulTimer[TMR_NUM] = { 0 };


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

void Main_IotTime_Read(void)
{
	time_tInfo.IOT_systick_min = time_tInfo.systick_hrs * 60 + time_tInfo.systick_min;
}

TtimeInfo* Main_ptGetInfo(void)
{
	return &time_tInfo;
}








