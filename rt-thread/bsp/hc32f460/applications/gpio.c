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

    PORT_Init(LED1_PORT, LED1_PIN, &stcPortInit);    //LED初始化
    PORT_Init(LED2_PORT, LED2_PIN, &stcPortInit);

  
    PORT_Init(POWEN_PORT, POWEN_PIN, &stcPortInit);     //电源保持引脚配置

    PORT_Init(VALVE_PORT, VALVE_PIN, &stcPortInit);     //电磁阀引脚配置

    PORT_Init(PWM_EN_PORT, PWM_EN_PIN, &stcPortInit);   //大功率使能口引脚配置



   
     /*输入配置---------------------------------------------------*/  
    MEM_ZERO_STRUCT(stcPortInit);
    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Disable;

    PORT_Init(BRUSH_START_PORT, BRUSH_START_PIN, &stcPortInit);         //盘刷启动引脚配置

    PORT_Init(SUCTION_START_PORT, SUCTION_START_PIN, &stcPortInit);     //吸风启动引脚配置

    PORT_Init(ALL_START_PORT, ALL_START_PIN, &stcPortInit);             //盘刷 + 吸风启动引脚配置

    PORT_Init(HAND_PORT, HAND_PIN, &stcPortInit);                       //把手开关引脚配置

    /*IIC初始化---------------------------------------------------*/
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Enable;
    stcPortInit.enPinDrv = Pin_Drv_H;
    PORT_Init(IIC_SDA_PORT, IIC_SDA_PIN, &stcPortInit);     
    PORT_Init(IIC_SCL_PORT, IIC_SCL_PIN, &stcPortInit);

 }

