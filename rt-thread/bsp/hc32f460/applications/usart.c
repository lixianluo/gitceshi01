#include "usart.h"


UART_HandleTypeDef uart3_handler;   

/*私有变量-------------------------------------------------*/



/*---------------------------------------------------------*/

/*私有函数-------------------------------------------------*/
static void UART3_vDmaInit(void);				//串口3DMA初始化
static void Usart3ErrIrqCallback(void);			//串口3错误中断回调函数
static void Usart3_vTxCpltCallback(void);		//串口3发送完成回调函数
static void Uart3TxDmaBtcIrqCallback(void);		//串口3DMA发送完成中断
/*---------------------------------------------------------*/

static void Usart3ErrIrqCallback(void)
{
	if (Set == USART_GetStatus(USART_3, UsartFrameErr))
	{
		USART_ClearStatus(USART_3, UsartFrameErr);
	}

	if (Set == USART_GetStatus(USART_3, UsartParityErr))
	{
		USART_ClearStatus(USART_3, UsartParityErr);
	}

	if (Set == USART_GetStatus(USART_3, UsartOverrunErr))
	{
		USART_ClearStatus(USART_3, UsartOverrunErr);
		(void)USART_RecData(USART_3);
	}
}

static void Usart3_vTxCpltCallback(void)
{
	USART_FuncCmd(USART_3, UsartTxCmpltInt, Disable);//失能串口3发送完成中断
	USART_ClearStatus(USART_3, UsartTxComplete);	   //清除串口3发送完成标志位
}

static void Uart3TxDmaBtcIrqCallback(void)
{
	DMA_ClearIrqFlag(DMA_UNIT_UART3_TX, DMA_CH_UART3_TX, BlkTrnCpltIrq);//清除DMA块完成标志位
	DMA_ClearIrqFlag(DMA_UNIT_UART3_TX, DMA_CH_UART3_TX, TrnCpltIrq);	//清除DMA发送完成标志位
	USART_FuncCmd(USART_3, UsartTxCmpltInt, Enable);					//使能串口3发送完成中断
}

void UART3_vInit(void)
{
	uint32_t u32Fcg1Periph = PWC_FCG1_PERIPH_USART3;
	const stc_usart_uart_init_t stcInitCfg = {
		UsartIntClkCkNoOutput,
		UsartClkDiv_1,
		UsartDataBits8,
		UsartDataLsbFirst,
		UsartOneStopBit,
		UsartParityNone,
		UsartSampleBit8,
		UsartStartBitFallEdge,
		UsartRtsEnable,
	};

	stc_irq_regi_conf_t stcIrqRegiCfg;

	UART3_vDmaInit();

	/**config uart handler*/
	uart3_handler.USARTx = USART_3;
	uart3_handler.ptTxDmaReg = DMA_UNIT_UART3_TX;
	uart3_handler.ucTxDmaCh = DMA_CH_UART3_TX;
	uart3_handler.ptRxDmaReg = DMA_UNIT_UART3_RX;
	uart3_handler.ucRxDmaCh = DMA_CH_UART3_RX;

	/* Enable peripheral clock */
	PWC_Fcg1PeriphClockCmd(u32Fcg1Periph, Enable);

	/* Initialize USART IO */
	PORT_SetFunc(USART_3_RX_PORT, USART_3_RX_PIN, Func_Usart3_Rx, Disable);
	PORT_SetFunc(USART_3_TX_PORT, USART_3_TX_PIN, Func_Usart3_Tx, Disable);

	/* Initialize USART */
	en_result_t enRet = USART_UART_Init(USART_3, &stcInitCfg);
	if (enRet != Ok)
	{
		while (1)
		{
		}
	}
	else
	{
	}

	/* Set baudrate */
	enRet = USART_SetBaudrate(USART_3, USART_3_BAUDRATE);
	if (enRet != Ok)
	{
		while (1)
		{
		}
	}
	else
	{
	}

	/* Set USART RX error IRQ 接收错误中断*/
	stcIrqRegiCfg.enIRQn = Int026_IRQn;
	stcIrqRegiCfg.pfnCallback = &Usart3ErrIrqCallback;
	stcIrqRegiCfg.enIntSrc = INT_USART2_EI;
	enIrqRegistration(&stcIrqRegiCfg);
	NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_05);
	NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
	NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

	/* Set USART RX IRQ 接收中断没用到屏蔽*/
	//stcIrqRegiCfg.enIRQn = Int000_IRQn;
	//stcIrqRegiCfg.pfnCallback = &UsartRxIrqCallback;
	//stcIrqRegiCfg.enIntSrc = INT_USART1_RI;
	//enIrqRegistration(&stcIrqRegiCfg);
	//NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_03);
	//NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
	//NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

	/* Set USART RX timeout error IRQ 接收超时中断*/
	/**Timer01_A as timeout clk source*/
	stcIrqRegiCfg.enIRQn = Int027_IRQn;
	stcIrqRegiCfg.pfnCallback = &Usart3RxIrqCallback;//接收超时中断回调函数(空闲中断 待写)
	stcIrqRegiCfg.enIntSrc = INT_USART3_RTO;
	enIrqRegistration(&stcIrqRegiCfg);
	NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_03);
	NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
	NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

	/* Set USART TX IRQ 发送中断没用到屏蔽 */
	//stcIrqRegiCfg.enIRQn = Int005_IRQn;
	//stcIrqRegiCfg.pfnCallback = &UsartTxIrqCallback;
	//stcIrqRegiCfg.enIntSrc = INT_USART1_TI;
	//enIrqRegistration(&stcIrqRegiCfg);
	//NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_03);
	//NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
	//NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

	/* Set USART TX complete IRQ 发送完成中断*/
	stcIrqRegiCfg.enIRQn = Int028_IRQn;
	stcIrqRegiCfg.pfnCallback = &Usart3_vTxCpltCallback;
	stcIrqRegiCfg.enIntSrc = INT_USART3_TCI;
	enIrqRegistration(&stcIrqRegiCfg);
	NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_03);
	NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
	NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

	/*Enable TX && RX && RX interrupt function*/
	USART_FuncCmd(USART_3, UsartTx, Enable);
	USART_FuncCmd(USART_3, UsartTxCmpltInt, Enable);
	USART_FuncCmd(USART_3, UsartRx, Enable);
	USART_FuncCmd(USART_3, UsartRxInt, Enable);
	USART_FuncCmd(USART_3, UsartTimeOut, Enable);
	USART_FuncCmd(USART_3, UsartTimeOutInt, Enable);

}
/*串口DMA只用DMA1,因为DMA2的CH0用在了ADC1上，CH1用在了ADC2上*/

static void UART3_vDmaInit(void)
{
	stc_irq_regi_conf_t stcIrqRegiCfg;
	stc_dma_config_t uart3_dma_tx_handler;
	stc_dma_config_t uart3_dma_rx_handler;

	/* Enable peripheral clock */
	PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_DMA1 ,Enable);

	/* Enable DMA. */
	DMA_Cmd(M4_DMA1, Enable);

	/* Initialize Uart1 Tx DMA. */
	MEM_ZERO_STRUCT(uart3_dma_tx_handler);
	uart3_dma_tx_handler.u16BlockSize = 1u;   /* 1 block */
	uart3_dma_tx_handler.u16TransferCnt = 1u;  /* Transfer count */
	uart3_dma_tx_handler.u32SrcAddr = ((uint32_t)(&(USART_3)->DR) + 2ul);  /* Set source address, this is init value!!!*/
	uart3_dma_tx_handler.u32DesAddr = (uint32_t)(&(USART_3)->DR);   /* Set destination address. */
	uart3_dma_tx_handler.stcDmaChCfg.enSrcInc = AddressIncrease;   /* Set source address mode. */
	uart3_dma_tx_handler.stcDmaChCfg.enDesInc = AddressFix;        /* Set destination address mode. */
	uart3_dma_tx_handler.stcDmaChCfg.enIntEn = Enable;             /* Enable interrupt. */
	uart3_dma_tx_handler.stcDmaChCfg.enTrnWidth = Dma8Bit;         /* Set data width 8bit. */
	DMA_InitChannel(DMA_UNIT_UART3_TX, DMA_CH_UART3_TX, &uart3_dma_tx_handler);

	/* 这里先不使能，等到DMA发送的时候才使能*/
	DMA_ChannelCmd(DMA_UNIT_UART3_TX, DMA_CH_UART3_TX, Disable);

	/* Initialize Uart1 Rx DMA. */
	MEM_ZERO_STRUCT(uart3_dma_rx_handler);
	uart3_dma_rx_handler.u16BlockSize = 1u;       /* 1 block */
	uart3_dma_rx_handler.u16TransferCnt = 8u;     /* Transfer count, init value */
	uart3_dma_rx_handler.u32SrcAddr = ((uint32_t)(&(USART_3)->DR) + 2ul);        /* Set source address. */
	uart3_dma_rx_handler.u32DesAddr = (uint32_t)(&(USART_3)->DR);    /* Set destination address. */
	uart3_dma_rx_handler.stcDmaChCfg.enSrcInc = AddressFix;           /* Set source address mode. */
	uart3_dma_rx_handler.stcDmaChCfg.enDesInc = AddressIncrease;      /* Set destination address mode. */
	uart3_dma_rx_handler.stcDmaChCfg.enIntEn = Enable;                /* Enable interrupt. */
	uart3_dma_rx_handler.stcDmaChCfg.enTrnWidth = Dma8Bit;            /* Set data width 8bit. */
	DMA_InitChannel(DMA_UNIT_UART3_RX, DMA_CH_UART3_RX, &uart3_dma_rx_handler);

	/* 这里先不使能，等到DMA接收的时候才使能*/
	DMA_ChannelCmd(DMA_UNIT_UART3_RX, DMA_CH_UART3_RX, Disable);

	/* Clear DMA flag. */
	DMA_ClearIrqFlag(DMA_UNIT_UART3_TX, DMA_CH_UART3_TX, TrnCpltIrq);
	DMA_ClearIrqFlag(DMA_UNIT_UART3_RX, DMA_CH_UART3_RX, TrnCpltIrq);

	/* Enable peripheral circuit trigger function. */
	PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

	/* Set DMA trigger source. */
	DMA_SetTriggerSrc(DMA_UNIT_UART3_TX, DMA_CH_UART3_TX, EVT_USART3_TI);
	DMA_SetTriggerSrc(DMA_UNIT_UART3_RX, DMA_CH_UART3_RX, EVT_USART3_RI);

	/* Set Uart Tx DMA block transfer complete IRQ DMA块发送完成中断*/
	stcIrqRegiCfg.enIRQn = Int025_IRQn;
	stcIrqRegiCfg.pfnCallback = &Uart3TxDmaBtcIrqCallback;
	stcIrqRegiCfg.enIntSrc = INT_DMA1_BTC2;
	enIrqRegistration(&stcIrqRegiCfg);
	NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_03);
	NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
	NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

	/* Set Uart Rx DMA block transfer complete IRQ DMA块接收完成中断，暂时没用到屏蔽*/
	/*stcIrqRegiCfg.enIRQn = Int025_IRQn;
	stcIrqRegiCfg.pfnCallback = &Uart2RxDmaBtcIrqCallback;
	stcIrqRegiCfg.enIntSrc = INT_DMA1_BTC2;
	enIrqRegistration(&stcIrqRegiCfg);
	NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_03);
	NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
	NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);*/
}




void UART_vTransmitDMA(UART_HandleTypeDef* ptUartHandler, TDmaBuffDef* ptDmaBuff)
{

#if 1
	DMA_SetTransferCnt(ptUartHandler->ptTxDmaReg, ptUartHandler->ucTxDmaCh, ptDmaBuff->uiDataLen - 1);
	DMA_SetSrcAddress(ptUartHandler->ptTxDmaReg, ptUartHandler->ucTxDmaCh, (uint32_t)&ptDmaBuff->ucData[0]);    //!< start from byte 1
	DMA_ChannelCmd(ptUartHandler->ptTxDmaReg, ptUartHandler->ucTxDmaCh, Enable);

	USART_SendData(ptUartHandler->USARTx, ptDmaBuff->ucData[0]);    //!< send byte 0 to trigger dma transmission
#else
	DMA_SetTransferCnt(ptUartHandler->ptTxDmaReg, ptUartHandler->ucTxDmaCh, ptDmaBuff->uiDataLen);
	DMA_SetSrcAddress(ptUartHandler->ptTxDmaReg, ptUartHandler->ucTxDmaCh, (uint32_t)ptDmaBuff->ucData);
	DMA_ChannelCmd(ptUartHandler->ptTxDmaReg, ptUartHandler->ucTxDmaCh, Enable);

	USART_FuncCmd(ptUartHandler->USARTx, UsartTxAndTxEmptyInt, Enable);

#endif

}

void UART_vReceiveDMA(UART_HandleTypeDef* ptUartHandler, TDmaBuffDef* ptDmaBuff, uint16_t uiDataLen)
{
	DMA_ChannelCmd(ptUartHandler->ptRxDmaReg, ptUartHandler->ucRxDmaCh, Disable);
	(void)memset(ptDmaBuff->ucData, 0, UART_DMA_BUFF_SIZE);
	ptDmaBuff->uiDataLen = uiDataLen;
	DMA_SetTransferCnt(ptUartHandler->ptRxDmaReg, ptUartHandler->ucRxDmaCh, ptDmaBuff->uiDataLen);
	DMA_SetDesAddress(ptUartHandler->ptRxDmaReg, ptUartHandler->ucRxDmaCh, (uint32_t)ptDmaBuff->ucData);
	DMA_ChannelCmd(ptUartHandler->ptRxDmaReg, ptUartHandler->ucRxDmaCh, Enable);
}

uint32_t UART_vGetDMACount(UART_HandleTypeDef* ptUartHandler)
{
	uint8_t ucDmaChannel = ptUartHandler->ucRxDmaCh;

	if (DmaCh0 == ucDmaChannel)
	{
		return ptUartHandler->ptRxDmaReg->MONDTCTL0_f.CNT;
	}
	else if (DmaCh1 == ucDmaChannel)
	{
		return ptUartHandler->ptRxDmaReg->MONDTCTL1_f.CNT;
	}
	else if (DmaCh2 == ucDmaChannel)
	{
		return ptUartHandler->ptRxDmaReg->MONDTCTL2_f.CNT;
	}
	else if (DmaCh3 == ucDmaChannel)
	{
		return ptUartHandler->ptRxDmaReg->MONDTCTL3_f.CNT;
	}
	else
	{
		return 0;   //!< should not reach here!
	}
}
