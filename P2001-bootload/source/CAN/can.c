/**
  ******************************************************************************
  * File Name          : CAN.c
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

/* Includes ------------------------------------------------------------------*/
#include "can.h"
#include "sw_timer.h"
#include <string.h>
#include <stdio.h>

#include "hc32f46x_pwc.h"
#include "hc32f46x_interrupts.h"
#include "hc32f46x_gpio.h"


/* USER CODE BEGIN 0 */
#include "Board2Board.h"
#include "can_iap.h"
/* USER CODE END 0 */

static CAN_TxPacketTypeDef CAN_tTxPacket;

#define CAN_RX_PACKET_NUM       8
static CAN_RxPacketTypeDef CAN_tRxPacket[CAN_RX_PACKET_NUM];
static volatile uint8_t CAN_ucRxPacketHeadIndex = 0;    //!< next new rx packet index
static volatile uint8_t CAN_ucRxPacketTailIndex = 0;    //!< handled rx packet index    


static TCanTaskStateDef CAN_tTaskState = (TCanTaskStateDef)0;
static volatile uint8_t CAN_tBusStatus = 0;
static uint16_t CAN_uiTransmitFlagSubsystemIndex = 0; //!< Variable used to keep track of whether minimum one message is scheduled for transmission in a subsystem. Please note that only 16 subsystems may be defined, as this variable is 16 bits long.
static uint8_t CAN_ucSubsystemIndex = 0;    //!< This is a marker of the subsystem of the presently sent message, in case the message transmission must be repeated certain times.
static TCanStatusDef CAN_tNodeStatus = (TCanStatusDef)0;



static void CAN_vIrqCallBack(void);
static void CAN_vCheckTransmitTask(void);
static uint8_t CAN_ucGetSubsystemIndex(uint8_t ucSubsystem);
static void CAN_vReceptionHandler(CAN_RxPacketTypeDef* ptHandler);



static const TCanObjectTable CAN_tObjectTable[] = {
    /**Subsystem,           receive table,              transmit table,          number of elements in subsystem,                       transmit flag*/
    {CAN_NODE_BOARDS,    B2B_tReceiveTbl,           B2B_tTransmitTbl,        (uint8_t*)&B2B_ucNumberOfElementsInSubsystem,           B2B_ucTransmitFlags},
};



/************************************************************************************************************
* 32 Bit Filter - Identifier Mask
* ----------------------------< MSB---------------LSB>-------------------------------------------------------
*            | Byte 3          |    Byte 2                  |       Byte 1          |   Byte 0              |
* ID        - CAN_FxR1[31:24]  |  CAN_FxR1[23:16]           |       CAN_FxR1[15:8]  |   CAN_FxR1[7:0]
* Mask      - CAN_FxR2[31:24]  |  CAN_FxR2[23:16]           |       CAN_FxR2[15:8]  |   CAN_FxR2[7:0]
* Mapping   - STDID[10:3]      |  STDID[2:0] EXID[17:13]    |       EXID[12:5]      |   EXID[4:0] IDE RTR 0
************************************************************************************************************/


/* CAN init function */
void MX_CAN_Init(void)
{
    stc_pwc_ram_cfg_t       stcRamCfg;
    stc_can_init_config_t   stcCanInitCfg;
    stc_can_filter_t        stcFilter;

    stc_irq_regi_conf_t     stcIrqRegiConf;

    MEM_ZERO_STRUCT(stcRamCfg);
    MEM_ZERO_STRUCT(stcCanInitCfg);
    MEM_ZERO_STRUCT(stcFilter);

    //<<Enable can peripheral clock and buffer(ram)
    stcRamCfg.enRamOpMd = HighSpeedMd;
    stcRamCfg.enCan = DynamicCtl;
    PWC_RamCfg(&stcRamCfg);
    PWC_Fcg1PeriphClockCmd(PWC_FCG1_PERIPH_CAN, Enable);

    //<<CAN GPIO config
    PORT_SetFunc(PortB, Pin06, Func_Can1_Rx, Disable);
    PORT_SetFunc(PortB, Pin07, Func_Can1_Tx, Disable);
    //<<Can bit time config
    /**BT = 1 / 500Khz = ((SEG_1 + 2) + (SEG_2 + 1)) * (PSC + 1) / Fcan_clk*/
    /**Fcan_clk = 8M / 2*/
    stcCanInitCfg.stcCanBt.PRESC = 2u - 1u;    //!< 4M; range 4 ~ 24M
    stcCanInitCfg.stcCanBt.SEG_1 = 5u - 2u;
    stcCanInitCfg.stcCanBt.SEG_2 = 3u - 1u;
    stcCanInitCfg.stcCanBt.SJW = 3u - 1u;

    stcCanInitCfg.stcWarningLimit.CanErrorWarningLimitVal = 10u;
    stcCanInitCfg.stcWarningLimit.CanWarningLimitVal = 10u;

    stcCanInitCfg.enCanRxBufAll = CanRxNormal;
    stcCanInitCfg.enCanRxBufMode = CanRxBufNotStored;
    stcCanInitCfg.enCanSAck = CanSelfAckEnable;
    stcCanInitCfg.enCanSTBMode = CanSTBFifoMode;

    CAN_Init(&stcCanInitCfg);

    //<<Can filter config
    stcFilter.enAcfFormat = CanAllFrames;
    stcFilter.enFilterSel = CanFilterSel1;
    stcFilter.u32CODE = 0x00000000u;
    stcFilter.u32MASK = 0x1FFFFFFFu;
    CAN_FilterConfig(&stcFilter, Enable);

    //<<Can Irq Enable
    CAN_IrqCmd(CanRxIrqEn, Enable);
    /**Can tx ir*/
    CAN_IrqCmd(CanTxPrimaryIrqEn, Enable);
    /**Can tx ir*/
    //CAN_IrqCmd(CanErrorIrqEn, Enable);
    ///**Can passive error ir*/
    //CAN_IrqCmd(CanErrorPassiveIrqEn, Enable);
    ///**Can arbilost  error ir*/
    //CAN_IrqCmd(CanArbiLostIrqEn, Enable);
    /**Can bus error ir*/
    CAN_IrqCmd(CanBusErrorIrqEn, Enable);

    stcIrqRegiConf.enIRQn = Int013_IRQn;
    stcIrqRegiConf.enIntSrc = INT_CAN_INT;
    stcIrqRegiConf.pfnCallback = &CAN_vIrqCallBack;
    enIrqRegistration(&stcIrqRegiConf);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_01);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    ///**Can tx ir*/
    //CAN_IrqCmd(CanTxPrimaryIrqEn, Enable);
    //stcIrqRegiConf.enIRQn = Int024_IRQn;
    //stcIrqRegiConf.enIntSrc = INT_CAN_INT;
    //stcIrqRegiConf.pfnCallback = &CAN_vIrqCallBack;
    //enIrqRegistration(&stcIrqRegiConf);
    //NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_01);
    //NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    //NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);

    ///**Can tx ir*/
    //CAN_IrqCmd(CanErrorIrqEn, Enable);
    //stcIrqRegiConf.enIRQn = Int029_IRQn;
    //stcIrqRegiConf.enIntSrc = INT_CAN_INT;
    //stcIrqRegiConf.pfnCallback = &CAN_vIrqCallBack;
    //enIrqRegistration(&stcIrqRegiConf);
    //NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_01);
    //NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    //NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}


/* USER CODE BEGIN 1 */

static void CAN_vReceptionHandler(CAN_RxPacketTypeDef* ptHandler)
{
    uint8_t ucSubsystem;
    uint8_t ucSubsystemIndex;
    uint8_t ucMessageIndex;
    uint32_t ulMsgID;
    uint32_t ulExtID;
    uint32_t ulDLC;
    uint32_t ulRTR;
    const TCanObjectTable* ptBusObject;

    if (ptHandler->tRxFrame.Cst.Control_f.IDE)
    {
        ucSubsystem = CAN_NODE_BOARDS;
        ulExtID = ptHandler->tRxFrame.ExtID & 0xFF000000;
        ulMsgID = 0;
    }
    else if ((0x2F0 <= ptHandler->tRxFrame.StdID) && (0x2FF >= ptHandler->tRxFrame.StdID))
    {
        ucSubsystem = CAN_NODE_BOARDS;
        ulMsgID = ptHandler->tRxFrame.StdID;
        ulExtID = 0;
    }
    else
    {
        ucSubsystem = ptHandler->tRxFrame.StdID & 0xF; //!< should check it
        ulMsgID = ptHandler->tRxFrame.StdID;
        ulExtID = 0;
    }

    ulDLC = ptHandler->tRxFrame.Cst.Control_f.DLC;
    ulRTR = ptHandler->tRxFrame.Cst.Control_f.RTR;
    ptHandler->ucNodeID = ucSubsystem;

    ucSubsystemIndex = CAN_ucGetSubsystemIndex(ucSubsystem);

    if (ucSubsystemIndex < (sizeof(CAN_tObjectTable) / sizeof(CAN_tObjectTable[0])))
    {
        ptBusObject = CAN_tObjectTable + ucSubsystemIndex;

        /**find message index in subsystem*/
        for (ucMessageIndex = 0; ; ucMessageIndex++)
        {
            if ((ulMsgID == ptBusObject->ptReceiveTable[ucMessageIndex].ulMsgID) \
                && (ulExtID == ptBusObject->ptReceiveTable[ucMessageIndex].ulExtID) \
                && (ulDLC == ptBusObject->ptReceiveTable[ucMessageIndex].ulDLC) \
                && (ulRTR == ptBusObject->ptReceiveTable[ucMessageIndex].ulRTR))
            {
                break;
            }
            else
            {
                if (CAN_MSG_ID_END == ptBusObject->ptReceiveTable[ucMessageIndex].ulMsgID)
                {
                    return; //!< message not found
                }
            }
        }

        /**call service function*/
        if (ptBusObject->ptReceiveTable[ucMessageIndex].pvServiceFunc != NULL)
        {
            ptBusObject->ptReceiveTable[ucMessageIndex].pvServiceFunc(ptHandler);
        }
    }
}

/**
  * @brief  Rx Fifo 0 message pending callback in non blocking mode
  * @param  CanHandle: pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* canHandle)
//{
//    /**clear first*/
//    memset((void*)&CAN_tRxPacket[CAN_ucRxPacketHeadIndex], 0, sizeof(CAN_RxPacketTypeDef));
//
//    /* Get RX message */
//    if (HAL_CAN_GetRxMessage(canHandle, CAN_RX_FIFO0, &CAN_tRxPacket[CAN_ucRxPacketHeadIndex].tRxHeader, CAN_tRxPacket[CAN_ucRxPacketHeadIndex].ucData) == HAL_OK)
//    {
//        CAN_ucRxPacketHeadIndex = (CAN_ucRxPacketHeadIndex + 1) % CAN_RX_PACKET_NUM;
//    }
//
//    HAL_CAN_ActivateNotification(canHandle, CAN_IT_RX_FIFO0_MSG_PENDING);
//}

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
static void CAN_vIrqCallBack(void)
{
    if (true == CAN_IrqFlgGet(CanRxIrqFlg))
    {
        CAN_IrqFlgClr(CanRxIrqFlg);

        /**clear first*/
        (void)memset((void*)&CAN_tRxPacket[CAN_ucRxPacketHeadIndex], 0, sizeof(CAN_RxPacketTypeDef));

        uint8_t code = CAN_Receive(&CAN_tRxPacket[CAN_ucRxPacketHeadIndex].tRxFrame);
        CAN_ucRxPacketHeadIndex = (CAN_ucRxPacketHeadIndex + 1) % CAN_RX_PACKET_NUM;
    }

    if (true == CAN_IrqFlgGet(CanTxPrimaryIrqFlg))
    {
        CAN_IrqFlgClr(CanTxPrimaryIrqFlg);
        DEF_RES(CAN_tBusStatus, CAN_STATUS_SEND_DATA);
    }

    //if (true == CAN_IrqFlgGet(CanErrorIrqFlg))
    //{
    //    CAN_IrqFlgClr(CanErrorIrqFlg);

    //}

    //if (true == CAN_IrqFlgGet(CanErrorPassiveIrqFlg))
    //{
    //    CAN_IrqFlgClr(CanErrorPassiveIrqFlg);

    //}

    //if (true == CAN_IrqFlgGet(CanArbiLostIrqFlg))
    //{
    //    CAN_IrqFlgClr(CanArbiLostIrqFlg);

    //}

    if (true == CAN_IrqFlgGet(CanBusErrorIrqFlg))
    {
        CAN_IrqFlgClr(CanBusErrorIrqFlg);

    }
}
/* USER CODE END 1 */

/**
  * @brief  Transmission Mailbox 0 complete callback.
  * @param  hcan pointer to a CAN_HandleTypeDef structure that contains
  *         the configuration information for the specified CAN.
  * @retval None
  */
//void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef* canHandle)
//{
//    /* NOTE : This function Should not be modified, when the callback is needed,
//              the HAL_CAN_TxMailbox0CompleteCallback could be implemented in the
//              user file
//     */
//    HAL_CAN_DeactivateNotification(canHandle, CAN_IT_TX_MAILBOX_EMPTY);
//    DEF_RES(CAN_tBusStatus, CAN_STATUS_SEND_DATA);
//}

static uint8_t CAN_ucGetSubsystemIndex(uint8_t ucSubsystem)
{
    uint8_t ucSubsystemIndex;             //The index of the subsystem in the list of modules

    for (ucSubsystemIndex = 0; ucSubsystemIndex < (sizeof(CAN_tObjectTable) / sizeof(CAN_tObjectTable[0])); ucSubsystemIndex++)
    {
        if (CAN_tObjectTable[ucSubsystemIndex].ucSubsystem == ucSubsystem)
        {
            return ucSubsystemIndex;
            //[MISRA 2004 Rule 14.7]: This function contains a second return path. However, for such a compact function this is not likely to be used falsely.
        }
        else
        {
            //Index not found yet, continue searching.
        }
    }

    //Subsystem not found - 'cause it isn't defined
    return 0xFF;    // 0; //If the requested Subsystem is not found in the table, the first index (0) is returned, as this is always present.
}


static void CAN_vCheckTransmitTask(void)
{
    uint8_t ucMessageIndex = 0;
    uint8_t ucMessageToTransmit = 0;
    uint16_t uiMask;
    const TCanObjectTable* ptBusObject;

    if (DEF_TEST(CAN_tBusStatus, CAN_STATUS_SEND_DATA))
    {
        if (TMR_bIsTimeExpired(TMR_CAN_TX_TIMEOUT))
        {
            DEF_RES(CAN_tBusStatus, CAN_STATUS_SEND_DATA);

            M4_CAN->CFG_STAT_f.RESET = 0u;  //!< solve can DEAD
        }

        return;     //!< message is still transmitting
    }

    if (CAN_uiTransmitFlagSubsystemIndex != 0)
    {
        uiMask = 0x0001;
        for (CAN_ucSubsystemIndex = 0; CAN_ucSubsystemIndex < (sizeof(CAN_tObjectTable) / sizeof(CAN_tObjectTable[0])); CAN_ucSubsystemIndex++)   //!< find the bit number
        {
            if ((CAN_uiTransmitFlagSubsystemIndex & uiMask) != 0U)
            {
                //Found subsystem with message due for transmission.
                break;
            }
            else
            {
                //Continue searching ... 
                uiMask <<= 1;
            }
        }

        ptBusObject = CAN_tObjectTable + CAN_ucSubsystemIndex;
        for (ucMessageIndex = 0U; ucMessageIndex < *ptBusObject->pucNumberOfElementsInTransmitTable; ucMessageIndex++)
        {
            if (CAN_ucGetBit(ptBusObject->pucMessageToTransmitBits, ucMessageIndex) != 0)
            {
                //!< ucMessageIndex within the corresponding Subsystem has been found
                ucMessageToTransmit = 1U;
                break;
            }
            else
            {
                //!< Continue searching for Message to send.
            }
        }

        if (ucMessageToTransmit)
        {
            if (CAN_MSG_ID_IAP_DATA == ptBusObject->ptTransmitTable[ucMessageIndex].ulExtID)
            {
                /**can iap data message*/
                CAN_tTxPacket.tTxFrame.ExtID = can_get_msg_exd_id();
            }
            else
            {
                CAN_tTxPacket.tTxFrame.ExtID = ptBusObject->ptTransmitTable[ucMessageIndex].ulExtID;
            }

            CAN_tTxPacket.tTxFrame.Control_f.RTR = ptBusObject->ptTransmitTable[ucMessageIndex].ulRTR;

            if (CAN_tTxPacket.tTxFrame.ExtID)
            {
                CAN_tTxPacket.tTxFrame.Control_f.IDE = CAN_ID_EXT;
            }
            else
            {
                CAN_tTxPacket.tTxFrame.Control_f.IDE = CAN_ID_STD;

                if (0)//(SMOT_COBID_NMT == ptBusObject->ptTransmitTable[ucMessageIndex].ulMsgID)
                {
                    CAN_tTxPacket.tTxFrame.StdID = ptBusObject->ptTransmitTable[ucMessageIndex].ulMsgID;
                }
                else
                {
                    CAN_tTxPacket.tTxFrame.StdID = ptBusObject->ptTransmitTable[ucMessageIndex].ulMsgID | (uint32_t)(ptBusObject->ucSubsystem);
                }
            }

            CAN_tTxPacket.tTxFrame.Control_f.DLC = ptBusObject->ptTransmitTable[ucMessageIndex].ulDLC;

            CAN_tTxPacket.ucNodeID = ptBusObject->ucSubsystem;
            /**call service function */
            if (ptBusObject->ptTransmitTable[ucMessageIndex].pvServiceFunc != NULL)
            {
                ptBusObject->ptTransmitTable[ucMessageIndex].pvServiceFunc(&CAN_tTxPacket);
            }

            CAN_SetFrame(&CAN_tTxPacket.tTxFrame);

            /**check tranmist mailbox available*/

            if (CanTxBufFull != CAN_TransmitCmd(CanPTBTxCmd))
            {
                CAN_vSetBit(ptBusObject->pucMessageToTransmitBits, ucMessageIndex, 0); //Remove current message from transmission list.

                DEF_SET(CAN_tBusStatus, CAN_STATUS_SEND_DATA);  //!< set transmitting flag
                TMR_vSetTime(TMR_CAN_TX_TIMEOUT, TMR_TIME_MS2TICKS(4));    //!< max transmit time
            }
        }
        else
        {
            //Remove bit telling that there is a message due for transmission in this subsystem.
            CAN_uiTransmitFlagSubsystemIndex &= ~((uint16_t)(1 << CAN_ucSubsystemIndex));
        }
    }
}

void CAN_vTransmitMessage(uint8_t ucSubsystem, uint8_t ucMessageNumber)
{
    uint8_t ucSubsystemIndex;

    if ((CAN_NODE_BOARDS != ucSubsystem) || ((CAN_STATUS_STOPPED == CAN_tNodeStatus) && (ucMessageNumber < B2B_MSGIDX_SEND_NMT_STATUS)))
    {
        /**can in offline mode, start iap ...*/
        return;
    }

    ucSubsystemIndex = CAN_ucGetSubsystemIndex(ucSubsystem);

    if (ucSubsystemIndex != 0xFF)
    {
        if (ucMessageNumber < *CAN_tObjectTable[ucSubsystemIndex].pucNumberOfElementsInTransmitTable)
        {
            CAN_uiTransmitFlagSubsystemIndex |= (uint16_t)(1 << ucSubsystemIndex); //Mark that there is a message due for transmission in this subsystem.
            CAN_vSetBit(CAN_tObjectTable[ucSubsystemIndex].pucMessageToTransmitBits, ucMessageNumber, 1);
        }
    }
}

void CAN_vSetCanStatus(TCanStatusDef tStatus)
{
    if (tStatus < CAN_STATUS_NUM)
    {
        CAN_tNodeStatus = tStatus;
    }
}

TCanStatusDef CAN_tGetCanStatus(void)
{
    return CAN_tNodeStatus;
}


void CAN_vTaskHandler(void)
{
    switch (CAN_tTaskState)
    {
        case CAN_TASK_INIT:
        {
            CAN_tTaskState = CAN_TASK_RUN;

            break;
        }

        case CAN_TASK_RUN:
        {
            CAN_vCheckTransmitTask();

            if (CAN_ucRxPacketTailIndex != CAN_ucRxPacketHeadIndex)
            {
                CAN_vReceptionHandler(&CAN_tRxPacket[CAN_ucRxPacketTailIndex]);

                CAN_ucRxPacketTailIndex = (CAN_ucRxPacketTailIndex + 1) % CAN_RX_PACKET_NUM;
            }

            break;
        }

        default:break;
    }
}

void CAN_vSetBit(uint8_t* pucBitField, uint16_t uiBitId, uint8_t ucOnOff)
{
    uint8_t ucByte;
    uint8_t ucBitMask;

    ucByte = (uint8_t)(uiBitId / 8);
    pucBitField += ucByte;
    ucBitMask = 1 << (uiBitId % 8);

    if (ucOnOff)
    {
        *pucBitField |= ucBitMask;
    }
    else
    {
        *pucBitField &= ~ucBitMask;
    }
}

uint8_t CAN_ucGetBit(const uint8_t* pucBitField, uint16_t uiBitId)
{
    uint8_t ucByte;
    uint8_t ucBit;
    uint8_t ucBitMask;

    ucByte = (uint8_t)(uiBitId / 8);
    ucBit = uiBitId % 8;
    pucBitField += ucByte;
    ucBitMask = 1 << ucBit;

    return (*pucBitField & ucBitMask);
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
