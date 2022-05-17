#ifndef _APP_ERROR_H_
#define _APP_ERROR_H_

#include "stdint.h"



typedef enum {
	Temperature_Over,		//温度过高
	Batter_Over,			//电池过压
	Batter_Under,			//电池欠压

}ErrorCode;


#endif
