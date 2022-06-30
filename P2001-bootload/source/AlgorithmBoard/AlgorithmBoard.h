#ifndef _ALGORITHM_BOARD_H
#define _ALGORITHM_BOARD_H

#include "defines.h"
#include "uart.h"
#include "gpio.h"




//#define ALGO_UART_PORT	huart3



/****************Message protocol definitions***********************************************************************
* Head0 (1 byte) + Header1 (1 byte) + Payload Legnth (1 byte) + Payload (Payload length bytes) + Checksum (1 byte) *
* Checksum: XOR of Payload length and Payload																	   *
*******************************************************************************************************************/
#define ALGO_MSG_HEADER0		0xFF
#define ALGO_MSG_HEADER1		0xEE

/**UID definition*/
#define ALGO_MSG_UID_SINGLE_WHEEL				0x01	//!< single wheel speed control cmd
#define ALGO_MSG_UID_DIFF_CHASSIS				0x02	//!< differential chassis speed control cmd
#define ALGO_MSG_UID_ACKM_CHASSIS				0x03	//!< ackerman chassis speed control cmd
#define ALGO_MSG_UID_CHASSIS_ID_REQ				0x10	//!< request for chassis identification code
#define ALGO_MSG_UID_CHARGE_TASK				0x11
#define ALGO_MSG_UID_UPPER_MACHINE_STATUS		0x30	//!< system running status, sensor status
#define ALGO_MSG_UID_UPPER_MACHINE_SYS_INFO		0x31	//!< firmware version, protocal version ...
#define ALGO_MSG_UID_CHASSIS_FAULT_CODE			0x32	//!< firmware version, protocal version ...


#define ALGO_MSG_UID_HEARTBEAT					0x40	//!< periodically transmit
#define ALGO_MSG_UID_TIME_STAMP_SYNC			0x50	//!< periodically transmit

#define ALGO_MSG_UID_WRITE_PID					0x61	//!< set pid value
#define ALGO_MSG_UID_GET_PID					0x62	//!< read pid value 
#define ALGO_MSG_UID_UPLOAD_PID					0x63	//!< send pid value to the upper machine


#define ALGO_DATALEN_UID_DIFF_CHASSIS			10		//!< bytes




#define ALGO_MSG_PAYLOAD_LEN_MAX		50

#define ALGO_COM_TIMEOUT				50	//!< ms
#define ALGO_COM_TX_TIMEOUT				10	//!< ms

#define ALGO_COM_ERROR_RETRY_COUNT		3	


typedef struct {
	uint8_t ucHeader0;
	uint8_t ucHeader1;
	uint8_t ucPayloadLen;
	uint8_t ucPayload[ALGO_MSG_PAYLOAD_LEN_MAX];
}TAlgoMsgObjDef;

typedef struct {
	uint8_t ucMsgUID;
	void (*pvServiceFunc)(TAlgoMsgObjDef* tObject);
}TAlgoMsgTblDef;

typedef enum {
	ALGO_TASK_INIT = 0,
	ALGO_TASK_IDLE,
	ALGO_TASK_POLLING
}TAlgoTaskDef;


enum {
	ALGO_FLAG_RECEPTION_DONE = 0x01,
	ALGO_FLAG_SEND_DATA		 = 0x02,
	ALGO_FLAG_RECEIVING		 = 0x04
};






/**transmit message index definition*/
enum {
	ALGO_MSG_ID_SINGLE_WHEEL	= 0,
	ALGO_MSG_ID_CHASSIS_ID_REQ,
	ALGO_MSG_ID_CHARGE_TASK,
	ALGO_MSG_ID_CHASSIS_FAULT_CODE,

	ALGO_MSG_ID_SYNC_TIME_STAMP,
	ALGO_MSG_ID_UPLOAD_PID,
	ALGO_MSG_ID_NUM
};






extern void ALGO_vRxIdleEventHandler(void);
extern void ALGO_vTxCpltCallback(void);
extern void ALGO_vReceptionInit(void);













#endif
