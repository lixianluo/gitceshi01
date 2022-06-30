/**
  ******************************************************************************
  * File Name          : CAN.h
  * Description        : This file provides code for the configuration
  *                      of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __can_H
#define __can_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "hc32f46x_can.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

#define CAN_NODE_BOARDS							0x00

#define CAN_MAX_SUBSYS_NUM		16
#if 1	//!< StdID
#define CAN_BASE_ID				0x2F0
#else	//!< ExtID
#define CAN_BASE_ID				0x10000
#endif
#define CAN_MSG_ID_END			0xFFFFFFFF
#define CAN_MSG_ID_NMT			0x000



enum {
	CAN_RTR_DATA	= 0,	//!< data frame
	CAN_RTR_REMOTE	= 1		//!< remote frame
};

enum {
	CAN_ID_STD = 0,	//!< Standard Id 
	CAN_ID_EXT = 1     //!< Extended Id 
};





/* USER CODE BEGIN Private defines */
typedef struct {
	stc_can_txframe_t tTxFrame;
	uint8_t ucNodeID;
	uint32_t ulMailbox;
}CAN_TxPacketTypeDef;

typedef struct {
	stc_can_rxframe_t tRxFrame;
	uint8_t ucNodeID;
}CAN_RxPacketTypeDef;

typedef struct {
	uint32_t ulMsgID;
	uint32_t ulExtID;
	uint32_t ulDLC;
	uint32_t ulRTR;	//!< remote transmit request

	void (*pvServiceFunc)(CAN_TxPacketTypeDef* tObject);
}TCanTxMsgTblDef;

typedef struct {
	uint32_t ulMsgID;
	uint32_t ulExtID;
	uint32_t ulDLC;
	uint32_t ulRTR;	//!< remote transmit request

	void (*pvServiceFunc)(CAN_RxPacketTypeDef* tObject);
}TCanRxMsgTblDef;


typedef struct _TCanObjectTable {
	uint8_t ucSubsystem;
	const TCanRxMsgTblDef* ptReceiveTable;
	const TCanTxMsgTblDef* ptTransmitTable;
	const uint8_t* pucNumberOfElementsInTransmitTable;
	uint8_t* pucMessageToTransmitBits;
}TCanObjectTable;


typedef enum {
	CAN_STATUS_INITIALISING = 0,
	CAN_STATUS_DISCONNECTED,
	CAN_STATUS_CONNECTING,
	CAN_STATUS_PREPARING,
	CAN_STATUS_STOPPED,
	CAN_STATUS_OPERATIONAL,
	CAN_STATUS_PRE_OPERATIONAL,
	CAN_STATUS_NUM
}TCanStatusDef;







typedef enum {
	CAN_TASK_INIT = 0,
	CAN_TASK_RUN
}TCanTaskStateDef;



enum {
	CAN_STATUS_SEND_DATA = 0x01,
	CAN_STATUS_RECEIVE_DONE = 0x02
};



/* USER CODE END Private defines */
void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */
extern void CAN_vTransmitMessage(uint8_t ucSubsystem, uint8_t ucMessageNumber);
extern void CAN_vSetCanStatus(TCanStatusDef tStatus);
extern TCanStatusDef CAN_tGetCanStatus(void);

extern void CAN_vTaskHandler(void);
/* USER CODE END Prototypes */

















extern void CAN_vSetBit(uint8_t* pucBitField, uint16_t uiBitId, uint8_t ucOnOff);
extern uint8_t CAN_ucGetBit(const uint8_t* pucBitField, uint16_t uiBitId);


#ifdef __cplusplus
}
#endif
#endif /*__ can_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
