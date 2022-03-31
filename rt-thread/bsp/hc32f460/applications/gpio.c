#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "gpio.h"
#include "time4_1.h"
#include "timea.h"


//static void BRUSH_CURRENT_IT_Init(void);
//static void SUCTION_CURRENT_IT_Init(void);
static void DRIVE_CURRENT_IT_Init(void);


static void DRIVE_CURRENT_IT_Callback(void);
//static void SUCTION_CURRENT_IT_Callback(void);
//static void BRUSH_CURRENT_IT_Callback(void);

void GPIO_vInit(void)
{
    stc_port_init_t         stcPortInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Enable;

    PORT_Init(LED1_PORT, LED1_PIN, &stcPortInit);    //LED初始化
    PORT_Init(LED2_PORT, LED2_PIN, &stcPortInit);


	PORT_Init(BRU_MOTOR_BRAKE_PORT, BRU_MOTOR_BRAKE_PIN, &stcPortInit);  //盘刷电机刹车信号初始化

    PORT_Init(SOLENOID_PORT, SOLENOID_PIN, &stcPortInit);       //电磁阀
    PORT_Init(RELAY_PORT, RELAY_PIN, &stcPortInit);             //继电器

    PORT_Init(PortA, Pin10, &stcPortInit);
    PORT_Init(PortB, Pin15, &stcPortInit);

    
    PORT_SetBits(BRU_MOTOR_BRAKE_PORT, BRU_MOTOR_BRAKE_PIN);  //使能刹车

    //IIC初始化
    MEM_ZERO_STRUCT(stcPortInit);

    stcPortInit.enPinMode = Pin_Mode_Out;
    stcPortInit.enExInt = Disable;
    stcPortInit.enPullUp = Enable;
    stcPortInit.enPinDrv = Pin_Drv_H;
    PORT_Init(IIC_SDA_PORT, IIC_SDA_PIN, &stcPortInit);     
    PORT_Init(IIC_SCL_PORT, IIC_SCL_PIN, &stcPortInit);
   //IIC初始化
   
   //输入IO初始化
    MEM_ZERO_STRUCT(stcPortInit);


    stcPortInit.enPinMode = Pin_Mode_In;
    stcPortInit.enExInt = Enable;
    stcPortInit.enPullUp = Disable;




    PORT_Init(DRIVE_MOTOR_START_PORT, DRIVE_MOTOR_START_PIN, &stcPortInit);     //把手开关（驱动电机启动信号）
    
    PORT_Init(SUCTION_MOTOR_START_PORT, SUCTION_MOTOR_START_PIN, &stcPortInit); //刮条限位（吸风电机启动信号）

    PORT_Init(FOR_BACK_PORT, FOR_BACK_PIN, &stcPortInit);                       //前进后退按钮（正反转）
    
    PORT_Init(BRU_MOTOR_START_PORT, BRU_MOTOR_START_PIN, &stcPortInit);                     //盘刷限位


//  BRUSH_CURRENT_IT_Init();
//  SUCTION_CURRENT_IT_Init();
//    DRIVE_CURRENT_IT_Init();
}

//void BRUSH_CURRENT_IT_Init(void)
//{
//    stc_exint_config_t stcExtiConfig;
//    stc_irq_regi_conf_t stcIrqRegiConf;
//    stc_port_init_t stcPortInit;

//    /* configuration structure initialization */
//    MEM_ZERO_STRUCT(stcExtiConfig);
//    MEM_ZERO_STRUCT(stcIrqRegiConf);
//    MEM_ZERO_STRUCT(stcPortInit);


//    stcExtiConfig.enExitCh = ExtiCh02;
//    stcExtiConfig.enFilterEn = Enable;
//    stcExtiConfig.enFltClk = Pclk3Div8;
//    stcExtiConfig.enExtiLvl = ExIntLowLevel;
//    EXINT_Init(&stcExtiConfig);


//    stcPortInit.enExInt = Enable;
//    PORT_Init(BRUSH_CURRENT_IT_PORT, BRUSH_CURRENT_IT_PIN, &stcPortInit);

//    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ2;
//    stcIrqRegiConf.enIRQn = Int017_IRQn;
//    stcIrqRegiConf.pfnCallback = &BRUSH_CURRENT_IT_Callback;
//    enIrqRegistration(&stcIrqRegiConf);
//    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
//    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
//    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
//}

//static void SUCTION_CURRENT_IT_Init(void)
//{
//    stc_exint_config_t stcExtiConfig;
//    stc_irq_regi_conf_t stcIrqRegiConf;
//    stc_port_init_t stcPortInit;

//    /* configuration structure initialization */
//    MEM_ZERO_STRUCT(stcExtiConfig);
//    MEM_ZERO_STRUCT(stcIrqRegiConf);
//    MEM_ZERO_STRUCT(stcPortInit);


//    stcExtiConfig.enExitCh = ExtiCh09;
//    stcExtiConfig.enFilterEn = Enable;
//    stcExtiConfig.enFltClk = Pclk3Div8;
//    stcExtiConfig.enExtiLvl = ExIntLowLevel;
//    EXINT_Init(&stcExtiConfig);


//    stcPortInit.enExInt = Enable;
//    PORT_Init(SUCTION_CURRENT_IT_PORT, SUCTION_CURRENT_IT_PIN, &stcPortInit);

//    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ9;
//    stcIrqRegiConf.enIRQn = Int018_IRQn;
//    stcIrqRegiConf.pfnCallback = &SUCTION_CURRENT_IT_Callback;
//    enIrqRegistration(&stcIrqRegiConf);
//    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
//    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
//    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
//}


static void DRIVE_CURRENT_IT_Init(void)
{
    stc_exint_config_t stcExtiConfig;
    stc_irq_regi_conf_t stcIrqRegiConf;
    stc_port_init_t stcPortInit;

    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcExtiConfig);
    MEM_ZERO_STRUCT(stcIrqRegiConf);
    MEM_ZERO_STRUCT(stcPortInit);


    stcExtiConfig.enExitCh = ExtiCh12;
    stcExtiConfig.enFilterEn = Enable;
    stcExtiConfig.enFltClk = Pclk3Div8;
    stcExtiConfig.enExtiLvl = ExIntLowLevel;
    EXINT_Init(&stcExtiConfig);


    stcPortInit.enExInt = Enable;
    PORT_Init(DRIVE_CURRENT_IT_PORT, DRIVE_CURRENT_IT_PIN, &stcPortInit);

    stcIrqRegiConf.enIntSrc = INT_PORT_EIRQ12;
    stcIrqRegiConf.enIRQn = Int019_IRQn;
    stcIrqRegiConf.pfnCallback = &DRIVE_CURRENT_IT_Callback;
    enIrqRegistration(&stcIrqRegiConf);
    NVIC_ClearPendingIRQ(stcIrqRegiConf.enIRQn);
    NVIC_SetPriority(stcIrqRegiConf.enIRQn, DDL_IRQ_PRIORITY_15);
    NVIC_EnableIRQ(stcIrqRegiConf.enIRQn);
}

static void DRIVE_CURRENT_IT_Callback(void) //驱动电机过流中断回调函数
{
    if (Set == EXINT_IrqFlgGet(ExtiCh12))
    {
        //TIMER4_CNT_Stop(TIMER4_1_UNIT); //关闭就无法调PWM了
        Time4_1_Drive_Motor_Contorl(0);
        
        /* clear int request flag */
        EXINT_IrqFlgClr(ExtiCh12);
    }
}


//static void SUCTION_CURRENT_IT_Callback(void)//吸风电机过流中断回调函数
//{
//    if (Set == EXINT_IrqFlgGet(ExtiCh04))
//    {
//        Timera_Motor_Contorl(Suction_Motor, Disable, 0);
//        /* clear int request flag */
//        EXINT_IrqFlgClr(ExtiCh04);
//    }
//}


//static void BRUSH_CURRENT_IT_Callback(void)//盘刷电机过流中断回调函数
//{
//    if (Set == EXINT_IrqFlgGet(ExtiCh02))
//    {
//        Timera_Motor_Contorl(Brush_Motor, Disable, 0);
//        /* clear int request flag */
//        EXINT_IrqFlgClr(ExtiCh02);
//    }

//}

