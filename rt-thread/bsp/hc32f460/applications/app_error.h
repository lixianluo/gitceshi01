#ifndef _APP_ERROR_H_
#define _APP_ERROR_H_

#include "stdint.h"



typedef enum {
	Temperature_Over,		//�¶ȹ���
	Batter_Over,			//��ع�ѹ
	Batter_Under,			//���Ƿѹ
	Brush_Current_Over,		//��ˢ�����������
	Suction_Current_Over,	//��������������
	Drive_Current_Over,		//���ߵ����������
	Brush_Start_Fail,		//��ˢ�������ʧ�ܹ���
	Suction_Start_Fail,		//����������ʧ�ܹ���
	Drive_Start_Fail,		//���ߵ������ʧ�ܹ���
}ErrorCode;


#endif
