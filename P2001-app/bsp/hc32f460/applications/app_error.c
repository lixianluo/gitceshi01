#include "app_error.h"
#include <rtthread.h>
#include "adc.h"
#include "app_motor.h"
#include "gpio.h"
#include "board.h"

static uint32_t uiErrorCode = 0;
static uint16_t Suction_Count = 0;
static uint16_t Branch_Count = 0;



static void Error_TaskHandler(void);
static void vErrorCodeAdd(ErrorCode codenum);
static void vErrorCodeDel(ErrorCode codenum);

static void ERROR_vTaskHandler_Entry(void* parameter);
int ERROR_iTaskHandler(void)
{
    rt_thread_t tid1;
    /* ����ERROR_vTaskHandler_entry�� */
    tid1 = rt_thread_create("ERROR_vTaskHandler_entry",
        ERROR_vTaskHandler_Entry,
        RT_NULL,
        1024,
        4,
        20);
    /* �����ɹ��������߳� */
    if (tid1 != RT_NULL)rt_thread_startup(tid1);

    return 0;
}
INIT_APP_EXPORT(ERROR_iTaskHandler);


static void ERROR_vTaskHandler_Entry(void* parameter)
{
   
    while (1)
    {
        Error_TaskHandler();

        rt_thread_mdelay(10);
    }
}


static void Error_TaskHandler(void)
{
    float battery_voltage = ADC_ptGetInfo()->fConvertValue[ADC_IDX_2];
    float branch_current = ADC_ptGetInfo()->fConvertValue[ADC_IDX_1];
    float suction_current = ADC_ptGetInfo()->fConvertValue[ADC_IDX_0];
    
    int16_t suction_speed = ptApp_Motor_Branch()->v_rt_speed;
    int16_t branch_speed = ptApp_Motor_Suction()->v_rt_speed;
    
    if (battery_voltage < 19.2f)
    {
        /*���Ӵ������------------------------*/
        vErrorCodeAdd(Battery_Under);
        /*------------------------------------*/

        /*ɾ���������------------------------*/
        vErrorCodeDel(Battery_Empty);
        vErrorCodeDel(Battery_Over);
        /*------------------------------------*/
    }
    if ((battery_voltage > 19.2f) && (battery_voltage < 22.0f))
    {
        /*���Ӵ������------------------------*/
        vErrorCodeAdd(Battery_Empty);
        /*------------------------------------*/

        /*ɾ���������------------------------*/
        vErrorCodeDel(Battery_Under);
        vErrorCodeDel(Battery_Over);
        /*------------------------------------*/
    }
    if (battery_voltage > 30.5f)
    {
        /*���Ӵ������------------------------*/
        vErrorCodeAdd(Battery_Over);
        /*------------------------------------*/

        /*ɾ���������------------------------*/
        vErrorCodeDel(Battery_Empty);
        vErrorCodeDel(Battery_Under);
        /*------------------------------------*/
    }
    if ((battery_voltage > 22.0f) && (battery_voltage < 30.5f))
    { 
        /*ɾ���������------------------------*/
        vErrorCodeDel(Battery_Empty);
        vErrorCodeDel(Battery_Under);
        vErrorCodeDel(Battery_Under);
        /*------------------------------------*/
    }
//    if ((suction_speed > 150) && (suction_current < 2.0f))      //����������������<2.0A �� ��������·����
//    {
//        vErrorCodeAdd(Suction_Open);                            //���� ���� �����·����
//        SUCTION_DISABLE();				                        //������ʧ��
//    }
//    else
//    {
//        vErrorCodeDel(Suction_Open);                            //ɾ�� ���� �����·����
//        SUCTION_ENABLE();			                            //������ʹ��
//    }
//    if ((branch_speed > 150) && (branch_current < 2.0f))        //��ˢ�������������<2.0A �� ��ˢ�����·����
//    {
//        vErrorCodeAdd(Branch_Open);                             //���� ��ˢ �����·����
//        BRUSH_DISABLE();				                        //��ˢ���ʧ��
//    }
//    else
//    {
//        vErrorCodeDel(Branch_Open);                             //ɾ�� ��ˢ �����·����
//        BRUSH_ENABLE();				                            //��ˢ���ʹ��
//    }

//    if (suction_current > 18.0f)                                //�������>18A,ֱ�ӱ�����
//    {
//        vErrorCodeAdd(Suction_Over_Current);                    //���� ���� �����������
//        SUCTION_DISABLE();				//������ʧ��
//    }
//    else if ((suction_current > 13.0f) && (suction_current < 18.0f))    //13A<����<18A ��������5s ������
//    {
//        Suction_Count++;
//        if (Suction_Count > 500)
//        {
//            vErrorCodeAdd(Suction_Over_Current);                //���� ���� �����������
//            SUCTION_DISABLE();				//������ʧ��
//        }
//    }
//    else if(suction_current < 13.0f)
//    {
//        vErrorCodeDel(Suction_Over_Current);                    //ɾ�� ���� �����������
//        SUCTION_ENABLE();			//������ʹ��
//        Suction_Count = 0;
//    }
//   
//    if (branch_current > 18.0f)                                    //��ˢ����>18A,ֱ�ӱ�����
//    {
//        vErrorCodeAdd(Branch_Over_Current);
//        BRUSH_DISABLE();				//��ˢ���ʧ��
//    }
//    else if ((branch_current > 13.0f) && (branch_current < 18.0f))    //13A<����<18A ��������5s ������
//    {
//        Suction_Count++;
//        if (Suction_Count > 500)
//        {
//            vErrorCodeAdd(Branch_Over_Current);
//            BRUSH_DISABLE();				//��ˢ���ʧ��
//        }
//    }
//    else if (branch_current < 13.0f)
//    {
//        vErrorCodeDel(Branch_Over_Current);
//        BRUSH_ENABLE();				//��ˢ���ʹ��
//        Suction_Count = 0;
//    }
}



static void vErrorCodeAdd(ErrorCode codenum)
{
    uiErrorCode = uiErrorCode | (0x0001 << codenum);
}

static void vErrorCodeDel(ErrorCode codenum)
{
    uiErrorCode = uiErrorCode & (~(0x0001 << codenum));
}
uint32_t Error_GetInfo(void)
{
    return uiErrorCode;
}
