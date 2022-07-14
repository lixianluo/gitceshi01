#include "app_key.h"
#include "gpio.h"
#include "board.h"
#include "app_key.h"
#include "gpio.h"
#include "board.h"
#include "adc.h"
#include "app_motor.h"





/*私有变量-------------------------------------------------------*/
static TKeyInfoDef Key_tInfo;
static TKey_CountInfoDef Key_tCount;
/*---------------------------------------------------------------*/

/*私有函数-------------------------------------------------------*/
static void Key_vTaskHandler_entry(void* parameter);
TKeyInfoDef* Key_ptGetInfo(void);
static void Key_Scan(void);
static void Key_Output(void);
/*---------------------------------------------------------------*/

int Key_iTaskHandler(void)
{
    rt_thread_t tid1;
    /* 创建Key_vTaskHandler_entry线程 */
    tid1 = rt_thread_create("Key_vTaskHandler_entry",
        Key_vTaskHandler_entry,
        RT_NULL,
        1024,
        4,
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
        Key_Scan();
        Key_Output();
        rt_thread_mdelay(5);
    }
}

static void Key_Scan(void)
{
    uint32_t Off_vlaue;
    Off_vlaue = ADC_ptGetInfo()->ulADValue[ADC_IDX_2];
    if (PORT_GetBit(SWITCH_B3_PORT, SWITCH_B3_PIN) == Reset)          //B3被接通
    {
        Key_tCount.B3_Count++;
        if (Key_tCount.B3_Count >= 30)
        {
            Key_tCount.B3_Flag = 1;

        }
    }
    else if (PORT_GetBit(SWITCH_B3_PORT, SWITCH_B3_PIN) == Set)
    {
        Key_tCount.B3_Count = 0;
        Key_tCount.B3_Flag = 0;
    }

    if (PORT_GetBit(SWITCH_A2_PORT, SWITCH_A2_PIN) == Reset)          //A2被接通
    {
        Key_tCount.A2_Count++;
        if (Key_tCount.A2_Count >= 30)
        {
            Key_tCount.A2_Flag = 1;

        }
    }
    else if (PORT_GetBit(SWITCH_A2_PORT, SWITCH_A2_PIN) == Set)
    {
        Key_tCount.A2_Count = 0;
        Key_tCount.A2_Flag = 0;
    }

    if (PORT_GetBit(SWITCH_A1_PORT, SWITCH_A1_PIN) == Reset)          //A1被接通
    {
        Key_tCount.A1_Count++;
        if (Key_tCount.A1_Count >= 30)
        {
            Key_tCount.A1_Flag = 1;

        }
    }
    else if (PORT_GetBit(SWITCH_A1_PORT, SWITCH_A1_PIN) == Set)
    {
        Key_tCount.A1_Count = 0;
        Key_tCount.A1_Flag = 0;
    }

    if ((PORT_GetBit(HAND_PORT_1, HAND_PIN_1) == Reset)||(PORT_GetBit(HAND_PORT_2, HAND_PIN_2) == Reset))                     //把手开关被接通
    {
        Key_tCount.Hand_Count++;
        if (Key_tCount.Hand_Count >= 30)
        {
            Key_tCount.Hand_Flag = 1;

        }
    }
    else if ((PORT_GetBit(HAND_PORT_1, HAND_PIN_1) == Set)&& (PORT_GetBit(HAND_PORT_2, HAND_PIN_2) == Set))
    {
        Key_tCount.Hand_Count = 0;
        Key_tCount.Hand_Flag = 0;
    }

    if (Off_vlaue < 500)                                            //AD关机采样：关机为2.5V，开机为24V
    {
        Key_tCount.Off_Flag = 1;
    }
    else if (Off_vlaue > 500)
    {
        Key_tCount.Off_Flag = 0;
    }
}

static void Key_Output(void)
{
    if (Key_tCount.Hand_Flag == 1)          //目前双把手单信号，后期增加可扩展
    {
        Key_tInfo.Hand_Key_FLag = 1;
    }
    else
    {
        Key_tInfo.Hand_Key_FLag = 0;
    }

    if (Key_tCount.Off_Flag == 1)          //关机模式
    {
        Key_tInfo.Off_Key_Flag = 1;
    }
    else
    {
        Key_tInfo.Off_Key_Flag = 0;
    }


    if (Key_tInfo.Off_Key_Flag == 1)                                                                                                //旋钮开关第一档 默认为：关机模式
    {
        Key_tInfo.All_Key_Flag = 0;
        Key_tInfo.Suction_Key_Flag = 0;
        Key_tInfo.Brush_Key_Flag = 0;
        Key_tInfo.Work_Status = (uint8_t)OFF_MODE;
    }
    else if ((Key_tCount.B3_Flag == 1) && (Key_tCount.A1_Flag == 0) && (Key_tCount.A2_Flag == 0) && (Key_tInfo.Off_Key_Flag == 0))  //旋钮开关第二档 默认为：全功率模式
    {
        Key_tInfo.All_Key_Flag = 1;
        Key_tInfo.Suction_Key_Flag = 0;
        Key_tInfo.Brush_Key_Flag = 0;
        Key_tInfo.Work_Status = (uint8_t)ALL_MODE;
    }
    else if ((Key_tCount.B3_Flag == 1) && (Key_tCount.A1_Flag == 0) && (Key_tCount.A2_Flag == 1) && (Key_tInfo.Off_Key_Flag == 0))  //旋钮开关第三档 默认为：只刷模式
    {
        Key_tInfo.All_Key_Flag = 0;
        Key_tInfo.Suction_Key_Flag = 0;
        Key_tInfo.Brush_Key_Flag = 1;
       Key_tInfo.Work_Status = (uint8_t)BRUSH_MODE;
    }
    else if ((Key_tCount.B3_Flag == 1) && (Key_tCount.A1_Flag == 1) && (Key_tCount.A2_Flag == 1) && (Key_tInfo.Off_Key_Flag == 0))  //旋钮开关第四档 默认为：只吸模式
    {
        Key_tInfo.All_Key_Flag = 0;
        Key_tInfo.Suction_Key_Flag = 1;
        Key_tInfo.Brush_Key_Flag = 0;
        Key_tInfo.Work_Status = (uint8_t)SUCTION_MODE;
    }

}

TKeyInfoDef* Key_ptGetInfo(void)
{
    return &Key_tInfo;
}
