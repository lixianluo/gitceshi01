#ifndef _FLASH_H
#define _FLASH_H

#include "defines.h"


/******************************************************************************************************************
 *   HC32F460's flash size is 512K bytes, minimal memory ease unit is one sector,  minimal memory write unit is 4 bytes
 *
 *   Notice:
 *   Page(8KB) erase time is 16ms ~ 20ms
 *
 *
 *	 Flash Mapping Address:
 *   Sector 0:	0x00000000 - 0x00001FFF, 8K bytes
 *   Sector 1:	0x00002000 - 0x00003FFF, 8K bytes
 *   Sector 2:	0x00004000 - 0x00005FFF, 8K bytes
 *   Sector 3:	0x0000C000 - 0x0000FFFF, 8K bytes
 *   ...
 *   ...
 *   ...
 *   Sector 30:	0x0003C000 - 0x0003DFFF, 8K bytes
 *   Sector 31:	0x0003E000 - 0x0003FFFF, 8K bytes   ---> 256K flash size
 *   ...
 *   ...
 *   ...
 *   Sector 63:	0x0007E000 - 0x0007FFFF, 8K bytes   ---> 512K flash size
 *
 
 *
 *
 ******************************************************************************************************************/
#define FLASH_SIZE								  0x3FFFF			//!< 256 Kbytes
#define FLASH_SECTOR_SIZE						  0x2000			//!< 8 Kbytes





#define FLASH_BOOT_START_ADDR					0x00000000      
#define FLASH_BOOT_SIZE							0x8000		//!< 32 Kbytes			 

#define FLASH_APP_START_ADDR					(FLASH_BOOT_START_ADDR + FLASH_BOOT_SIZE)        
#define FLASH_APP_SIZE							0x1C000		//!< 112 Kbytes														

#define FLASH_DOWNLOAD_START_ADDR				(FLASH_APP_START_ADDR + FLASH_APP_SIZE)        
#define FLASH_DOWNLOAD_SIZE						0x1C000		//!< 112 Kbtes

#define FLASH_DOWNLOAD_MAGIC_WORD_BEGIN_OFFSET	0
#define FLASH_DOWNLOAD_FILE_SIZE_OFFSET			4
#define FLASH_DOWNLOAD_CRC32_OFFSET				8
#define FLASH_DOWNLOAD_TARGET_DEVICE_OFFSET		12
#define FLASH_DOWNLOAD_MAGIC_WORD_END_OFFSET	28

#define FLASH_DOWNLOAD_RAM_VETOR_OFFSET			32

#define FLASH_APP_INFO_ADDR						(FLASH_APP_START_ADDR + FLASH_APP_SIZE - 32)


#define FLASH_DOWNLOAD_FILE_SIZE_ADDR			(FLASH_DOWNLOAD_START_ADDR + FLASH_DOWNLOAD_SIZE - 8)	//!< last last word
#define FLASH_DOWNLOAD_CRC32_ADDR				(FLASH_DOWNLOAD_START_ADDR + FLASH_DOWNLOAD_SIZE - 4)	//!< last word






extern void FLASH_vWriteBytes(uint32_t ulTargetAddr, uint8_t* pucData, uint32_t ulDataLen);
extern void FLASH_vEraseSector(uint32_t ulSectorAddr, uint32_t ulSize);












































#endif