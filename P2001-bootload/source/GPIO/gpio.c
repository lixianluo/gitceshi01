#include "gpio.h"


void GPIO_vInit(void)
{
    stc_port_init_t stcPortInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Enable;

    /* LED0 Port/Pin initialization */
    PORT_Init(LED0_PORT, LED0_PIN, &stcPortInit);
    /* LED1 Port/Pin initialization */
    PORT_Init(LED1_PORT, LED1_PIN, &stcPortInit);

    
    /**RS485 DE pin*/
    //PORT_Init(RS485_1_DE_PORT, RS485_1_DE_PIN, &stcPortInit);
    //PORT_Init(RS485_2_DE_PORT, RS485_2_DE_PIN, &stcPortInit);

    //PORT_ResetBits(RS485_1_DE_PORT, RS485_1_DE_PIN);
    //PORT_ResetBits(RS485_2_DE_PORT, RS485_2_DE_PIN);

    /**Camera sync pin*/
    //PORT_Init(CAMERA_SYNC_PORT, CAMERA_SYNC_PIN, &stcPortInit);
    //PORT_ResetBits(CAMERA_SYNC_PORT, CAMERA_SYNC_PIN);
}
