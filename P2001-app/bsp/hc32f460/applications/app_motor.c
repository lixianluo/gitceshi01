#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "app_motor.h"
#include "adc.h"
#include "timea.h"
#include "app_key.h"
#include "gpio.h"
#include "stdlib.h"


#include "main.h"


/*私有变量-------------------------------------------------------*/
static uint16_t num;
static uint16_t indexWave[] = {
    1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
    8, 9, 9, 9,10,10,10,11,11,11,12,12,13,13,14,14,15,15,16,16,
    17,17,18,18,19,19,20,20,21,21,22,22,23,23,24,24,25,25,26,26,
    27,27,28,28,29,29,30,30,31,31,32,32,33,33,34,34,35,35,36,36,
    37,37,38,38,39,39,40,40,41,42,43,43,44,44,45,46,47,48,49,49,
    50,50,51,52,53,54,55,56,57,57,58,58,59,60,61,62,63,64,65,66,
    67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,
    87,88,90,91,92,94,95,97,98,100,101,103,104,106,107,108,109,111,112,114,116,
    118,119,121,123,124,126,128,130,132,134,136,138,140,146,148,150,152,154,156,158,
    162,163,164,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,199,200,200,
    199,196,194,192,190,188,186,184,182,180,178,176,174,172,170,168,164,163,162,158,156,
    154,152,150,148,146,140,138,136,134,132,130,128,126,124,123,121,119,118,116,114,112,
    111,109,108,107,106,104,103,101,100,98,97,95,94,92,91,90,88,87,86,85,84,83,82,81,80,
    79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,58,57,57,56,55,54,
    53,52,51,50,50,49,49,48,47,46,45,44,44,43,43,42,41,40,40,39,39,38,38,37,37,36,36,35,
    35,34,34,33,33,32,32,31,31,30,30,29,29,28,28,27,27,26,26,25,25,24,24,23,23,22,22,21,
    20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,11,10,10,10,9,9,9,8,8,8,
    7,7,6,6,5,5,4,4,4,3,3,3,2,2,2,1,1,1,
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
static  rt_timer_t RGB_timer;
/*---------------------------------------------------------------*/
/*私有函数-------------------------------------------------------*/
static void MOTOR_vTaskHandler_Entry(void* parameter);
static void RGB_timer_timeout(void* parameter);
/*  盘刷电机速度函数    */
static void Brush_Work_Speed_vTaskHandler(void);
static void Brush_Work_Stop_vTaskHandler(void);
/*  吸风电机速度函数    */
static void Suction_Work_Speed_vTaskHandler(void);
static void Suction_Work_Stop_vTaskHandler(void);
/*---------------------------------------------------------------*/

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
    /*10ms定时器*/
    RGB_timer = rt_timer_create("RGB_time", RGB_timer_timeout,
        RT_NULL, 1,
        RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    return 0;
}
INIT_APP_EXPORT(MOTOR_iTaskHandler);

static void MOTOR_vTaskHandler_Entry(void* parameter)
{ 
    uint8_t Suction_key_Flag, Brush_key_Flag, All_key_Flag, Off_key_Flag, Hand_FLag;
    rt_thread_mdelay(100);
    while (1)
    {
        Suction_key_Flag = Key_ptGetInfo()->Suction_Key_Flag;
        Brush_key_Flag = Key_ptGetInfo()->Brush_Key_Flag;
        All_key_Flag = Key_ptGetInfo()->All_Key_Flag;
        Off_key_Flag = Key_ptGetInfo()->Off_Key_Flag;
        Hand_FLag = Key_ptGetInfo()->Hand_Key_FLag;
        /*盘刷电机*/
        if ((Hand_FLag == 1) && ((Brush_key_Flag == 1)||(All_key_Flag == 1)))
        {
            VALVE_ENABLE();
            rt_timer_stop(RGB_timer);
            Timera_RGB_Contorl(Enable, 200);
            Brush_Work_Speed_vTaskHandler();
        }
		else
        {
			rt_timer_start(RGB_timer);
            VALVE_DISABLE();
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

static void RGB_timer_timeout(void* parameter)
{
    Timera_RGB_Contorl(Enable, indexWave[num]);
    num++;
    if (num == (sizeof(indexWave) / sizeof(indexWave[0])-1)) 
    { 
        num = 0; 
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
            Timera_Motor_Contorl(Suction_Motor, Enable, Suction_Speed_tInfo.v_rt_speed);//目标速度赋值pwm
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


TspeedInfo* ptApp_Motor_Branch(void)
{
    return &Brush_Speed_tInfo;
}

TspeedInfo* ptApp_Motor_Suction(void)
{
    return &Suction_Speed_tInfo;
}