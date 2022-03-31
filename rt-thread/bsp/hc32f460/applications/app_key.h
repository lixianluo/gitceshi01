#ifndef _APP_KEY_H_
#define _APP_KEY_H_
#include "stdint.h"
typedef struct{
	uint8_t		Drive_key_Flag;		//行程开关按键
	uint8_t		Suction_key_Flag;	//吸风电机按键
	uint8_t		For_key_Flag;		//前进按键
	uint8_t		Back_key_Flag;		//后退按键
	uint8_t		Brush_key_Flag;		//盘刷电机按键
	int16_t		Target_Speed;		//目标速度
	float		Moment_Speed;		//停止时刻的当前速度
}TKeyInfoDef;


extern TKeyInfoDef* Key_ptGetInfo(void);


#endif // !_APP_KEY_H_
