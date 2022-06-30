#ifndef _BOARD_2_BOARD_H
#define _BOARD_2_BOARD_H

#include "defines.h"
#include "can.h"

extern const TCanRxMsgTblDef B2B_tReceiveTbl[];
extern const TCanTxMsgTblDef B2B_tTransmitTbl[];
extern const uint8_t B2B_ucNumberOfElementsInSubsystem;
extern uint8_t B2B_ucTransmitFlags[2];


/**CAN Node ID ------------------------------------------------------------*/
#define CAN_NODE_ID_BROADCAST		0
#define CAN_NODE_ID_TND_CTL			1
#define CAN_NODE_ID_TND_INT			2
#define CAN_NODE_ID_TND_DRV			3
#define CAN_NODE_ID_TND_SNR			4

#define CAN_NODE_ID					CAN_NODE_ID_TND_SNR		//!< change it when need!


/**MsgID definition*/
/**
* NMT-Master -> NMT-Slave
* 0x700 + Node_ID, remote frame
*
* NMT-Slave -> NMT-Master
* 0x700 + Node_ID,
*-----------------------------*/
#define CAN_MSG_ID_NMT_GUARDING					(0x700 + CAN_NODE_ID)	//!< NMT node guarding	

/**transmit and receive*/
#define CAN_MSG_ID_NMT_CTRL						0x000	//!< NMT Module Control

#define B2B_MSG_ID_FAULT_CODE					0x2F4
#define B2B_MSG_ID_FW_VER						0x2F0


/**receive*/
#define B2B_MSG_ID_SET_TND_CTL_INFO				0x2F1
#define B2B_MSG_ID_TND_DRV_INFO					0x2F9
#define B2B_MSG_ID_PID_UPLOAD					0x2FB

#define CAN_MSG_ID_IAP_DATA						0x10000000


/**transmit*/
#define B2B_MSG_ID_ODOMETER						0x2F2		
#define B2B_MSG_ID_FAULT_CODE					0x2F4
#define B2B_MSG_ID_TND_CTL_INFO					0x2F5
#define B2B_MSG_ID_TND_INT_INFO					0x2F6
#define B2B_MSG_ID_SET_TND_DRV_INFO				0x2F8
#define B2B_MSG_ID_PID_SET						0x2FA


#define CAN_MSG_ID_IAP_CMD						0x11000000


/**NMT CS Definition-------------------------------------------------------*/
#define CAN_NMT_CS_START_NODE				1	//!< start remote node
#define CAN_NMT_CS_STOP_NODE				2	//!< stop remote node
#define CAN_NMT_CS_ENTER_PRE_OP_STATE		128	//!< enter pre-operational state
#define CAN_NMT_CS_RESET_NODE				129	//!< reset node
#define CAN_NMT_CS_RESET_COM				130	//!< reset communication






#define B2B_COM_TIMEOUT						100	//!< ms

enum {
	CAN_IAP_REQUEST = 0,
	CAN_IAP_REQUEST_ACK,
	CAN_IAP_REQUEST_ACK_CONFIRM,
	CAN_IAP_STATUS
};

enum {
	B2B_ID_TND_CTL = 0,
	B2B_ID_TND_INT,
	B2B_ID_TND_DRV,
	B2B_ID_TND_SNR,
	B2B_ID_NUM
};


/* Message Indexs - Transmit */
enum {
	B2B_MSGIDX_SEND_NMT_STATUS,
	B2B_MSGIDX_SEND_IAP_CMD,
	B2B_MSGIDX_SEND_IAP_DATA,
	B2B_MSGIDX_NMT_CTRL,
	B2B_TXIDX_COUNT
};

typedef enum {
	B2B_ACK_NULL	= 0,
	B2B_ACK_OK		= 1
}TB2BAckDef;

typedef enum {
	B2B_BUS_IDLE = 0,
	B2B_BUS_SEND_DATA,
	B2B_BUS_RECEIVE_DATA,
	B2B_BUS_MSG_PROCESS
}TB2BbusStatusDef;

typedef struct {
	uint8_t opcode;
	uint8_t node_addr;
	uint8_t status;
}TIapCmdDef;

extern void set_iap_cmd(TIapCmdDef* cmd);
extern void set_iap_data(uint8_t* data);
























#endif

