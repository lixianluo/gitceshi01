#ifndef _APP_KEY_H_
#define _APP_KEY_H_
#include "stdint.h"

typedef struct{
	uint8_t		Suction_key_Flag;	//����������
	uint8_t		Brush_key_Flag;		//��ˢ�������
	uint8_t		All_key_Flag;		//��ˢ+����������
	uint8_t		Off_key_Flag;		//OFF����
	int16_t		Hand_FLag;			//���ֿ���
}TKeyInfoDef;

typedef struct {
	uint8_t		Suction_key_Flag;	//����������
	uint8_t		Brush_key_Flag;		//��ˢ�������
	uint8_t		All_key_Flag;		//��ˢ+����������
	int16_t		Hand_FLag;			//���ֿ���
}TKey_CountInfoDef;



extern TKeyInfoDef* Key_ptGetInfo(void);


#endif // !_APP_KEY_H_