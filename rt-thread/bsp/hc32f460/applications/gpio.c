#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "gpio.h"
#include "timea.h"





void GPIO_vInit(void)
{
    stc_port_init_t         stcPortInit;

   /*�������-------------------------------------------------*/
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Enable;

    PORT_Init(LED1_PORT, LED1_PIN, &stcPortInit);    //LED��ʼ��
    PORT_Init(LED2_PORT, LED2_PIN, &stcPortInit);

  
    PORT_Init(POWEN_PORT, POWEN_PIN, &stcPortInit);     //��Դ������������

    PORT_Init(VALVE_PORT, VALVE_PIN, &stcPortInit);     //��ŷ���������

    PORT_Init(PWM_EN_PORT, PWM_EN_PIN, &stcPortInit);   //����ʹ�ܿ���������



   
     /*��������---------------------------------------------------*/  
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Disable;

    PORT_Init(BRUSH_START_PORT, BRUSH_START_PIN, &stcPortInit);         //��ˢ������������

    PORT_Init(SUCTION_START_PORT, SUCTION_START_PIN, &stcPortInit);     //����������������

    PORT_Init(ALL_START_PORT, ALL_START_PIN, &stcPortInit);             //��ˢ + ����������������

    PORT_Init(HAND_PORT, HAND_PIN, &stcPortInit);                       //���ֿ�����������

    /*IIC��ʼ��---------------------------------------------------*/
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Enable;
    stcPortInit.enPinDrv = Pin_Drv_H;
    PORT_Init(IIC_SDA_PORT, IIC_SDA_PIN, &stcPortInit);     
    PORT_Init(IIC_SCL_PORT, IIC_SCL_PIN, &stcPortInit);

 }

