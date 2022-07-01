#ifndef _TND_FOTA_H
#define _TND_FOTA_H

#include "defines.h"

#define FOTA_DEVICE_NAME	"P2001_DRV"

#define TND_CTL_DEVICE_NAME	"TND_CTL"
#define TND_DRV_DEVICE_NAME	"TND_DRV"
#define TND_INT_DEVICE_NAME	"TND_INT"




typedef enum {
	FOTA_TASK_INIT = 0,
	FOTA_TASK_CHECK_UPDATE,
	FOTA_TASK_DOWNLOADING,
	FOTA_TASK_CHECK_FW,
	FOTA_TASK_FW_UPDATE,	//!< sensor hub fw update
	FOTA_TASK_FW_FORWARD,	//!< tnd_ctl\tnd_drv\tnd_int fw update
	FOTA_TASK_START_APP
}TfotaTaskStateDef;









#define CRC_INIT			        0xFFFFFFFF
#define CRC_REFOUT				    0xFFFFFFFF























extern void FOTA_vTaskHandler(void);
extern uint32_t FOTA_ulGetAppFileSize(void);






#endif