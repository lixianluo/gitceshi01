#ifndef _APP_ERROR_H_
#define _APP_ERROR_H_

#include "stdint.h"



typedef enum {
	Battery_Empty,			//��ؿ�
	Battery_Under,			//���Ƿѹ
	Battery_Over,			//��ع�ѹ
	
	Suction_Over_Current,	//����������
	Branch_Over_Current,	//��ˢ�������

	Suction_Open,			//��ˢ�����·
	Branch_Open,			//��������·
	
	ERROR_NUM,
}ErrorCode;

/*���⺯��-----------------------------------*/
uint32_t Error_GetInfo(void);
/*-------------------------------------------*/

#endif
