#ifndef _APP_ERROR_H_
#define _APP_ERROR_H_

#include "stdint.h"



typedef enum {
	Temperature_Over,		//�¶ȹ���
	Batter_Over,			//��ع�ѹ
	Batter_Under,			//���Ƿѹ

}ErrorCode;

/*���⺯��-----------------------------------*/
uint32_t Error_ptGetInfo(void);
/*-------------------------------------------*/

#endif
