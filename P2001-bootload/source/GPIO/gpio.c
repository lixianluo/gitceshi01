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

    
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Disable;

    PORT_Init(SWITCH_B3_PORT, SWITCH_B3_PIN, &stcPortInit);             //��ת����B3��������

    PORT_Init(SWITCH_A2_PORT, SWITCH_A2_PIN, &stcPortInit);             //��ת����A2��������

    PORT_Init(SWITCH_A1_PORT, SWITCH_A1_PIN, &stcPortInit);             //��ת����A1��������

    PORT_Init(HAND_PORT_1, HAND_PIN_1, &stcPortInit);                   //���ֿ�����������
    PORT_Init(HAND_PORT_2, HAND_PIN_2, &stcPortInit);                   //���ֿ�����������
}
