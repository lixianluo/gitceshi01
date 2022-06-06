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


/*  ��ˢ����ٶȺ���    */
static void Brush_Work_Speed_vTaskHandler(void);
static void Brush_Work_Stop_vTaskHandler(void);
/*  �������ٶȺ���    */
static void Suction_Work_Speed_vTaskHandler(void);
static void Suction_Work_Stop_vTaskHandler(void);


int MOTOR_iTaskHandler(void)
{
    rt_thread_t tid1;
    /* ����MOTOR_vTaskHandler_entry�߳� */
    tid1 = rt_thread_create("MOTOR_vTaskHandler_entry",
        MOTOR_vTaskHandler_Entry,
        RT_NULL,
        1024,
        5,
        20);
    /* �����ɹ��������߳� */
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
        /*��ˢ���*/
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
        /*������*/
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
    
    if (200 > Brush_Speed_tInfo.v_rt_speed)                 //Ŀ���ٶ� > ��ǰ�ٶ�
    {
        Brush_Speed_tInfo.v_next_speed = Brush_Speed_tInfo.v_rt_speed + Brush_Speed_tInfo.v_add;  //��һ��ʱ���ٶ� = ��ǰ�ٶ�+���ٶ�

        if (Brush_Speed_tInfo.v_next_speed > 200)           //��һʱ���ٶ� > Ŀ���ٶ�
        {
            Brush_Speed_tInfo.v_rt_speed = 200;             //��ǰ�ٶ� = Ŀ���ٶ�
            Timera_Motor_Contorl(Brush_Motor, Enable, Brush_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Brush_Speed_tInfo.v_rt_speed = Brush_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Timera_Motor_Contorl(Brush_Motor, Enable, Brush_Speed_tInfo.v_rt_speed);               //��ǰ�ٶȸ�ֵpwm
        }
    }
}
static void Brush_Work_Stop_vTaskHandler(void)
{

    if (Brush_Speed_tInfo.v_rt_speed > 0)                 //��ǰ�ٶ� > 0
    {
        Brush_Speed_tInfo.v_next_speed = Brush_Speed_tInfo.v_rt_speed - Brush_Speed_tInfo.v_dec;  //��һ��ʱ���ٶ� = ��ǰ�ٶ�-���ٶ�

        if (Brush_Speed_tInfo.v_next_speed < 0)           //��һʱ���ٶ� < 0
        {
            Brush_Speed_tInfo.v_rt_speed = 0;             //��ǰ�ٶ� = Ŀ���ٶ�
            Timera_Motor_Contorl(Brush_Motor, Enable, Brush_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Brush_Speed_tInfo.v_rt_speed = Brush_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Timera_Motor_Contorl(Brush_Motor, Enable, Brush_Speed_tInfo.v_rt_speed);               //��ǰ�ٶȸ�ֵpwm
        }
    }
}



static void Suction_Work_Speed_vTaskHandler(void)
{
    if (200 > Suction_Speed_tInfo.v_rt_speed)                 //Ŀ���ٶ� > ��ǰ�ٶ�
    {
        Suction_Speed_tInfo.v_next_speed = Suction_Speed_tInfo.v_rt_speed + Suction_Speed_tInfo.v_add;  //��һ��ʱ���ٶ� = ��ǰ�ٶ�+���ٶ�

        if (Suction_Speed_tInfo.v_next_speed > 200)           //��һʱ���ٶ� > Ŀ���ٶ�
        {
            Suction_Speed_tInfo.v_rt_speed = 200;             //��ǰ�ٶ� = Ŀ���ٶ�
            Timera_Motor_Contorl(Suction_Motor, Enable, Suction_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Suction_Speed_tInfo.v_rt_speed = Suction_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Timera_Motor_Contorl(Suction_Motor, Enable, Suction_Speed_tInfo.v_rt_speed);               //��ǰ�ٶȸ�ֵpwm
        }
    }

}
static void Suction_Work_Stop_vTaskHandler(void)
{
    if (Suction_Speed_tInfo.v_rt_speed > 0)                 //��ǰ�ٶ� > 0
    {
        Suction_Speed_tInfo.v_next_speed = Suction_Speed_tInfo.v_rt_speed - Suction_Speed_tInfo.v_dec;  //��һ��ʱ���ٶ� = ��ǰ�ٶ�-���ٶ�

        if (Suction_Speed_tInfo.v_next_speed < 0)           //��һʱ���ٶ� < 0
        {
            Suction_Speed_tInfo.v_rt_speed = 0;             //��ǰ�ٶ� = Ŀ���ٶ�
            Timera_Motor_Contorl(Suction_Motor, Enable, Suction_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Suction_Speed_tInfo.v_rt_speed = Suction_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Timera_Motor_Contorl(Suction_Motor, Enable, Suction_Speed_tInfo.v_rt_speed);               //��ǰ�ٶȸ�ֵpwm
        }
    }

}


