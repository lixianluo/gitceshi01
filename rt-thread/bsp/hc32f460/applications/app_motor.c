#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "app_motor.h"
#include "adc.h"
#include "timea.h"
#include "app_key.h"
#include "gpio.h"
#include "stdlib.h"






static TspeedInfo Brush_Speed_tInfo = {
    .v_add = 1,
    .v_dec = 1,
    .v_rt_speed = 0,
    .v_next_speed = 0,
};
static TspeedInfo Suction_Speed_tInfo = {
    .v_add = 1,
    .v_dec = 1,
    .v_rt_speed = 0,
    .v_next_speed = 0,
};


static void MOTOR_vTaskHandler_Entry(void* parameter);


/*  盘刷电机速度函数    */
static void Brush_Work_Speed_vTaskHandler(void);
static void Brush_Work_Stop_vTaskHandler(void);
/*  吸风电机速度函数    */
static void Suction_Work_Speed_vTaskHandler(void);
static void Suction_Work_Stop_vTaskHandler(void);


int MOTOR_iTaskHandler(void)
{
    rt_thread_t tid1;
    /* 创建MOTOR_vTaskHandler_entry线程 */
    tid1 = rt_thread_create("MOTOR_vTaskHandler_entry",
        MOTOR_vTaskHandler_Entry,
        RT_NULL,
        1024,
        5,
        20);
    /* 创建成功则启动线程 */
    if (tid1 != RT_NULL)rt_thread_startup(tid1);

    return 0;
}
INIT_APP_EXPORT(MOTOR_iTaskHandler);

static void MOTOR_vTaskHandler_Entry(void* parameter)
{ 
    uint8_t Suction_key_Flag, Brush_key_Flag, All_key_Flag, Off_key_Flag, Hand_FLag;
    rt_thread_mdelay(100);
    while (1)
    {
        Suction_key_Flag = Key_ptGetInfo()->Suction_key_Flag;
        Brush_key_Flag = Key_ptGetInfo()->Brush_key_Flag;
        All_key_Flag = Key_ptGetInfo()->All_key_Flag;
        Off_key_Flag = Key_ptGetInfo()->Off_key_Flag;
        Hand_FLag = Key_ptGetInfo()->Hand_FLag;
        /*盘刷电机*/
        if ((Hand_FLag == 1) && ((Brush_key_Flag == 1)||(All_key_Flag == 1)))
        {
            PORT_SetBits(VALVE_PORT, VALVE_PIN);
            Brush_Work_Speed_vTaskHandler();
        }
        else
        {
            PORT_ResetBits(VALVE_PORT, VALVE_PIN);
            Brush_Work_Stop_vTaskHandler();
        }
        /*吸风电机*/
        if ((Suction_key_Flag == 1)||(All_key_Flag == 1))
        {
            Suction_Work_Speed_vTaskHandler();
        }
        else
        {
            Suction_Work_Stop_vTaskHandler();
        }
        rt_thread_mdelay(10);
    }
}








static void Brush_Work_Speed_vTaskHandler(void)
{
    
    if (200 > Brush_Speed_tInfo.v_rt_speed)                 //目标速度 > 当前速度
    {
        Brush_Speed_tInfo.v_next_speed = Brush_Speed_tInfo.v_rt_speed + Brush_Speed_tInfo.v_add;  //下一个时刻速度 = 当前速度+加速度

        if (Brush_Speed_tInfo.v_next_speed > 200)           //下一时刻速度 > 目标速度
        {
            Brush_Speed_tInfo.v_rt_speed = 200;             //当前速度 = 目标速度
            Timera_Motor_Contorl(Brush_Motor, Enable, Brush_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Brush_Speed_tInfo.v_rt_speed = Brush_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Timera_Motor_Contorl(Brush_Motor, Enable, Brush_Speed_tInfo.v_rt_speed);               //当前速度赋值pwm
        }
    }
}
static void Brush_Work_Stop_vTaskHandler(void)
{

    if (Brush_Speed_tInfo.v_rt_speed > 0)                 //当前速度 > 0
    {
        Brush_Speed_tInfo.v_next_speed = Brush_Speed_tInfo.v_rt_speed - Brush_Speed_tInfo.v_dec;  //下一个时刻速度 = 当前速度-减速度

        if (Brush_Speed_tInfo.v_next_speed < 0)           //下一时刻速度 < 0
        {
            Brush_Speed_tInfo.v_rt_speed = 0;             //当前速度 = 目标速度
            Timera_Motor_Contorl(Brush_Motor, Enable, Brush_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Brush_Speed_tInfo.v_rt_speed = Brush_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Timera_Motor_Contorl(Brush_Motor, Enable, Brush_Speed_tInfo.v_rt_speed);               //当前速度赋值pwm
        }
    }
}



static void Suction_Work_Speed_vTaskHandler(void)
{
    if (200 > Suction_Speed_tInfo.v_rt_speed)                 //目标速度 > 当前速度
    {
        Suction_Speed_tInfo.v_next_speed = Suction_Speed_tInfo.v_rt_speed + Suction_Speed_tInfo.v_add;  //下一个时刻速度 = 当前速度+加速度

        if (Suction_Speed_tInfo.v_next_speed > 200)           //下一时刻速度 > 目标速度
        {
            Suction_Speed_tInfo.v_rt_speed = 200;             //当前速度 = 目标速度
            Timera_Motor_Contorl(Suction_Motor, Enable, Suction_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Suction_Speed_tInfo.v_rt_speed = Suction_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Timera_Motor_Contorl(Suction_Motor, Enable, Suction_Speed_tInfo.v_rt_speed);               //当前速度赋值pwm
        }
    }

}
static void Suction_Work_Stop_vTaskHandler(void)
{
    if (Suction_Speed_tInfo.v_rt_speed > 0)                 //当前速度 > 0
    {
        Suction_Speed_tInfo.v_next_speed = Suction_Speed_tInfo.v_rt_speed - Suction_Speed_tInfo.v_dec;  //下一个时刻速度 = 当前速度-减速度

        if (Suction_Speed_tInfo.v_next_speed < 0)           //下一时刻速度 < 0
        {
            Suction_Speed_tInfo.v_rt_speed = 0;             //当前速度 = 目标速度
            Timera_Motor_Contorl(Suction_Motor, Enable, Suction_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Suction_Speed_tInfo.v_rt_speed = Suction_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Timera_Motor_Contorl(Suction_Motor, Enable, Suction_Speed_tInfo.v_rt_speed);               //当前速度赋值pwm
        }
    }

}


