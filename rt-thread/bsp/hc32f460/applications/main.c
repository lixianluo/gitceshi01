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

#define DBG_TAG "main"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>


/*---------------------------------------------------------------*/

/*私有变量-------------------------------------------------------*/
static TtimeInfo time_tInfo;
/*---------------------------------------------------------------*/


/*私有函数-------------------------------------------------------*/
static  rt_timer_t one_msecond_timer;
static void one_msecond_timeout(void* parameter);


/*---------------------------------------------------------------*/






int32_t main(void)
{

	LOG_I("main fun is running");


	/*1s定时器*/
	one_msecond_timer = rt_timer_create("one_second", one_msecond_timeout , 
									RT_NULL, 100, 
									RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER); 
	if (one_msecond_timer != RT_NULL)rt_timer_start(one_msecond_timer);
	



	while(1)
	{
		rt_thread_mdelay(500);
		GREEN_TOGGLE();
		
	}
}

static void one_msecond_timeout(void* parameter)
{
	time_tInfo.systick_s++;
	if ((time_tInfo.systick_s % 60) == 0)
	{
		time_tInfo.systick_s = 0;
		time_tInfo.systick_min++;
		if ((time_tInfo.systick_min % 60) == 0)
		{
			time_tInfo.systick_min = 0;
			time_tInfo.systick_hrs++;
		}
	}
	
}

TtimeInfo* Main_ptGetInfo(void)
{
	return &time_tInfo;
}
