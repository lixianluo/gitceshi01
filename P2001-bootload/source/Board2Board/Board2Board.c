#include "Board2Board.h"
#include "sw_timer.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
#include "can_iap.h"

static TIapCmdDef iap_cmd;

static uint8_t iap_data[8];


/**Reception functions declaration*/
static void B2B_vRxNMTControl(CAN_RxPacketTypeDef* ptHandler);
static void B2B_vRxNMTGuarding(CAN_RxPacketTypeDef* ptHandler);

static void B2B_vRxIapData(CAN_RxPacketTypeDef* ptHandler);
static void B2B_vRxCanIapCmd(CAN_RxPacketTypeDef* ptHandler);

/**Transmit functions declaration*/
static void B2B_vTxNMTStatus(CAN_TxPacketTypeDef* ptHandler);
static void B2B_vTxCanIapCmd(CAN_TxPacketTypeDef* ptHandler);
static void B2B_vTxCanIapData(CAN_TxPacketTypeDef* ptHandler);
static void B2B_vTxNMTControl(CAN_TxPacketTypeDef* ptHandler);

/**reception service message tbl*/
const TCanRxMsgTblDef B2B_tReceiveTbl[] = {
	/**MsgID,									ExtID,					DLC,				RTR,					ServiceFunc*/
	{CAN_MSG_ID_NMT_CTRL,						0,						2,					CAN_RTR_DATA,			B2B_vRxNMTControl},
	{CAN_MSG_ID_NMT_GUARDING,					0,						0,					CAN_RTR_REMOTE,			B2B_vRxNMTGuarding},

	/**CAN IAP*/
	{0,											CAN_MSG_ID_IAP_DATA,	8,					CAN_RTR_DATA,			B2B_vRxIapData},
	{0,											CAN_MSG_ID_IAP_CMD,		3,					CAN_RTR_DATA,			B2B_vRxCanIapCmd},



	/**reserved, don't remove*/
	{CAN_MSG_ID_END,							0,						8,					CAN_RTR_DATA,			NULL}
};


/**transmit service message tbl*/
const TCanTxMsgTblDef B2B_tTransmitTbl[] = {
	/**MsgID,									ExtID,					DLC,				RTR,					ServiceFunc*/
	/**CAN IAP*/
	{CAN_MSG_ID_NMT_GUARDING,					0,						1,					CAN_RTR_DATA,			B2B_vTxNMTStatus},
	{0,											CAN_MSG_ID_IAP_CMD,		3,					CAN_RTR_DATA,			B2B_vTxCanIapCmd},
	{0,											CAN_MSG_ID_IAP_DATA,	8,					CAN_RTR_DATA,			B2B_vTxCanIapData},

	{CAN_MSG_ID_NMT_CTRL,						0,						2,					CAN_RTR_DATA,			B2B_vTxNMTControl}
};

const uint8_t B2B_ucNumberOfElementsInSubsystem = B2B_TXIDX_COUNT;
uint8_t B2B_ucTransmitFlags[2];


/**Reception functions definition*/

static void B2B_vRxNMTControl(CAN_RxPacketTypeDef* ptHandler)
{
	/**check target node id*/
	if ((CAN_NODE_ID_BROADCAST == ptHandler->tRxFrame.Data[1]) || (CAN_NODE_ID_TND_SNR == ptHandler->tRxFrame.Data[1]))
	{
		uint8_t cmd = ptHandler->tRxFrame.Data[0];

		if (CAN_NMT_CS_START_NODE == cmd)
		{
			/**start remote node*/
			CAN_vSetCanStatus(CAN_STATUS_OPERATIONAL);
		}
		else if (CAN_NMT_CS_STOP_NODE == cmd)
		{
			/**stop remote node*/
			CAN_vSetCanStatus(CAN_STATUS_STOPPED);
		}
	}
	else
	{
		/**not this node, ignore the cmd*/
	}
}

static void B2B_vRxNMTGuarding(CAN_RxPacketTypeDef* ptHandler)
{
	CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_NMT_STATUS);
}

static void B2B_vRxIapData(CAN_RxPacketTypeDef* ptHandler)
{
	can_set_received_data(ptHandler->tRxFrame.ExtID, ptHandler->tRxFrame.Data);
}

static void B2B_vRxCanIapCmd(CAN_RxPacketTypeDef* ptHandler)
{
	uint8_t opcode = ptHandler->tRxFrame.Data[0];

	switch (opcode)
	{
		case CAN_IAP_REQUEST:
		{
			if ((CAN_NODE_ID_BROADCAST == ptHandler->tRxFrame.Data[1]) || (CAN_NODE_ID_TND_SNR == ptHandler->tRxFrame.Data[1]))
			{
				iap_cmd.opcode = CAN_IAP_REQUEST_ACK;
				CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);
			}

			break;
		}

		case CAN_IAP_REQUEST_ACK:
		{
			/**server mode*/
			can_set_received_data(ptHandler->tRxFrame.ExtID, ptHandler->tRxFrame.Data);

			break;
		}

		case CAN_IAP_REQUEST_ACK_CONFIRM:
		{
			if ((CAN_NODE_ID_BROADCAST == ptHandler->tRxFrame.Data[1]) || (CAN_NODE_ID_TND_SNR == ptHandler->tRxFrame.Data[1]))
			{
				/**start iap thread*/
				can_start_iap(CAN_IAP_CLENT_MODE);
			}

			break;
		}

		case CAN_IAP_STATUS:
		{
			can_set_received_data(ptHandler->tRxFrame.ExtID, ptHandler->tRxFrame.Data);

			break;
		}

		default: break;
	}
}



/**Transmit functions definition*/
static void B2B_vTxNMTStatus(CAN_TxPacketTypeDef* ptHandler)
{
	ptHandler->tTxFrame.Data[0] = CAN_tGetCanStatus();
}

static void B2B_vTxCanIapCmd(CAN_TxPacketTypeDef* ptHandler)
{
	switch (iap_cmd.opcode)
	{
		/**server mode*/
		case CAN_IAP_REQUEST:
		{
			/**byte 0: opcode*/
			ptHandler->tTxFrame.Data[0] = iap_cmd.opcode;
			/**byte 1: node address*/
			ptHandler->tTxFrame.Data[1] = iap_cmd.node_addr;
			/**byte 2: null*/
			ptHandler->tTxFrame.Data[2] = 0;

			break;
		}

		case CAN_IAP_REQUEST_ACK:
		{
			/**byte 0: opcode*/
			ptHandler->tTxFrame.Data[0] = iap_cmd.opcode;
			/**byte 1: node address*/
			ptHandler->tTxFrame.Data[1] = CAN_NODE_ID_TND_SNR;
			/**byte 2: null*/
			ptHandler->tTxFrame.Data[2] = 0;

			break;
		}

		case CAN_IAP_REQUEST_ACK_CONFIRM:
		{
			/**byte 0: opcode*/
			ptHandler->tTxFrame.Data[0] = iap_cmd.opcode;
			/**byte 1: node address*/
			ptHandler->tTxFrame.Data[1] = iap_cmd.node_addr;
			/**byte 2: null*/
			ptHandler->tTxFrame.Data[2] = 0;

			break;
		}

		case CAN_IAP_STATUS:
		{
			/**byte 0: opcode*/
			ptHandler->tTxFrame.Data[0] = iap_cmd.opcode;
			/**byte 1: node address*/
			ptHandler->tTxFrame.Data[1] = iap_cmd.node_addr;
			/**byte 2: ack*/
			ptHandler->tTxFrame.Data[2] = iap_cmd.status;
			break;
		}

		default:break;
	}
}

static void B2B_vTxCanIapData(CAN_TxPacketTypeDef* ptHandler)
{
	(void)memcpy(ptHandler->tTxFrame.Data, iap_data, 8u);
	(void)memset(iap_data, 0, 8u);	//!< clear iap data
}

static void B2B_vTxNMTControl(CAN_TxPacketTypeDef* ptHandler)
{
	/**byte 0: */
	ptHandler->tTxFrame.Data[0] = CAN_NMT_CS_STOP_NODE;

	/**byte 1: */
	ptHandler->tTxFrame.Data[1] = CAN_NODE_ID_BROADCAST;
}

void set_iap_cmd(TIapCmdDef* cmd)
{
	if (cmd->opcode <= CAN_IAP_STATUS)
	{
		iap_cmd.opcode = cmd->opcode;
		iap_cmd.node_addr = cmd->node_addr;
		iap_cmd.status = cmd->status;
	}
}

void set_iap_data(uint8_t* data)
{
	(void)memcpy(iap_data, data, 8u);
}