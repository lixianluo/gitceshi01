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

    PORT_Init(BLUE_PORT, BLUE_PIN, &stcPortInit);    //LED��ʼ��
    PORT_Init(RED_PORT, RED_PIN, &stcPortInit);         
    

    PORT_Init(POWEN_PORT, POWEN_PIN, &stcPortInit);     //��Դ������������

    PORT_Init(VALVE_PORT, VALVE_PIN, &stcPortInit);     //��ŷ���������

    PORT_Init(BRUSH_EN_PORT, BRUSH_EN_PIN, &stcPortInit);   //��ˢ���ʹ�ܿ���������
    
    PORT_Init(SUCTION_EN_PORT, SUCTION_EN_PIN, &stcPortInit);   //������ʹ�ܿ���������



   
     /*��������---------------------------------------------------*/  
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Disable;

    PORT_Init(SWITCH_B3_PORT, SWITCH_B3_PIN, &stcPortInit);             //��ת����B3��������

    PORT_Init(SWITCH_A2_PORT, SWITCH_A2_PIN, &stcPortInit);             //��ת����A2��������

    PORT_Init(SWITCH_A1_PORT, SWITCH_A1_PIN, &stcPortInit);             //��ת����A1��������

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

