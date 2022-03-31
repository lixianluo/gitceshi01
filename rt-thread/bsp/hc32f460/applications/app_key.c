#include "app_key.h"
#include "gpio.h"
#include "board.h"
#include "adc.h"
#include "app.display.h"
#include "app_motor.h"


static void Key_vTaskHandler_entry(void* parameter);
TKeyInfoDef* Key_ptGetInfo(void);


static en_flag_status_t  Drive_key = Set;
static en_flag_status_t  Suction_key = Set;
static en_flag_status_t  For_Back_key = Set;
static en_flag_status_t  Brush_key = Set;
static TKeyInfoDef Key_tInfo;
static uint8_t Drive_key_Count = 0;
static uint8_t Suction_key_Count = 0;
static uint8_t For_key_Count = 0;
static uint8_t Back_key_Count = 0;
static uint8_t Brush_key_Count = 0;

static uint8_t Drive_key_Flag = 0;

int Key_iTaskHandler(void)
{
    rt_thread_t tid1;
    /* 创建Key_vTaskHandler_entry线程 */
    tid1 = rt_thread_create("Key_vTaskHandler_entry",
        Key_vTaskHandler_entry,
        RT_NULL,
        1024,
        6,
        20);
    /* 创建成功则启动线程 */
    if (tid1 != RT_NULL)rt_thread_startup(tid1);

    

    return 0;
}
INIT_APP_EXPORT(Key_iTaskHandler);


static void Key_vTaskHandler_entry(void* parameter)
{
    rt_thread_mdelay(50);
    while (1)
    {
        Drive_key = PORT_GetBit(DRIVE_MOTOR_START_PORT, DRIVE_MOTOR_START_PIN);
        Suction_key = PORT_GetBit(SUCTION_MOTOR_START_PORT, SUCTION_MOTOR_START_PIN);
        For_Back_key = PORT_GetBit(FOR_BACK_PORT, FOR_BACK_PIN);
        Brush_key = PORT_GetBit(BRU_MOTOR_START_PORT, BRU_MOTOR_START_PIN);

        if (Drive_key == Set)
        {
            Drive_key_Count = 0;
            Key_tInfo.Drive_key_Flag = 0;
        }
        else if ((Drive_key == Reset) && (Key_tInfo.Drive_key_Flag == 0))  //把手开关
        {
            Drive_key_Count++;
            if (Drive_key_Count > 50)  //延时250ms进行判断
            {
                Key_tInfo.Drive_key_Flag = 1;
            }
        }

        if (Suction_key == Set)
        {
            Suction_key_Count = 0;
            Key_tInfo.Suction_key_Flag = 0;
        }
        else if ((Suction_key == Reset) && (Key_tInfo.Suction_key_Flag == 0))   //吸风电机
        {
            Suction_key_Count++;
            if (Suction_key_Count > 50)  //延时250ms进行判断
            {
                Key_tInfo.Suction_key_Flag = 1;
            }
        }

        if (For_Back_key == Set)    //前进后退按钮
        {
            Back_key_Count = 0;
            For_key_Count++;
            if (For_key_Count > 50)
            {
                Key_tInfo.Back_key_Flag = 0;
                Key_tInfo.For_key_Flag = 1;
            }
        }
        else
        {
            For_key_Count = 0;
            Back_key_Count++;
            if (Back_key_Count > 50)
            {
                Key_tInfo.For_key_Flag = 0;
                Key_tInfo.Back_key_Flag = 1;
            }
        }

        if (Brush_key == Set)
        {
            Brush_key_Count = 0;
            Key_tInfo.Brush_key_Flag = 0;
        }
        else if ((Brush_key == Reset) && (Key_tInfo.Brush_key_Flag == 0))   //盘刷电机
        {
            Brush_key_Count++;
            if (Brush_key_Count > 50)  //延时250ms进行判断
            {
                Key_tInfo.Brush_key_Flag = 1;
            }
        }

        if (Key_tInfo.Drive_key_Flag == 1)//把手开关按下
        {
            Drive_key_Flag = 2;
            if ((Key_ptGetInfo()->For_key_Flag == 1) && (Key_ptGetInfo()->Back_key_Flag == 0))     //前进按钮按下
            {
                Key_tInfo.Target_Speed = (Display_tGetInfo()->Speed_Level) * 160; //10个档位 分别控制pwm0-1600 一个等级*160
            }
            if ((Key_ptGetInfo()->For_key_Flag == 0) && (Key_ptGetInfo()->Back_key_Flag == 1))     //后退按钮按下
            {
                Key_tInfo.Target_Speed = -((Display_tGetInfo()->Speed_Level) * 160); //10个档位 分别控制pwm0-1600 一个等级*160
            }
            
        }
        else if ((Key_tInfo.Drive_key_Flag == 0) && (Drive_key_Flag == 2))//把手开关释放
        {
            Drive_key_Flag = 0;
            Key_tInfo.Moment_Speed = (float)Drive_Motor_ptGetInfo()->v_rt_speed;

        }
        else if ((Key_tInfo.Drive_key_Flag == 0) && (Drive_key_Flag == 0))//把手开关松开
        {
            Key_tInfo.Target_Speed = 0;//目标速度为0
        }
        rt_thread_mdelay(5);
    }
}








TKeyInfoDef* Key_ptGetInfo(void)
{
    return &Key_tInfo;
}

