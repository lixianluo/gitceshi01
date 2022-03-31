#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "app_motor.h"
#include "adc.h"
#include "time4_1.h"
#include "timea.h"
#include "app_key.h"
#include "gpio.h"
#include "stdlib.h"

static float vola_dec = 0;  //停止时刻的减速度(固定减速时刻1s)



static TspeedInfo Drive_Speed_tInfo = {
    .v_add = 4,
    .v_dec = 4,
    .v_rt_speed = 0,
    .v_next_speed = 0,
};
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

/*  行程电机速度函数    */
static void Drive_For_Speed_vTaskHandler(void);
static void Drive_Back_Speed_vTaskHandler(void);
static void Drive_Stop_Speed_vTaskHandler(void);

/*  盘刷电机速度函数    */
static void Brush_Work_Speed_vTaskHandler(void);
static void Brush_Work_Stop_vTaskHandler(void);
/*  吸风电机速度函数    */
static void Suction_Work_Speed_vTaskHandler(void);
static void Suction_Work_Stop_vTaskHandler(void);

/*接口参数函数*/
TspeedInfo* Drive_Motor_ptGetInfo(void);

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
    rt_thread_mdelay(100);
    while (1)
    {
        if (Key_ptGetInfo()->Brush_key_Flag == 1)
        {
            Brush_Work_Speed_vTaskHandler();                //盘刷电机工作
            if (Drive_Speed_tInfo.v_rt_speed != 0)          //行程电机当前速度！=0
            {
                PORT_SetBits(SOLENOID_PORT, SOLENOID_PIN);        //打开电磁阀
            }
            
        }
        if (Key_ptGetInfo()->Brush_key_Flag == 0)
        {
            Brush_Work_Stop_vTaskHandler();//盘刷电机停止
            PORT_ResetBits(SOLENOID_PORT, SOLENOID_PIN);          //关闭电磁阀
        }
        if (Key_ptGetInfo()->Suction_key_Flag == 1)
        {
            Suction_Work_Speed_vTaskHandler();//吸风电机工作
        }
        if (Key_ptGetInfo()->Suction_key_Flag == 0)
        {
            Suction_Work_Stop_vTaskHandler();//吸风电机停止
        }
        /*  把手开关按下 &&  前进按钮按下 &&后退按钮为0 && 当前速度 >=0*/
        if ((Key_ptGetInfo()->Drive_key_Flag == 1)&& (Key_ptGetInfo()->For_key_Flag == 1) && (Key_ptGetInfo()->Back_key_Flag == 0) && (Drive_Speed_tInfo.v_rt_speed >= 0))
        {   
            Drive_For_Speed_vTaskHandler(); //正转梯型加速
        }
        /*  把手开关按下 &&  前进按钮按下 && 后退按钮为0 && 当前速度 <0     正在前进突然切换方向*/
        if ((Key_ptGetInfo()->Drive_key_Flag == 1) && (Key_ptGetInfo()->For_key_Flag == 1) && (Key_ptGetInfo()->Back_key_Flag == 0) && (Drive_Speed_tInfo.v_rt_speed < 0))
        {
            Drive_Stop_Speed_vTaskHandler();//停止
        }
        /*  把手开关按下 &&  前进按钮为0 && 后退按钮按下 && 当前速度 <=0*/
        if ((Key_ptGetInfo()->Drive_key_Flag == 1) && (Key_ptGetInfo()->For_key_Flag == 0) && (Key_ptGetInfo()->Back_key_Flag == 1) && (Drive_Speed_tInfo.v_rt_speed <= 0))
        {
            Drive_Back_Speed_vTaskHandler();//反转
        }
        /*  把手开关按下 &&  前进按钮为0 && 后退按钮按下 && 当前速度 >0      正在后退突然切换方向*/
        if ((Key_ptGetInfo()->Drive_key_Flag == 1) && (Key_ptGetInfo()->For_key_Flag == 0) && (Key_ptGetInfo()->Back_key_Flag == 1) && (Drive_Speed_tInfo.v_rt_speed > 0))
        {
            Drive_Stop_Speed_vTaskHandler();//停止
        }
        /* 松开把手开关*/
        if (Key_ptGetInfo()->Drive_key_Flag == 0)
        {
            Drive_Stop_Speed_vTaskHandler();//停止
        }
        rt_thread_mdelay(10);
    }
}


static void Drive_For_Speed_vTaskHandler(void)
{
    if (Key_ptGetInfo()->Target_Speed > Drive_Speed_tInfo.v_rt_speed)                 //目标速度 > 当前速度
    {
        Drive_Speed_tInfo.v_next_speed = Drive_Speed_tInfo.v_rt_speed + Drive_Speed_tInfo.v_add;  //下一个时刻速度 = 当前速度+加速度

        if (Drive_Speed_tInfo.v_next_speed > Key_ptGetInfo()->Target_Speed)           //下一时刻速度 > 目标速度
        {
            Drive_Speed_tInfo.v_rt_speed = Key_ptGetInfo()->Target_Speed;             //当前速度 = 目标速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //当前速度赋值pwm
        }      
    }
    if (Key_ptGetInfo()->Target_Speed < Drive_Speed_tInfo.v_rt_speed)                 //目标速度 < 当前速度
    {
        Drive_Speed_tInfo.v_next_speed = Drive_Speed_tInfo.v_rt_speed - Drive_Speed_tInfo.v_dec;  //下一个时刻速度 = 当前速度-减速度

        if (Drive_Speed_tInfo.v_next_speed < Key_ptGetInfo()->Target_Speed)           //下一时刻速度 < 目标速度
        {
            Drive_Speed_tInfo.v_rt_speed = Key_ptGetInfo()->Target_Speed;             //当前速度 = 目标速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //当前速度赋值pwm
        }
    }
}


static void Drive_Back_Speed_vTaskHandler(void)
{
    if (Key_ptGetInfo()->Target_Speed < Drive_Speed_tInfo.v_rt_speed)                 //目标速度 < 当前速度
    {
        Drive_Speed_tInfo.v_next_speed = Drive_Speed_tInfo.v_rt_speed - Drive_Speed_tInfo.v_add;  //下一个时刻速度 = 当前速度-加速度

        if (Drive_Speed_tInfo.v_next_speed < Key_ptGetInfo()->Target_Speed)           //下一时刻速度 < 目标速度
        {
            Drive_Speed_tInfo.v_rt_speed = Key_ptGetInfo()->Target_Speed;             //当前速度 = 目标速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //当前速度赋值pwm
        }
    }
    if (Key_ptGetInfo()->Target_Speed > Drive_Speed_tInfo.v_rt_speed)                 //目标速度 > 当前速度
    {
        Drive_Speed_tInfo.v_next_speed = Drive_Speed_tInfo.v_rt_speed + Drive_Speed_tInfo.v_dec;  //下一个时刻速度 = 当前速度+减速度

        if (Drive_Speed_tInfo.v_next_speed > Key_ptGetInfo()->Target_Speed)           //下一时刻速度 > 目标速度
        {
            Drive_Speed_tInfo.v_rt_speed = Key_ptGetInfo()->Target_Speed;             //当前速度 = 目标速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //当前速度赋值pwm
        }
    }
}
static void Drive_Stop_Speed_vTaskHandler(void)
{
    vola_dec = abs((Key_ptGetInfo()->Moment_Speed / 100));
    if (Drive_Speed_tInfo.v_rt_speed > 0)                                            // 当前速度 > 0
    {
        Drive_Speed_tInfo.v_next_speed = (int16_t)((float)Drive_Speed_tInfo.v_rt_speed - vola_dec); //下一个时刻速度 = 当前速度 - 把手开关释放时速度/100(10ms处理一次 100次就是1s)

        if (Drive_Speed_tInfo.v_next_speed < 0)                                       //下一时刻速度 < 0
        {
            Drive_Speed_tInfo.v_rt_speed = 0;                                         //当前速度 = 0
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //当前速度赋值pwm
        }
    }
    if (Drive_Speed_tInfo.v_rt_speed < 0)                                            // 当前速度 < 0
    {
        Drive_Speed_tInfo.v_next_speed = (int16_t)((float)Drive_Speed_tInfo.v_rt_speed + vola_dec);   //下一个时刻速度 = 当前速度 + 把手开关释放时速度/100(10ms处理一次 100次就是1s)

        if (Drive_Speed_tInfo.v_next_speed > 0)                                       //下一时刻速度 < 0
        {
            Drive_Speed_tInfo.v_rt_speed = 0;                                         //当前速度 = 0
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //目标速度赋值pwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //否则，当前速度 = 下一时刻速度
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //当前速度赋值pwm
        }
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



TspeedInfo* Drive_Motor_ptGetInfo(void)
{
    return &Drive_Speed_tInfo;
}

/*
输入  当前速度V_start
输入  目标速度V_end
输入  加加速度jerk_acc
输入  加减速度jerk_dec
输入  加速度最大值acc_max
输入  减速度最大值dec_max

        
if()




*/
