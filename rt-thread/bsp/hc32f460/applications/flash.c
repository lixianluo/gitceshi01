#include "flash.h"
#include "hc32f460_efm.h"
#include "main.h"
#include "app_display.h"
#include <rtthread.h>
#include "app_key.h"


static TFlashInfo FLASH_tFlashInfo;


static void FLASH_vTaskHandler_Entry(void* parameter);


//read one byte from flash
void FLASH_vReadByte(uint32_t ulTargetAddr, uint8_t* pucData, uint32_t uiLen)
{
	/* used memory copy function */
	(void)memcpy(pucData, (uint8_t*)ulTargetAddr, uiLen);
}

//write bytes into flash
void FLASH_WriteByte(uint32_t ulTargetAddr, uint8_t* pucData,uint32_t ulDatalen)  //
{
	uint32_t ulWordNum = 0;
	uint32_t ulWord = 0;

	ulWordNum = ulDatalen / 4u;
	/* Unlock EFM. */
	EFM_Unlock();
	/* Enable flash. */
	EFM_FlashCmd(Enable);
	/* Wait flash ready. */
	while (Set != EFM_GetFlagStatus(EFM_FLAG_RDY))
	{
		;
	}
	while (ulWordNum--)
	{
		ulWord = *(pucData++);
		ulWord |= *(pucData++) << 8;
		ulWord |= *(pucData++) << 16;
		ulWord |= *(pucData++) << 24;
		if (EFM_SingleProgram(ulTargetAddr, ulWord) != Ok)
		{
			while (1);
		}
		ulTargetAddr += 4u;
	}
	
	EFM_Lock();
}

void FLASH_vEraseSector(uint32_t ulSectorAddr, uint32_t ulSize)
{
	uint32_t ulSectorNum = 0;

	if ((ulSize % FLASH_SECTOR_SIZE) == 0)
	{
		ulSectorNum = ulSize / FLASH_SECTOR_SIZE;
	}
	else
	{
		ulSectorNum = ulSize / FLASH_SECTOR_SIZE + 1;
	}

	/* Unlock EFM. */
	EFM_Unlock();

	/* Enable flash. */
	EFM_FlashCmd(Enable);
	/* Wait flash ready. */
	while (Set != EFM_GetFlagStatus(EFM_FLAG_RDY))
	{
		;
	}

	while (ulSectorNum--)
	{
		/* Erase sector  */
		EFM_SectorErase(ulSectorAddr);

		ulSectorAddr += FLASH_SECTOR_SIZE;
	}

	/* Lock EFM. */
	EFM_Lock();
}
/*********************************************************************************************************************
*Search flag from flash
* description:
*check flash write flag from last segment of last page, if not found then switch to the segment ahead until the flag found or
*exceeds the flash area :
*
*↑	start page
* ↑			start segment
* ↑			...
*	↑			last segment
* ↑	...
*  ↑	...
*  ↑	last page
* ↑			start segment
* ↑			...
*	↑			last segment
*
*return:
*0 -- - not found, check the next segment(middle search state)
* 1 -- - data is found
* 2 -- - exceed page size, no data found
********************************************************************************************************************/
static uint8_t FLASH_ucSearchFlagFromFlash(void)
{
	uint8_t ucData[2] = { 0, 0 };

	/** calculate address based on seg index and page index */
	FLASH_tFlashInfo.ulNextDataAddr = FLASH_ADDR_START + FLASH_PAGE_SIZE * FLASH_tFlashInfo.ucPageIndex + FLASH_SEG_SIZE * FLASH_tFlashInfo.ucSegIndex;

	/** read flag and segment size*/
	FLASH_vReadByte(FLASH_tFlashInfo.ulNextDataAddr, ucData, 2u);

	//check flag value
	if ((FLASH_DATA_FLAG == ucData[0]) && (FLASH_SEG_SIZE == ucData[1]))
	{
		return 1;				//!< data is found,flag is found
	}
	else
	{
		if (FLASH_tFlashInfo.ucSegIndex)
		{
			FLASH_tFlashInfo.ucSegIndex--;
		}
		else
		{
			FLASH_tFlashInfo.ucSegIndex = FLASH_SEG_INDEX_END;	//!< reset value

			/**Already first segment, check next page*/
			if (FLASH_tFlashInfo.ucPageIndex)
			{
				FLASH_tFlashInfo.ucPageIndex--;
			}
			else
			{
				/**Already first page, no data found at all*/
				FLASH_tFlashInfo.ucPageIndex = FLASH_PAGE_INDEX_END;

				return 2;	//!< exceeds page size, no data found
			}
		}

		return 0;	//!< not found, check next segment
	}
}

static void FLASH_vReadData(void)
{
	uint8_t ucReadData[FLASH_SEG_SIZE];	//one segment

	/**read one segment data*/
	//where found the data flag, then read the data from the address,read all the storage data
	FLASH_vReadByte(FLASH_tFlashInfo.ulNextDataAddr, ucReadData, FLASH_SEG_SIZE);

	/**check seg index, page index*/
	if ((FLASH_tFlashInfo.ucSegIndex == ucReadData[FLASH_OFFSET_SEG_INDEX]) && (FLASH_tFlashInfo.ucPageIndex == ucReadData[FLASH_OFFSET_PAGE_INDEX]))
	{
		/*************************************************************************************************
		 * *********** Application code start from here !!!
		 * **********************************************************************************************/
		 /**read flash write counter*/
		(void)memcpy(&FLASH_tFlashInfo.ulWriteCounter, &ucReadData[FLASH_OFFSET_WRITE_COUNTER], 4u);

		/**将Flah的值读出并处理*/
		uint32_t temp_value = 0;
		(void)memcpy(&temp_value, &ucReadData[FLASH_ADD_TIME_HRS], 4u);
		APP_Display_read_hrs_time(temp_value);

		(void)memcpy(&temp_value, &ucReadData[FLASH_ADD_TIME_MIN], 4u);
		APP_Display_read_min_time(temp_value);

		/*************************************************************************************************
		 * *********** Application code end here !!!
		 * **********************************************************************************************/
	}
}

//flash init
//1 --- flash init is finished
//0 --- flash init is not finished
static uint8_t FLASH_ucInitData(void)
{
	uint8_t ucResult = 0;

	ucResult = FLASH_ucSearchFlagFromFlash();	//read flag from flash

	//handle result
	if (1 == ucResult)
	{
		/**data is found*/
		FLASH_vReadData();	//read data from flash and set data

		return 1;
	}
	else if (2 == ucResult)
	{
		/**no data found,  data will saved at start address*/
		//start address;segment index = 0;page index = 0
		FLASH_tFlashInfo.ulNextDataAddr = FLASH_ADDR_START;	 	 	//!< start address
		FLASH_tFlashInfo.ucSegIndex = FLASH_SEG_INDEX_END;			//!< set last segment index to trigger increment in FLASH_vSaveData()
		FLASH_tFlashInfo.ucPageIndex = FLASH_PAGE_INDEX_END;		//!< set last page index to trigger increment in FLASH_vSaveData()

		return 1;
	}
	else	//ucRresult = 0;check is not finished,keep checking
	{
		/**check is not finished, wait*/
		return 0;
	}
}
//save data into flash
void FLASH_vSaveData(void)
{
	uint8_t ucTempData[FLASH_SEG_SIZE];	//data buffer

	/** clear data buffer */
	(void)memset(&ucTempData[4], 0xFF, FLASH_SEG_SIZE - 4);

	/**one page is full, write to next page*/
	if (FLASH_SEG_INDEX_END <= FLASH_tFlashInfo.ucSegIndex)
	{
		FLASH_tFlashInfo.ucSegIndex = 0;	//!< back to first segment

		/**currently already last page, write to first page*/
		if (FLASH_PAGE_INDEX_END <= FLASH_tFlashInfo.ucPageIndex)
		{
			FLASH_tFlashInfo.ucPageIndex = 0;	//switch to first page
		}
		else
		{
			FLASH_tFlashInfo.ucPageIndex++;	//page index ++
		}
	}
	else
	{
		FLASH_tFlashInfo.ucSegIndex++;		//!< one page is not full, write to next segment(write one segment every time)
	}

	/* update header data(header information before the data) */
	ucTempData[FLASH_OFFSET_FLAG] = FLASH_DATA_FLAG;		//!< flag(fixed) --- 1 byte
	ucTempData[FLASH_OFFSET_SEG_SIZE] = FLASH_SEG_SIZE;		//!< segment size(useless) --- 1 byte
	ucTempData[FLASH_OFFSET_SEG_INDEX] = FLASH_tFlashInfo.ucSegIndex;		//!< segment index(useless) --- 1 byte
	ucTempData[FLASH_OFFSET_PAGE_INDEX] = FLASH_tFlashInfo.ucPageIndex;	//!< page index(useless) --- 1 byte



	/*************************************************************************************************
	 * *********** Application code start from here !!!
	 * **********************************************************************************************/

	 /**flash write counter*/
	FLASH_tFlashInfo.ulWriteCounter++;		//!< record flash write counter for system tracking use
	(void)memcpy(&ucTempData[FLASH_OFFSET_WRITE_COUNTER], (uint8_t*)&FLASH_tFlashInfo.ulWriteCounter, 4u);	//!< 4 bytes

	/**vacuum lifetime on time in seconds*/
	uint32_t temp_value = APP_Display_save_hrs_time();
	(void)memcpy(&ucTempData[FLASH_ADD_TIME_HRS], (uint8_t*)&temp_value, 4u);	//!< 4 bytes

	/**valve lifetime on time in seconds*/
	temp_value = APP_Display_save_min_time();
	(void)memcpy(&ucTempData[FLASH_ADD_TIME_MIN], (uint8_t*)&temp_value, 4u);	//!< 4 bytes

	/*************************************************************************************************
	 * *********** Application code end here !!!
	 * **********************************************************************************************/


	 /**************************************************************************************************
	  *  Reserve area, do not modify
	  *************************************************************************************************/
	  /**calculate new address(get the start address)*/
	FLASH_tFlashInfo.ulNextDataAddr = FLASH_ADDR_START + FLASH_PAGE_SIZE * FLASH_tFlashInfo.ucPageIndex + FLASH_SEG_SIZE * FLASH_tFlashInfo.ucSegIndex;

	/**Erase all pages if start from start address*/
	if ((0 == FLASH_tFlashInfo.ucSegIndex) && (0 == FLASH_tFlashInfo.ucPageIndex))
	{
		FLASH_vEraseSector(FLASH_ADDR_START, FLASH_SECTOR_SIZE* 2);
	}

	/**write to flash(write one segment data into flash)*/
	FLASH_WriteByte(FLASH_tFlashInfo.ulNextDataAddr, ucTempData, FLASH_SEG_SIZE);
}



static void FLASH_vTaskHandler_Entry(void* parameter)
{
	
	while (1)
	{
		
		switch (FLASH_tFlashInfo.tTaskState)
		{
			case FLASH_TASK_INIT:		//flash init
			{
				FLASH_tFlashInfo.ucSegIndex = FLASH_SEG_INDEX_END;
				FLASH_tFlashInfo.ucPageIndex = FLASH_PAGE_INDEX_END;
				FLASH_tFlashInfo.tTaskState = FLASH_TASK_INIT_DONE;
				break;
			}
			case FLASH_TASK_INIT_DONE:
			{
				FLASH_tFlashInfo.tTaskState = FLASH_TASK_READ;
				break;

			}
			case FLASH_TASK_READ:
			{
				if (FLASH_ucInitData())	//flash init is finished,and then read data from flash
				{
					FLASH_tFlashInfo.tTaskState = FLASH_TASK_READ_DONE;		//switch running state -> run
				}
				break;
			}
			case FLASH_TASK_READ_DONE:
			{
				rt_thread_mdelay(40);
				break;
			}
			case FLASH_TASK_SAVE:
			{
				FLASH_vSaveData();
				FLASH_tFlashInfo.tTaskState = FLASH_TASK_SAVE_DONE;
				break;
			}
			case FLASH_TASK_SAVE_DONE:
			{					
				rt_thread_mdelay(40);
				break;
				
			}
			default:break;
		}
		
	}
}
int Flash_iTaskHandler(void)
{
	rt_thread_t tid1;
	/* 创建Display_vTaskHandler_entry线程 */
	tid1 = rt_thread_create("FLASH_vTaskHandler_entry",
		FLASH_vTaskHandler_Entry,
		RT_NULL,
		1024,
		6,
		20);
	/* 创建成功则启动线程 */
	if (tid1 != RT_NULL)rt_thread_startup(tid1);

	
	return 0;
}
INIT_APP_EXPORT(Flash_iTaskHandler);


TFlashInfo* Flash_ptGetInfo(void)
{
	return &FLASH_tFlashInfo;
}
