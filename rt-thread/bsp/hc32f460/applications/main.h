#ifndef _MAIN_H_
#define _MAIN_H_
#include "hc32_common.h"
/*基础定时间（秒）（分）（时）*/
typedef struct {
	uint32_t systick_s;
	uint32_t systick_min;
	uint32_t systick_hrs;
}TtimeInfo;


/*对外函数-------------------------------------------------------*/

extern TtimeInfo* Main_ptGetInfo(void);

/*---------------------------------------------------------------*/

#endif // !_MAIN_H_
