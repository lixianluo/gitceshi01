#ifndef _APP_ERROR_H_
#define _APP_ERROR_H_

#include "stdint.h"



typedef enum {
	Temperature_Over,		//温度过高
	Batter_Over,			//电池过压
	Batter_Under,			//电池欠压
	Brush_Current_Over,		//盘刷电机电流故障
	Suction_Current_Over,	//吸风电机电流故障
	Drive_Current_Over,		//行走电机电流故障
	Brush_Start_Fail,		//盘刷电机启动失败故障
	Suction_Start_Fail,		//吸风电机启动失败故障
	Drive_Start_Fail,		//行走电机启动失败故障
}ErrorCode;


#endif
