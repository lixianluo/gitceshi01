#ifndef _APP_ERROR_H_
#define _APP_ERROR_H_

#include "stdint.h"



typedef enum {
	Battery_Empty,			//电池空
	Battery_Under,			//电池欠压
	Battery_Over,			//电池过压
	
	Suction_Over_Current,	//吸风电机过流
	Branch_Over_Current,	//盘刷电机过流

	Suction_Open,			//盘刷电机开路
	Branch_Open,			//吸风电机开路
	
	ERROR_NUM,
}ErrorCode;

/*对外函数-----------------------------------*/
uint32_t Error_GetInfo(void);
/*-------------------------------------------*/

#endif
