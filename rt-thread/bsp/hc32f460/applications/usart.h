#ifndef _USART_H_
#define _USART_H_
#include <stdint.h>
#include "hc32f460_usart.h"
#include "hc32f460_gpio.h"
#include "hc32f460_dmac.h"
#include "board.h"

#define UART_DMA_BUFF_SIZE      80


/* USART channel definition */
/**UART1 -----------------------------------------*/
#define USART_3                (M4_USART3)
#define USART_3_BAUDRATE       (115200ul)
#define USART_3_RX_PORT        (PortB)
#define USART_3_RX_PIN         (Pin05)

#define USART_3_TX_PORT        (PortB)
#define USART_3_TX_PIN         (Pin06)
/*------------------------------------------------*/

/*DMA---------------------------------------------*/
/*串口DMA只用DMA1,因为DMA2的CH0用在了ADC1上，CH1用在了ADC2上*/
#define DMA_UNIT_UART3_TX               (M4_DMA1)
#define DMA_CH_UART3_TX                 (DmaCh0)

#define DMA_UNIT_UART3_RX               (M4_DMA1)
#define DMA_CH_UART3_RX                 (DmaCh1)
/*-------------------------------------------------*/

typedef struct _TDmaBuffDef
{
    uint16_t uiDataLen;
    uint8_t ucData[UART_DMA_BUFF_SIZE];
}TDmaBuffDef;



typedef struct _UART_HandleTypeDef {
    M4_USART_TypeDef* USARTx;
    M4_DMA_TypeDef* ptTxDmaReg;     //!< dma group
    M4_DMA_TypeDef* ptRxDmaReg;
    uint8_t ucTxDmaCh;              //!< dma channel
    uint8_t ucRxDmaCh;
}UART_HandleTypeDef;



/*对外函数---------------------------------------------*/
uint32_t UART_vGetDMACount(UART_HandleTypeDef* ptUartHandler);
extern void UART_vTransmitDMA(UART_HandleTypeDef* ptUartHandler, TDmaBuffDef* ptDmaBuff);
extern void UART_vReceiveDMA(UART_HandleTypeDef* ptUartHandler, TDmaBuffDef* ptDmaBuff, uint16_t uiDataLen);
extern void UART3_vInit(void);
/*------------------------------------------------------*/

#endif // !_USART_H_

