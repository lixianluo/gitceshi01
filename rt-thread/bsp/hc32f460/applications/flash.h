#ifndef _FLASH_H_
#define _FLASH_H_

#include "stdint.h"









/*******************************************************************************************************************
*				user data flash configuration, please modify based on project needs								   *
********************************************************************************************************************
 * Offset:
 * 0:Flag	     1:segment size     2:segment index     34~127:page index						4~127:data area
 * 0xA5      	 0x80(128bytes)	    range[00~15]	    range[00~03](page124 ~ page127)		data[0~123]
*******************************************************************************************************************/
#define FLASH_ADDR_START		0x0003C000			//!< flash start address for data storage
#define FLASH_ADDR_END			0x0003FFFF			//!< flash end address for data storage





#define FLASH_SECTOR_SIZE						  0x2000			//!< 8 Kbytes




#define FLASH_SEG_INDEX_END		127					//!< segment (one page = 16segment)
#define FLASH_PAGE_INDEX_END	7					//!< 4 pages (for data storage; page124 ~ page127)

#define FLASH_PAGE_SIZE			0x800				//!< one page size = 2048 = 0x800
#define FLASH_SEG_SIZE			0x10				//!< one segment size = 16bytes, this is the size of user data area

#define FLASH_DATA_FLAG			0xA5				//!< flash data flag


/*******************************************************************************************************************
 * Address offset of user data, please modify based on project needs
 ******************************************************************************************************************/
enum {
	/**Reserve area start, do not modify*/
	FLASH_OFFSET_FLAG = 0,					  	//!< data flag(data is stored here)				--- 1 byte
	FLASH_OFFSET_SEG_SIZE = 1,				  		//!< segment size(fixed)					--- 1 byte
	FLASH_OFFSET_SEG_INDEX = 2,				  		//!< segment index(changeable)				--- 1 byte
	FLASH_OFFSET_PAGE_INDEX = 3,			  			//!< page index(changeable)				--- 1 byte
	/**Reserve area end-----------------*/
	FLASH_OFFSET_WRITE_COUNTER = 4,			//!< total write times of flash						--- 4 bytes
	FLASH_ADD_TIME_HRS = 8,			//!< 存在Flash里面的小时时间								--- 4 bytes
	FLASH_ADD_TIME_MIN = 12,			//!< 存在Flash里面的分钟时间							--- 4 bytes 
};






//flash task state(init and  run)
//first init flash,
typedef enum _TFlashTaskState {
	FLASH_TASK_INIT = 0,
	FLASH_TASK_INIT_DONE,
	FLASH_TASK_READ,
	FLASH_TASK_READ_DONE,
	FLASH_TASK_SAVE,
	FLASH_TASK_SAVE_DONE,
}TFlashTaskState;

typedef struct _TFlashInfo {
	uint8_t ucSegIndex;		//!< current segment index in page where data saved
	uint8_t ucPageIndex;	//!< current page index 

	TFlashTaskState tTaskState;

	uint32_t ulNextDataAddr;		//!< target flash address calculated to write data
	uint32_t ulWriteCounter;
}TFlashInfo;



extern void FLASH_vReadByte(uint32_t ulTargetAddr, uint8_t* pucData, uint32_t uiLen);	//read one byte from flash
extern void FLASH_WriteByte(uint32_t ulAddr, uint8_t* pucData, uint32_t uiDataLen);	//write bytes into flash



extern void FLASH_vSaveData(void);												//save data into flash



extern TFlashInfo* Flash_ptGetInfo(void);
#endif


