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

static float vola_dec = 0;  //ֹͣʱ�̵ļ��ٶ�(�̶�����ʱ��1s)



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

/*  �г̵���ٶȺ���    */
static void Drive_For_Speed_vTaskHandler(void);
static void Drive_Back_Speed_vTaskHandler(void);
static void Drive_Stop_Speed_vTaskHandler(void);

/*  ��ˢ����ٶȺ���    */
static void Brush_Work_Speed_vTaskHandler(void);
static void Brush_Work_Stop_vTaskHandler(void);
/*  �������ٶȺ���    */
static void Suction_Work_Speed_vTaskHandler(void);
static void Suction_Work_Stop_vTaskHandler(void);

/*�ӿڲ�������*/
TspeedInfo* Drive_Motor_ptGetInfo(void);

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
    rt_thread_mdelay(100);
    while (1)
    {
        if (Key_ptGetInfo()->Brush_key_Flag == 1)
        {
            Brush_Work_Speed_vTaskHandler();                //��ˢ�������
            if (Drive_Speed_tInfo.v_rt_speed != 0)          //�г̵����ǰ�ٶȣ�=0
            {
                PORT_SetBits(SOLENOID_PORT, SOLENOID_PIN);        //�򿪵�ŷ�
            }
            
        }
        if (Key_ptGetInfo()->Brush_key_Flag == 0)
        {
            Brush_Work_Stop_vTaskHandler();//��ˢ���ֹͣ
            PORT_ResetBits(SOLENOID_PORT, SOLENOID_PIN);          //�رյ�ŷ�
        }
        if (Key_ptGetInfo()->Suction_key_Flag == 1)
        {
            Suction_Work_Speed_vTaskHandler();//����������
        }
        if (Key_ptGetInfo()->Suction_key_Flag == 0)
        {
            Suction_Work_Stop_vTaskHandler();//������ֹͣ
        }
        /*  ���ֿ��ذ��� &&  ǰ����ť���� &&���˰�ťΪ0 && ��ǰ�ٶ� >=0*/
        if ((Key_ptGetInfo()->Drive_key_Flag == 1)&& (Key_ptGetInfo()->For_key_Flag == 1) && (Key_ptGetInfo()->Back_key_Flag == 0) && (Drive_Speed_tInfo.v_rt_speed >= 0))
        {   
            Drive_For_Speed_vTaskHandler(); //��ת���ͼ���
        }
        /*  ���ֿ��ذ��� &&  ǰ����ť���� && ���˰�ťΪ0 && ��ǰ�ٶ� <0     ����ǰ��ͻȻ�л�����*/
        if ((Key_ptGetInfo()->Drive_key_Flag == 1) && (Key_ptGetInfo()->For_key_Flag == 1) && (Key_ptGetInfo()->Back_key_Flag == 0) && (Drive_Speed_tInfo.v_rt_speed < 0))
        {
            Drive_Stop_Speed_vTaskHandler();//ֹͣ
        }
        /*  ���ֿ��ذ��� &&  ǰ����ťΪ0 && ���˰�ť���� && ��ǰ�ٶ� <=0*/
        if ((Key_ptGetInfo()->Drive_key_Flag == 1) && (Key_ptGetInfo()->For_key_Flag == 0) && (Key_ptGetInfo()->Back_key_Flag == 1) && (Drive_Speed_tInfo.v_rt_speed <= 0))
        {
            Drive_Back_Speed_vTaskHandler();//��ת
        }
        /*  ���ֿ��ذ��� &&  ǰ����ťΪ0 && ���˰�ť���� && ��ǰ�ٶ� >0      ���ں���ͻȻ�л�����*/
        if ((Key_ptGetInfo()->Drive_key_Flag == 1) && (Key_ptGetInfo()->For_key_Flag == 0) && (Key_ptGetInfo()->Back_key_Flag == 1) && (Drive_Speed_tInfo.v_rt_speed > 0))
        {
            Drive_Stop_Speed_vTaskHandler();//ֹͣ
        }
        /* �ɿ����ֿ���*/
        if (Key_ptGetInfo()->Drive_key_Flag == 0)
        {
            Drive_Stop_Speed_vTaskHandler();//ֹͣ
        }
        rt_thread_mdelay(10);
    }
}


static void Drive_For_Speed_vTaskHandler(void)
{
    if (Key_ptGetInfo()->Target_Speed > Drive_Speed_tInfo.v_rt_speed)                 //Ŀ���ٶ� > ��ǰ�ٶ�
    {
        Drive_Speed_tInfo.v_next_speed = Drive_Speed_tInfo.v_rt_speed + Drive_Speed_tInfo.v_add;  //��һ��ʱ���ٶ� = ��ǰ�ٶ�+���ٶ�

        if (Drive_Speed_tInfo.v_next_speed > Key_ptGetInfo()->Target_Speed)           //��һʱ���ٶ� > Ŀ���ٶ�
        {
            Drive_Speed_tInfo.v_rt_speed = Key_ptGetInfo()->Target_Speed;             //��ǰ�ٶ� = Ŀ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //��ǰ�ٶȸ�ֵpwm
        }      
    }
    if (Key_ptGetInfo()->Target_Speed < Drive_Speed_tInfo.v_rt_speed)                 //Ŀ���ٶ� < ��ǰ�ٶ�
    {
        Drive_Speed_tInfo.v_next_speed = Drive_Speed_tInfo.v_rt_speed - Drive_Speed_tInfo.v_dec;  //��һ��ʱ���ٶ� = ��ǰ�ٶ�-���ٶ�

        if (Drive_Speed_tInfo.v_next_speed < Key_ptGetInfo()->Target_Speed)           //��һʱ���ٶ� < Ŀ���ٶ�
        {
            Drive_Speed_tInfo.v_rt_speed = Key_ptGetInfo()->Target_Speed;             //��ǰ�ٶ� = Ŀ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //��ǰ�ٶȸ�ֵpwm
        }
    }
}


static void Drive_Back_Speed_vTaskHandler(void)
{
    if (Key_ptGetInfo()->Target_Speed < Drive_Speed_tInfo.v_rt_speed)                 //Ŀ���ٶ� < ��ǰ�ٶ�
    {
        Drive_Speed_tInfo.v_next_speed = Drive_Speed_tInfo.v_rt_speed - Drive_Speed_tInfo.v_add;  //��һ��ʱ���ٶ� = ��ǰ�ٶ�-���ٶ�

        if (Drive_Speed_tInfo.v_next_speed < Key_ptGetInfo()->Target_Speed)           //��һʱ���ٶ� < Ŀ���ٶ�
        {
            Drive_Speed_tInfo.v_rt_speed = Key_ptGetInfo()->Target_Speed;             //��ǰ�ٶ� = Ŀ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //��ǰ�ٶȸ�ֵpwm
        }
    }
    if (Key_ptGetInfo()->Target_Speed > Drive_Speed_tInfo.v_rt_speed)                 //Ŀ���ٶ� > ��ǰ�ٶ�
    {
        Drive_Speed_tInfo.v_next_speed = Drive_Speed_tInfo.v_rt_speed + Drive_Speed_tInfo.v_dec;  //��һ��ʱ���ٶ� = ��ǰ�ٶ�+���ٶ�

        if (Drive_Speed_tInfo.v_next_speed > Key_ptGetInfo()->Target_Speed)           //��һʱ���ٶ� > Ŀ���ٶ�
        {
            Drive_Speed_tInfo.v_rt_speed = Key_ptGetInfo()->Target_Speed;             //��ǰ�ٶ� = Ŀ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //��ǰ�ٶȸ�ֵpwm
        }
    }
}
static void Drive_Stop_Speed_vTaskHandler(void)
{
    vola_dec = abs((Key_ptGetInfo()->Moment_Speed / 100));
    if (Drive_Speed_tInfo.v_rt_speed > 0)                                            // ��ǰ�ٶ� > 0
    {
        Drive_Speed_tInfo.v_next_speed = (int16_t)((float)Drive_Speed_tInfo.v_rt_speed - vola_dec); //��һ��ʱ���ٶ� = ��ǰ�ٶ� - ���ֿ����ͷ�ʱ�ٶ�/100(10ms����һ�� 100�ξ���1s)

        if (Drive_Speed_tInfo.v_next_speed < 0)                                       //��һʱ���ٶ� < 0
        {
            Drive_Speed_tInfo.v_rt_speed = 0;                                         //��ǰ�ٶ� = 0
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //��ǰ�ٶȸ�ֵpwm
        }
    }
    if (Drive_Speed_tInfo.v_rt_speed < 0)                                            // ��ǰ�ٶ� < 0
    {
        Drive_Speed_tInfo.v_next_speed = (int16_t)((float)Drive_Speed_tInfo.v_rt_speed + vola_dec);   //��һ��ʱ���ٶ� = ��ǰ�ٶ� + ���ֿ����ͷ�ʱ�ٶ�/100(10ms����һ�� 100�ξ���1s)

        if (Drive_Speed_tInfo.v_next_speed > 0)                                       //��һʱ���ٶ� < 0
        {
            Drive_Speed_tInfo.v_rt_speed = 0;                                         //��ǰ�ٶ� = 0
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //Ŀ���ٶȸ�ֵpwm
        }
        else
        {
            Drive_Speed_tInfo.v_rt_speed = Drive_Speed_tInfo.v_next_speed;                  //���򣬵�ǰ�ٶ� = ��һʱ���ٶ�
            Time4_1_Drive_Motor_Contorl(Drive_Speed_tInfo.v_rt_speed);                //��ǰ�ٶȸ�ֵpwm
        }
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



TspeedInfo* Drive_Motor_ptGetInfo(void)
{
    return &Drive_Speed_tInfo;
}

/*
����  ��ǰ�ٶ�V_start
����  Ŀ���ٶ�V_end
����  �Ӽ��ٶ�jerk_acc
����  �Ӽ��ٶ�jerk_dec
����  ���ٶ����ֵacc_max
����  ���ٶ����ֵdec_max

        
if()




*/
