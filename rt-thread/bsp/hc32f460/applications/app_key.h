#ifndef _APP_KEY_H_
#define _APP_KEY_H_
#include "stdint.h"
typedef struct{
	uint8_t		Drive_key_Flag;		//�г̿��ذ���
	uint8_t		Suction_key_Flag;	//����������
	uint8_t		For_key_Flag;		//ǰ������
	uint8_t		Back_key_Flag;		//���˰���
	uint8_t		Brush_key_Flag;		//��ˢ�������
	int16_t		Target_Speed;		//Ŀ���ٶ�
	float		Moment_Speed;		//ֹͣʱ�̵ĵ�ǰ�ٶ�
}TKeyInfoDef;


extern TKeyInfoDef* Key_ptGetInfo(void);


#endif // !_APP_KEY_H_
