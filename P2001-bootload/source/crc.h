/*
 * crc.h
 *
 *  Created on: 2018Äê3ÔÂ23ÈÕ
 *      Author: hp
 */

#ifndef _CRC_H_
#define _CRC_H_

#include "defines.h"
#include "flash.h"


/*~+:ROM check: (0x00010000 - 0x00003000) = 0xD000 ;*/
/*~+:           0xD000 equals 53248 = (128 seg * 32bytes) * 13 blocks; */


#define CRC_ROM_BLOCK_NUM_IN_SEG       128		//(128 - 1)//212/*caution: maximum of uchar*/
#define CRC_ROM_BLOCK_SIZE             32		//16 /*Bytes, short calculation time in one cycle*/

#define CRC_ROM_HEAD_PROTECT			0xFFFFFFFF
#define CRC_ROM_END_PROTECT				0xFFFFFFFF
#define CRC_ROM_INIT_SEG_INDEX			0
#define CRC_ROM_INIT_BLOCK_INDEX		0


#define CRC_ROM_SEG_START_ADDR			FLASH_APP_START_ADDR	//! user app start address
#define CRC_ROM_SEG_NUM_ALL				12u//27 				/*NOT include the last block*/



#define CRC_FIRST_POWER_ON_FLAG					0x5A


#define CRC16_INIT_VALUE		0xFFFF


enum{
	CRC_CHECK_FAIL = 0,
	CRC_CHECK_PASS
};

typedef enum _TCrcTask{
	CRC_TASK_INIT = 0,
	CRC_TASK_CHECK,
	CRC_TASK_IDLE,
	CRC_TASK_ERROR
}TCrcTask;

#define BigtoLittle32(A)   ((( (ulong)(A) & 0xff000000) >> 24) | \
                                       (( (ulong)(A) & 0x00ff0000) >> 8)   | \
                                       (( (ulong)(A) & 0x0000ff00) << 8)   | \
                                       (( (ulong)(A) & 0x000000ff) << 24))


extern void CRC_vCrcInit(void);
extern uchar CRC_ucCalculateFlashCrc(void);
extern uchar CRC_ucUpdateCalculatedCrc(void);;

extern void CRC_vCheckCrcStatus(void);
extern void CRC_vSaveCRCValue(void);
extern BOOL CRC_bGetPowerOnCheckCrcFlag(void);
extern uchar CRC_ucGetCrcCheckResult(void);





















#endif /* PROJECTS_STM32F0XX_STDPERIPH_TEMPLATES_CRC_H_ */
