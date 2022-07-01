#ifndef _APP_KEY_H_
#define _APP_KEY_H_
#include "stdint.h"

typedef struct{
	uint8_t		Suction_Key_Flag;	//����������
	uint8_t		Brush_Key_Flag;		//��ˢ�������
	uint8_t		All_Key_Flag;		//��ˢ+����������
	uint8_t		Off_Key_Flag;		//OFF����
	uint8_t		Hand_Key_FLag;		//���ֿ���
	uint8_t		Work_Status;		//����״̬
}TKeyInfoDef;

typedef struct {
	uint8_t		B3_Count;			//B3��������
	uint8_t		A1_Count;			//A1��������
	uint8_t		A2_Count;			//A2��������
	uint8_t		Hand_Count;			//���ֿ�����������
	uint8_t     B3_Flag;			//B3��־λ
	uint8_t     A1_Flag;			//A1��־λ
	uint8_t     A2_Flag;			//A2��־λ
	uint8_t     Hand_Flag;			//���ֿ��ر�־λ
	uint8_t		Off_Flag;			//�ػ���־λ
}TKey_CountInfoDef;

enum{
	OFF_MODE = 0,			//�ػ�ģʽ    
	ALL_MODE,				//ȫ����ģʽ
	BRUSH_MODE,				//ֻˢģʽ
	SUCTION_MODE,			//ֻ��ģʽ
};

extern TKeyInfoDef* Key_ptGetInfo(void);


#endif // !_APP_KEY_H_
