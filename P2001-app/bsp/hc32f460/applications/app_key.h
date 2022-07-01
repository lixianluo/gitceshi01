#ifndef _APP_KEY_H_
#define _APP_KEY_H_
#include "stdint.h"

typedef struct{
	uint8_t		Suction_Key_Flag;	//吸风电机按键
	uint8_t		Brush_Key_Flag;		//盘刷电机按键
	uint8_t		All_Key_Flag;		//盘刷+吸风电机按键
	uint8_t		Off_Key_Flag;		//OFF按键
	uint8_t		Hand_Key_FLag;		//把手开关
	uint8_t		Work_Status;		//工作状态
}TKeyInfoDef;

typedef struct {
	uint8_t		B3_Count;			//B3消抖计数
	uint8_t		A1_Count;			//A1消抖计数
	uint8_t		A2_Count;			//A2消抖计数
	uint8_t		Hand_Count;			//把手开关消抖计数
	uint8_t     B3_Flag;			//B3标志位
	uint8_t     A1_Flag;			//A1标志位
	uint8_t     A2_Flag;			//A2标志位
	uint8_t     Hand_Flag;			//把手开关标志位
	uint8_t		Off_Flag;			//关机标志位
}TKey_CountInfoDef;

enum{
	OFF_MODE = 0,			//关机模式    
	ALL_MODE,				//全功率模式
	BRUSH_MODE,				//只刷模式
	SUCTION_MODE,			//只吸模式
};

extern TKeyInfoDef* Key_ptGetInfo(void);


#endif // !_APP_KEY_H_
