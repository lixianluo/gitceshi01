#ifndef _MAIN_H_
#define _MAIN_H_
#include "hc32_common.h"
/*������ʱ�䣨�룩���֣���ʱ��*/
typedef struct {
	uint32_t systick_s;
	uint32_t systick_min;
	uint32_t systick_hrs;
}TtimeInfo;


/*���⺯��-------------------------------------------------------*/

extern TtimeInfo* Main_ptGetInfo(void);

/*---------------------------------------------------------------*/

#endif // !_MAIN_H_
