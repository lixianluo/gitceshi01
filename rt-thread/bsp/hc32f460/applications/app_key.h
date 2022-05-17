#ifndef _APP_KEY_H_
#define _APP_KEY_H_
#include "stdint.h"

typedef struct{
	uint8_t		Suction_key_Flag;	//吸风电机按键
	uint8_t		Brush_key_Flag;		//盘刷电机按键
	uint8_t		All_key_Flag;		//盘刷+吸风电机按键
	uint8_t		Off_key_Flag;		//OFF按键
	int16_t		Hand_FLag;			//把手开关
}TKeyInfoDef;

typedef struct {
	uint8_t		Suction_key_Flag;	//吸风电机按键
	uint8_t		Brush_key_Flag;		//盘刷电机按键
	uint8_t		All_key_Flag;		//盘刷+吸风电机按键
	int16_t		Hand_FLag;			//把手开关
}TKey_CountInfoDef;



extern TKeyInfoDef* Key_ptGetInfo(void);


#endif // !_APP_KEY_H_
