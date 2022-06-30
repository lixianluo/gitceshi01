#ifndef _UART_H
#define _UART_H

#include <stdint.h>
#include "hc32f46x_usart.h"
#include "hc32f46x_gpio.h"
//#include "hc32f460keta.h"
#include "hc32f46x_dmac.h"

/**UART2 -----------------------------------------*/
#define USART_2                         (M4_USART2)
#define USART_2_BAUDRATE                (115200ul)
#define USART_2_RX_PORT                 (PortA)
#define USART_2_RX_PIN                  (Pin04)

#define USART_2_TX_PORT                 (PortA)
#define USART_2_TX_PIN                  (Pin05)

/*DMA---------------------------------------------*/
/*串口DMA只用DMA1,因为DMA2的CH0用在了ADC1上，CH1用在了ADC2上*/
#define DMA_UNIT_UART2_TX               (M4_DMA1)
#define DMA_CH_UART2_TX                 (DmaCh2)

#define DMA_UNIT_UART2_RX               (M4_DMA1)
#define DMA_CH_UART2_RX                 (DmaCh3)
/*-------------------------------------------------*/

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
 /**
  *******************************************************************************
  ** \brief buffer handle
  **
  ******************************************************************************/
#define UART_DMA_BUFF_SIZE       1040
typedef struct _TDmaBuffDef
{
    uint16_t uiDataLen;
    uint8_t ucData[UART_DMA_BUFF_SIZE];
} TDmaBuffDef;


typedef struct _UART_HandleTypeDef {
    M4_USART_TypeDef* USARTx;
    M4_DMA_TypeDef* ptTxDmaReg;     //!< dma group
    M4_DMA_TypeDef* ptRxDmaReg;
    uint8_t ucTxDmaCh;              //!< dma channel
    uint8_t ucRxDmaCh;
}UART_HandleTypeDef;

extern void Usart2RxIrqCallback(void);

extern UART_HandleTypeDef uart2_handler;


extern void UART2_vInit(void);

extern void UART_vTransmitDMA(UART_HandleTypeDef* ptUartHandler, TDmaBuffDef* ptDmaBuff);
extern void UART_vReceiveDMA(UART_HandleTypeDef* ptUartHandler, TDmaBuffDef* ptDmaBuff, uint16_t uiDataLen);




extern uint32_t UART_vGetDMACount(UART_HandleTypeDef* ptUartHandler);















#endif

