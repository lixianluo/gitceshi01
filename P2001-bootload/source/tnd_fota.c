#include "tnd_fota.h"
#include "flash.h"
#include "hc32f46x_interrupts.h"
//#include "hc32f460keta.h"
#include "hc32f46x_pwc.h"
#include "hc32f46x_crc.h"
#include "sw_timer.h"
#include "can_iap.h"
#include "ymodem_ota.h"
#include "Board2Board/Board2Board.h"
#include <string.h>


static TfotaTaskStateDef FOTA_tTaskState = (TfotaTaskStateDef)0;
static uint32_t app_file_cur_size = 0;
uint32_t crc_rom_value = 0;
uint32_t crc_rom_cur_size = 0;
uint32_t app_file_size = 0;
uint8_t ota_target_dev = 0;

/**rt_fota_head for TND_CTL board*/
typedef struct {
	char type[4];					//!< RBL head
	uint16_t fota_algo;				//!< algorithm configuration
	uint8_t fm_time[6];				//!< timestamp of raw bin file
	char app_part_name[16];			//!< app part name
	char download_version[24];		//!< firmware version
	char current_version[24];		//!< current version. meaningless
	uint32_t code_crc;				//!< code crc after packing, 
	uint32_t hash_val;				//!< raw code hash value
	uint32_t raw_size;				//!< raw code size
	uint32_t com_size;				//!< code size after packing
	uint32_t head_crc;				//!< crc of rbl head
} rt_fota_part_head, * rt_fota_part_head_t;

static rt_fota_part_head fota_part_head;

uint32_t head_crc;
uint32_t file_crc;


typedef void (*fota_app_func)(void);

static uint8_t FOTA_ucCheckFlashCrc(uint32_t ulFlashAddr);


static uint8_t FOTA_ucCheckFlashCrc(uint32_t ulFlashAddr)
{
	uint32_t crc_in_flash;

	crc_in_flash = *(__IO uint32_t*)(ulFlashAddr + FLASH_DOWNLOAD_CRC32_OFFSET);
	crc_rom_value = CRC_INIT;
	app_file_size = *(uint32_t*)(FLASH_DOWNLOAD_START_ADDR + FLASH_DOWNLOAD_FILE_SIZE_OFFSET);
	crc_rom_cur_size = 0;

	while (crc_rom_cur_size < app_file_size)
	{
		crc_rom_value = CRC_Calculate32B(crc_rom_value, (uint32_t*)(ulFlashAddr + FLASH_DOWNLOAD_RAM_VETOR_OFFSET + crc_rom_cur_size), app_file_size / 4);
		crc_rom_cur_size += app_file_size;
	}

	return (crc_in_flash == crc_rom_value);	//!< 0: crc wrong; 1: crc match
}

static uint32_t FOTA_ulCalculateCrc(uint32_t ulAddr, uint32_t ulDataLen)
{
	uint32_t crc = CRC_INIT;

	crc = CRC_Calculate32B(crc, (uint32_t*)(ulAddr), ulDataLen / 4);

	return crc;
}


static uint8_t FOTA_ucStartApplication(uint32_t ulAppAddr)
{
	IRQn_Type irq;
	fota_app_func app_func = NULL;

	/**check if is 0x1FFFxxxx*/
	if (((*(__IO uint32_t*)ulAppAddr) & 0xffff0000) != 0x1fff0000)
	{
		/**"Illegal Flash code*/
		return 1;
	}

	__disable_irq();
	for (irq = 0; irq < Int143_IRQn; irq++)
	{
		enIrqResign(irq);
	}

	/**app start address*/
	app_func = (fota_app_func) * (__IO uint32_t*)(ulAppAddr + 4);

	/* Configure main stack */
	__set_MSP(*(__IO uint32_t*)ulAppAddr);

	/* Rebase the vector table base address */
	SCB->VTOR = ((uint32_t)ulAppAddr & SCB_VTOR_TBLOFF_Msk);

	/* jump to application */
	app_func();

	return 0;	//!< ok
}

static uint8_t FOTA_ucCheckFirmware(uint32_t ulDownloadAddr)
{
	char* file_name;
	//uint32_t ulData = 0;
	uint32_t magic_word_begin = IAP_MAGIC_WORD_BEGIN;
	uint8_t result = 0;

	(void)memcpy((uint8_t*)&fota_part_head, (uint8_t*)ulDownloadAddr, sizeof(rt_fota_part_head));
	
	/**if TND_CTL ota file*/
	if (strcmp(fota_part_head.type, "RBL") == 0)
	{
		/**check rbl head crc*/
		head_crc = FOTA_ulCalculateCrc(ulDownloadAddr, sizeof(rt_fota_part_head) - 4);	//!< rbl head size - 4

		if (head_crc != fota_part_head.head_crc)
		{
			/**head crc wrong*/

			/**erase download area*/
			//FLASH_vEraseSector(FLASH_DOWNLOAD_START_ADDR, FLASH_DOWNLOAD_SIZE);

			return 0;
		}

		file_crc = FOTA_ulCalculateCrc(ulDownloadAddr + sizeof(rt_fota_part_head), fota_part_head.com_size);

		if (file_crc != fota_part_head.code_crc)
		{
			/**ota file crc wrong*/

			/**erase download area*/
			//FLASH_vEraseSector(FLASH_DOWNLOAD_START_ADDR, FLASH_DOWNLOAD_SIZE);

			return 0;
		}

		/**congratulations， crc check pass*/
		app_file_size = fota_part_head.com_size + sizeof(rt_fota_part_head);
		can_set_target_device(CAN_NODE_ID_TND_CTL);
		return 2;	//!< fw forward
	}

	/**check magic word*/
	if (0 != memcmp((uint8_t*)(ulDownloadAddr + FLASH_DOWNLOAD_MAGIC_WORD_BEGIN_OFFSET), (uint8_t*)&magic_word_begin, 4u))
	{
		return 0;	//!< magic word wrong, skip to app
	}

	/**check flash crc*/
	if (0 == FOTA_ucCheckFlashCrc(FLASH_DOWNLOAD_START_ADDR))
	{
		/**erase download area*/
		FLASH_vEraseSector(FLASH_DOWNLOAD_START_ADDR, FLASH_DOWNLOAD_SIZE);

		return 0;	//!< crc wrong, skip to app
	}

	/**check target device*/
	file_name = (char*)(ulDownloadAddr + FLASH_DOWNLOAD_TARGET_DEVICE_OFFSET);

	if (0 != strcmp(file_name, FOTA_DEVICE_NAME))
	{
		if (0 == strcmp(file_name, TND_CTL_DEVICE_NAME))
		{
			can_set_target_device(CAN_NODE_ID_TND_CTL);
			result = 2;	//!< fw forward
		}
		else if (0 == strcmp(file_name, TND_DRV_DEVICE_NAME))
		{
			can_set_target_device(CAN_NODE_ID_TND_DRV);
			result = 2;	//!< fw forward
		}
		else if (0 == strcmp(file_name, TND_INT_DEVICE_NAME))
		{
			can_set_target_device(CAN_NODE_ID_TND_INT);
			result = 2;	//!< fw forward
		}
		else
		{
			return 0;	//!< target device wrong, skip to app
		}
	}
	else
	{
		result = 1;		//!< fw update
	}

	///**check ram vector*/
	//if (((*(__IO uint32_t*)(ulDownloadAddr + FLASH_DOWNLOAD_RAM_VETOR_OFFSET)) & 0xffff0000) != 0x1fff0000)
	//{
	//	/**"Illegal Flash code, skip to app*/
	//	return 0;
	//}

	return result;
}

static uint8_t FOTA_ucFirmwareUpdate(uint32_t ulDownloadAddr)
{
	/* Implement upgrade, copy firmware partition to app partition */
	app_file_cur_size = 0;
	/**erase flash */
	FLASH_vEraseSector(FLASH_APP_START_ADDR, FLASH_APP_SIZE);

	while (app_file_cur_size < app_file_size)
	{
		uint32_t ulData = *(uint32_t*)(ulDownloadAddr + FLASH_DOWNLOAD_RAM_VETOR_OFFSET + app_file_cur_size);

		/**write to flash*/
		FLASH_vWriteBytes(FLASH_APP_START_ADDR + app_file_cur_size, (uint8_t*)&ulData, 4u);

		app_file_cur_size += 4u;
	}

	/**copy application bin file info to app area*/
	FLASH_vWriteBytes(FLASH_APP_INFO_ADDR, (uint8_t*)(ulDownloadAddr), 32u);

	/**erase download area*/
	FLASH_vEraseSector(FLASH_DOWNLOAD_START_ADDR, FLASH_DOWNLOAD_SIZE);

	return 0;
}

static uint8_t FOTA_ucFirmwareForward(uint32_t ulDownloadAddr)
{
	/**erase download area*/
	can_start_iap(CAN_IAP_SERVER_MODE);

	if (IAP_SERVER_STAGE_FINISHED == can_iap_get_task_state())
	{
		FLASH_vEraseSector(FLASH_DOWNLOAD_START_ADDR, FLASH_DOWNLOAD_SIZE);
		return 0;
	}

	return 1;
}



void FOTA_vTaskHandler(void)
{
	switch (FOTA_tTaskState)
	{
		case FOTA_TASK_INIT:  //OTA初始化
		{
			FOTA_tTaskState = FOTA_TASK_CHECK_UPDATE; 
			//can_start_iap(CAN_IAP_CLENT_MODE);			//该项目没有用到CAN
			ymodem_start_ota();								//将ymodem_ota_sem = 1;
			//!修改 改为2分钟
			TMR_vSetTime(TMR_FOTA_POLLING, TMR_TIME_MIN2TICKS(2));
			//TMR_vSetTime(TMR_FOTA_POLLING, TMR_TIME_MS2TICKS(200));		//!< 200ms to check can iap request   
			break;
		}
		
		case FOTA_TASK_CHECK_UPDATE:   //OTA检查更新
		{
			/**check updating cmd from CAN*/
			if (TMR_bIsTimeExpired(TMR_FOTA_POLLING))
			{
				FOTA_tTaskState = FOTA_TASK_CHECK_FW;
			}

			//if (can_is_data_received() || ymodem_is_ota_data_received())	//判断ymode数据是否被接收

			if (ymodem_is_ota_data_received()	//判断ymode数据是否被接收
			{
				FOTA_tTaskState = FOTA_TASK_DOWNLOADING;
			}

			break;
		}

		case FOTA_TASK_DOWNLOADING: //OTA任务下载
		{
			break;
		}
			
		case FOTA_TASK_CHECK_FW:
		{
			uint8_t result = FOTA_ucCheckFirmware(FLASH_DOWNLOAD_START_ADDR); //检查固件 

			/**check download area in flash*/
			if (0 == result)
			{
				FOTA_tTaskState = FOTA_TASK_START_APP;
			}
			else if(1 == result)
			{
				FOTA_tTaskState = FOTA_TASK_FW_UPDATE; 
			}
			else if (2 == result)
			{
				FOTA_tTaskState = FOTA_TASK_FW_FORWARD; 
			}

			break;
		}

		case FOTA_TASK_FW_UPDATE://固件更新
		{
			if (0 == FOTA_ucFirmwareUpdate(FLASH_DOWNLOAD_START_ADDR))
			{
				FOTA_tTaskState = FOTA_TASK_START_APP;
			}

			break;
		}

		case FOTA_TASK_FW_FORWARD: //固件传输
		{
			if (0 == FOTA_ucFirmwareForward(FLASH_DOWNLOAD_START_ADDR))
			{
				FOTA_tTaskState = FOTA_TASK_START_APP;
			}

			break;
		}
			
		case FOTA_TASK_START_APP:    //检查app验证并开始
		{
			/**check app validation and start*/
			if (FOTA_ucStartApplication(FLASH_APP_START_ADDR) != 0) //如果等于0就从app启动   否则跳到检查更新
			{
				FOTA_tTaskState = FOTA_TASK_CHECK_UPDATE;
				can_iap_task_init();
				ymodem_ota_task_init();

				TMR_vSetTime(TMR_FOTA_POLLING, TMR_TIME_SEC2TICKS(3600));
			}

			break;
		}

		default: break;
	}
}

uint32_t FOTA_ulGetAppFileSize(void)
{
	return app_file_size;
}