#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "gpio.h"
#include "timea.h"





void GPIO_vInit(void)
{
    stc_port_init_t         stcPortInit;

   /*输出配置-------------------------------------------------*/
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Enable;

    PORT_Init(BLUE_PORT, BLUE_PIN, &stcPortInit);    //LED初始化
    PORT_Init(RED_PORT, RED_PIN, &stcPortInit);         
    

    PORT_Init(POWEN_PORT, POWEN_PIN, &stcPortInit);     //电源保持引脚配置

    PORT_Init(VALVE_PORT, VALVE_PIN, &stcPortInit);     //电磁阀引脚配置

    PORT_Init(BRUSH_EN_PORT, BRUSH_EN_PIN, &stcPortInit);   //盘刷电机使能口引脚配置
    
    PORT_Init(SUCTION_EN_PORT, SUCTION_EN_PIN, &stcPortInit);   //吸风电机使能口引脚配置



   
     /*输入配置---------------------------------------------------*/  
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Disable;

    PORT_Init(SWITCH_B3_PORT, SWITCH_B3_PIN, &stcPortInit);             //旋转开关B3引脚配置

    PORT_Init(SWITCH_A2_PORT, SWITCH_A2_PIN, &stcPortInit);             //旋转开关A2引脚配置

    PORT_Init(SWITCH_A1_PORT, SWITCH_A1_PIN, &stcPortInit);             //旋转开关A1引脚配置

    PORT_Init(HAND_PORT_1, HAND_PIN_1, &stcPortInit);                       //把手开关引脚配置
    PORT_Init(HAND_PORT_2, HAND_PIN_2, &stcPortInit);                       //把手开关引脚配置

    /*IIC初始化---------------------------------------------------*/
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Enable;
    stcPortInit.enPinDrv = Pin_Drv_H;
    PORT_Init(IIC_SDA_PORT, IIC_SDA_PIN, &stcPortInit);     
    PORT_Init(IIC_SCL_PORT, IIC_SCL_PIN, &stcPortInit);

 }

