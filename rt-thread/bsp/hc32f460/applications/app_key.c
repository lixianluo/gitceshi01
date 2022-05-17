#include "app_key.h"
#include "gpio.h"
#include "board.h"
#include "app_key.h"
#include "gpio.h"
#include "board.h"
#include "adc.h"
#include "app_motor.h"


static void Key_vTaskHandler_entry(void* parameter);
TKeyInfoDef* Key_ptGetInfo(void);



static TKeyInfoDef Key_tInfo;
static TKey_CountInfoDef Key_tCount;



int Key_iTaskHandler(void)
{
    rt_thread_t tid1;
    /* ����Key_vTaskHandler_entry�߳� */
    tid1 = rt_thread_create("Key_vTaskHandler_entry",
        Key_vTaskHandler_entry,
        RT_NULL,
        1024,
        6,
        20);
    /* �����ɹ��������߳� */
    if (tid1 != RT_NULL)rt_thread_startup(tid1);
    return 0;
}
INIT_APP_EXPORT(Key_iTaskHandler);


static void Key_vTaskHandler_entry(void* parameter)
{
    uint32_t Off_vlaue;
    rt_thread_mdelay(50);
    while (1)
    {
        Off_vlaue = ADC_ptGetInfo()->ulADValue[ADC_IDX_3];
        if (PORT_GetBit(BRUSH_START_PORT, BRUSH_START_PIN) == Reset)          //��ˢ�����ť����
        {
            Key_tCount.Brush_key_Flag++;
            if (Key_tCount.Brush_key_Flag >= 40)
            {
                Key_tInfo.Brush_key_Flag = 1;
            }
        }
        else if (PORT_GetBit(BRUSH_START_PORT, BRUSH_START_PIN) == Set)
        {
            Key_tInfo.Brush_key_Flag = 0;
            Key_tCount.Brush_key_Flag = 0;
        }
        

        if (PORT_GetBit(SUCTION_START_PORT, SUCTION_START_PIN) == Reset)      //��������ť����
        {
            Key_tCount.Suction_key_Flag++;
            if (Key_tCount.Suction_key_Flag >= 40)
            {
                Key_tInfo.Suction_key_Flag = 1;
            }
        }
        else if (PORT_GetBit(SUCTION_START_PORT, SUCTION_START_PIN) == Set)
        {
            Key_tInfo.Suction_key_Flag = 0;
            Key_tCount.Suction_key_Flag = 0;
        }


        if (PORT_GetBit(ALL_START_PORT, ALL_START_PIN) == Reset)              //��ˢ+��������ť����
        {
            Key_tCount.All_key_Flag++;
            if (Key_tCount.All_key_Flag >= 40)
            {
                Key_tInfo.All_key_Flag = 1;
            }
        }
        else if (PORT_GetBit(ALL_START_PORT, ALL_START_PIN) == Set)
        {
            Key_tInfo.All_key_Flag = 0;
            Key_tCount.All_key_Flag = 0;
        }


        if (PORT_GetBit(HAND_PORT, HAND_PIN) == Reset)                        //���ֿ��ذ���
        {
            Key_tCount.Hand_FLag++;
            if (Key_tCount.Hand_FLag >= 40)
            {
                Key_tInfo.Hand_FLag = 1;
            }
            
        }
        else  if (PORT_GetBit(HAND_PORT, HAND_PIN) == Set)
        {
            Key_tInfo.Hand_FLag = 0;
            Key_tCount.Hand_FLag = 0;
        }
       
        if (Off_vlaue < 500)  //AD�ػ��������ػ�Ϊ0-2������Ϊ3164-3167
        {
            Key_tInfo.Off_key_Flag = 1;
        }
        else if (Off_vlaue > 500)
        {
            Key_tInfo.Off_key_Flag = 0;
        }

        rt_thread_mdelay(5);
    }
}








TKeyInfoDef* Key_ptGetInfo(void)
{
    return &Key_tInfo;
}
