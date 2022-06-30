#include "AlgorithmBoard.h"
#include "sw_timer.h"
//#include "hc32f460keta.h"
#include <string.h>
#include <stdio.h>
#include "hc32f46x_timer0.h"
#include "gpio.h"
#include "ymodem_ota.h"


#define ALGO_RX_BUFFER_SIZE	1040
static TDmaBuffDef ALGO_ucRxBuffer;

void Usart2RxIrqCallback(void)
{
	TIMER0_Cmd(M4_TMR01, Tim0_ChannelB, Disable);
	USART_ClearStatus(USART_2, UsartRxTimeOut);
	DMA_ChannelCmd(DMA_UNIT_UART2_RX, DMA_CH_UART2_RX, Disable);

	/**read dma data*/
	uint32_t ucRxByteCount = ALGO_RX_BUFFER_SIZE - UART_vGetDMACount(&uart2_handler);
	ymodem_ota_set_received_data(ucRxByteCount, ALGO_ucRxBuffer.ucData);

	/**clear buffer, enable DMA reception again*/
	(void)memset((void*)ALGO_ucRxBuffer.ucData, 0x00, UART_DMA_BUFF_SIZE);
	UART_vReceiveDMA(&uart2_handler, &ALGO_ucRxBuffer, ALGO_RX_BUFFER_SIZE);
}

void ALGO_vTxCpltCallback(void)
{
	USART_FuncCmd(USART_2, UsartTxCmpltInt, Disable);
	DMA_ChannelCmd(DMA_UNIT_UART2_TX, DMA_CH_UART2_TX, Disable);	//!< ToDo: enable it for test later
	USART_ClearStatus(USART_2, UsartTxComplete);
}


void ALGO_vReceptionInit(void)
{
	UART_vReceiveDMA(&uart2_handler, &ALGO_ucRxBuffer, ALGO_RX_BUFFER_SIZE);
}