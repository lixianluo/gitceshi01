#include "app.display.h"
#include "board.h"
#include <rtthread.h>
#include "lcd1602.h"
#include "adc.h"
#include "app_key.h"
#include "stdlib.h"

extern uint32_t systick;

static uint8_t  Compare_buff_1[16] = {0};
static uint8_t  Compare_buff_2[16] = { 0 };
static TDisplayInfoDef Display_tInfo;
static uint32_t LastTimeADC_IDX_0;
extern uint32_t systick;
static uint32_t lastsystick = 0;
static const uint8_t table1[] = { "      Speed     " };
static const uint8_t table2[] = { "           Taung" };

static const uint8_t table3[16] = { 0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20,0x20 };//清屏写入

static const uint8_t table[11][16] =
{
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x20,0x20,0x31,0x30,0x30,0x25 },//********** 100%
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x20,0x20,0x39,0x30,0x25,0x20 },//*********_ 90%
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x20,0x20,0x38,0x30,0x25,0x20 },//********__ 80%
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x20,0x20,0x37,0x30,0x25,0x20 },//*******___ 70%
    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x20,0x20,0x36,0x30,0x25,0x20 },//******____ 60%
    {0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x20,0x20,0x35,0x30,0x25,0x20 },//*****_____ 50%
    {0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x20,0x20,0x34,0x30,0x25,0x20 },//****______ 40%
    {0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x20,0x20,0x33,0x30,0x25,0x20 },//***_______ 30%
    {0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x20,0x20,0x32,0x30,0x25,0x20 },//**________ 20%
    {0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x20,0x20,0x31,0x30,0x25,0x20 },//*_________ 10%
    {0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x20,0x20,0x4D,0x69,0x6E,0x20 },//__________ Min
};

static const uint8_t table4[11][16] =
{
    {0x42,0x41,0x54,0x20,0x45,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//BAT E__________F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//BAT E*_________F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//BAT E**________F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//BAT E***_______F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//BAT E****______F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//BAT E*****_____F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x46 },//BAT E******____F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x46 },//BAT E*******___F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x46 },//BAT E********__F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x46 },//BAT E*********_F
    {0x42,0x41,0x54,0x20,0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x46 },//BAT E**********F 24.
};

//static void one_minute_timeout(void* parameter);
static void Display_vTaskHandler_entry(void* parameter);
static void LastTimeValue_vTaskHandler_entry(void* parameter);
static void Speed_Init(void);
static void Display_Init(void);
static void Display_Idle(void);
static void Display_Speed(void);
static void Display_Empty(void);
static void Display_Error(void);
static void DisplayErrorHandler(void);
int Display_iTaskHandler(void)
{
    rt_thread_t tid1,tid2;
    /* 创建Display_vTaskHandler_entry线程 */
    tid1 = rt_thread_create("Display_vTaskHandler_entry",
        Display_vTaskHandler_entry,
        RT_NULL,
        1024,
        6,
        20);
    /* 创建成功则启动线程 */
    if (tid1 != RT_NULL)rt_thread_startup(tid1);
    
    tid2 = rt_thread_create("LastTimeValue_vTaskHandler_entry",
        LastTimeValue_vTaskHandler_entry,
        RT_NULL,
        512,
        9,
        10);
    /* 创建成功则启动线程 */
    if (tid2 != RT_NULL)rt_thread_startup(tid2);

    return 0;
}
INIT_APP_EXPORT(Display_iTaskHandler);


static void Display_vTaskHandler_entry(void* parameter)
{
    //rt_thread_mdelay(100);
    while (1)
    {
        switch (Display_tInfo.tDisplayTaskState)
        {
            case DISPLAY_TASK_INIT:                  //LCD1602初始化显示
            {
                IIC_LCD1602_Write_Init();
                Speed_Init();
                Display_Init();
			  
                    Display_tInfo.tDisplayTaskState = DISPLAY_TASK_IDLE;
				
                break;
            }
            case DISPLAY_TASK_IDLE:                 //LCD1602空闲显示(电量)
            {
                Display_Idle();
                if ((abs(LastTimeADC_IDX_0 - ADC_ptGetInfo()->ulADValue[ADC_IDX_0]))>10)
                {
                    Display_tInfo.tDisplayTaskState = DISPLAY_TASK_RUN_SPEED;
                    lastsystick = systick;
                }
                
                break;
            }
            case DISPLAY_TASK_RUN_SPEED:            //LCD1602速度显示
            {
                Display_Speed();
                if ((abs(LastTimeADC_IDX_0 - ADC_ptGetInfo()->ulADValue[ADC_IDX_0])) > 10)
                {
                    lastsystick = systick;
                }
                if ((systick - lastsystick) > 2)
                {    
                    Display_Empty();
                    Display_tInfo.tDisplayTaskState = DISPLAY_TASK_IDLE;
                }
                break;
            }
            case DISPLAY_TASK_RUN_ERROR:            //LCD1602错误代码显示
            {

                break;
            }
            default:break;
        }
      if((systick % 3)==0)
			{
				DisplayErrorHandler();
			}
	    rt_thread_mdelay(8);
    }
}

static void LastTimeValue_vTaskHandler_entry(void* parameter)
{
    while (1)
    {
        LastTimeADC_IDX_0 = ADC_ptGetInfo()->ulADValue[ADC_IDX_0];
				
        rt_thread_mdelay(5);
    }
}
static void Display_Empty(void)
{
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table3), (uint8_t*)table3);  //上行写入空
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table3), (uint8_t*)table3);  //下行写入空
}
static void Display_Speed(void)
{  
    /*
    AD值4096分11份  分别为0-372-744-1117-1489-1861-2234-2606-2978-3351-3723-4096
    */
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table1) - 1, (uint8_t*)table1);
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 372) && (&ADC_ptGetInfo()->ulADValue[ADC_IDX_0] >= 0))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[10]), (uint8_t*)table[10]);
        Display_tInfo.Speed_Level = 0;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 744) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 372))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[9]), (uint8_t*)table[9]);
        Display_tInfo.Speed_Level = 1;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 1117) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 744))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[8]), (uint8_t*)table[8]);
        Display_tInfo.Speed_Level = 2;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 1489) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 1117))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[7]), (uint8_t*)table[7]);
        Display_tInfo.Speed_Level = 3;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 1861) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 1489))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[6]), (uint8_t*)table[6]);
        Display_tInfo.Speed_Level = 4;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 2234) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 1861))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[5]), (uint8_t*)table[5]);
        Display_tInfo.Speed_Level = 5;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 2606) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 2234))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[4]), (uint8_t*)table[4]);
        Display_tInfo.Speed_Level = 6;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 2978) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 2606))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[3]), (uint8_t*)table[3]);
        Display_tInfo.Speed_Level = 7;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 3351) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 2978))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[2]), (uint8_t*)table[2]);
        Display_tInfo.Speed_Level = 8;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 3723) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 3351))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[1]), (uint8_t*)table[1]);
        Display_tInfo.Speed_Level = 9;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 4096) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 3723))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table[0]), (uint8_t*)table[0]);
        Display_tInfo.Speed_Level = 10;
    }
    
}
static void Display_Init(void)
{
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table2)-1, (uint8_t*)table2); //上行写入Taung
		
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(table3), (uint8_t*)table3);  //下行写入空
    rt_thread_mdelay(1500);
}
static void Display_Idle(void)
{
    if ((Key_ptGetInfo()->Brush_key_Flag == 1) || (Key_ptGetInfo()->Suction_key_Flag == 1) || (Key_ptGetInfo()->Drive_key_Flag == 1)) //工作模式电量显示
    {
        if (ADC_ptGetInfo()->supply_voltage >= 24.5f)
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[10]), (uint8_t*)table4[10]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 24.3f) && (ADC_ptGetInfo()->supply_voltage < 24.5f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[9]), (uint8_t*)table4[9]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 24.1f) && (ADC_ptGetInfo()->supply_voltage < 24.3f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[8]), (uint8_t*)table4[8]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.7f) && (ADC_ptGetInfo()->supply_voltage < 24.1f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[7]), (uint8_t*)table4[7]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.3f) && (ADC_ptGetInfo()->supply_voltage < 23.7f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[6]), (uint8_t*)table4[6]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.0f) && (ADC_ptGetInfo()->supply_voltage < 23.3f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[5]), (uint8_t*)table4[5]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 22.8f) && (ADC_ptGetInfo()->supply_voltage < 23.0f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[4]), (uint8_t*)table4[4]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 22.6f) && (ADC_ptGetInfo()->supply_voltage < 22.8f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[3]), (uint8_t*)table4[3]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 22.4f) && (ADC_ptGetInfo()->supply_voltage < 22.6f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[2]), (uint8_t*)table4[2]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 22.2f) && (ADC_ptGetInfo()->supply_voltage < 22.4f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[1]), (uint8_t*)table4[1]);
        }
        if (ADC_ptGetInfo()->supply_voltage < 22.2f)
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[0]), (uint8_t*)table4[0]);
        }
			
    }
    else                                                                                                                   //待机模式电量显示
    {
        if (ADC_ptGetInfo()->supply_voltage >= 24.9f)
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[10]), (uint8_t*)table4[10]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 24.7f) && (ADC_ptGetInfo()->supply_voltage < 24.9f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[9]), (uint8_t*)table4[9]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 24.5f) && (ADC_ptGetInfo()->supply_voltage < 24.7f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[8]), (uint8_t*)table4[8]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 24.3f) && (ADC_ptGetInfo()->supply_voltage < 24.5f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[7]), (uint8_t*)table4[7]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 24.1f) && (ADC_ptGetInfo()->supply_voltage < 24.3f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[6]), (uint8_t*)table4[6]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.9f) && (ADC_ptGetInfo()->supply_voltage < 24.1f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[5]), (uint8_t*)table4[5]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.7f) && (ADC_ptGetInfo()->supply_voltage < 23.9f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[4]), (uint8_t*)table4[4]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.5f) && (ADC_ptGetInfo()->supply_voltage < 23.7f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[3]), (uint8_t*)table4[3]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.3f) && (ADC_ptGetInfo()->supply_voltage < 23.5f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[2]), (uint8_t*)table4[2]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.1f) && (ADC_ptGetInfo()->supply_voltage < 23.3f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[1]), (uint8_t*)table4[1]);
        }
        if (ADC_ptGetInfo()->supply_voltage < 23.1f)
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(table4[0]), (uint8_t*)table4[0]);
        }
    }  
}
static void Speed_Init(void)
{
    /*
    AD值4096分11份  分别为0-372-744-1117-1489-1861-2234-2606-2978-3351-3723-4096
    */
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 372) && (&ADC_ptGetInfo()->ulADValue[ADC_IDX_0] >= 0))
    {
        Display_tInfo.Speed_Level = 0;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 744) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 372))
    {
        Display_tInfo.Speed_Level = 1;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 1117) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 744))
    {
        Display_tInfo.Speed_Level = 2;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 1489) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 1117))
    {
        Display_tInfo.Speed_Level = 3;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 1861) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 1489))
    {
        Display_tInfo.Speed_Level = 4;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 2234) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 1861))
    {
        Display_tInfo.Speed_Level = 5;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 2606) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 2234))
    {
        Display_tInfo.Speed_Level = 6;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 2978) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 2606))
    {
        Display_tInfo.Speed_Level = 7;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 3351) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 2978))
    {
        Display_tInfo.Speed_Level = 8;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 3723) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 3351))
    {
        Display_tInfo.Speed_Level = 9;
    }
    if ((ADC_ptGetInfo()->ulADValue[ADC_IDX_0] <= 4096) && (ADC_ptGetInfo()->ulADValue[ADC_IDX_0] > 3723))
    {
        Display_tInfo.Speed_Level = 10;
    }

}
static void Display_Error(void)
{
    ;

}
static void DisplayErrorHandler(void)
{
    IIC_LCD1602_Read_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(Compare_buff_1), (uint8_t*)Compare_buff_1);
    IIC_LCD1602_Read_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(Compare_buff_2), (uint8_t*)Compare_buff_2);
    if ((memcmp(Compare_buff_1, table2, sizeof(table2)-1) == 0)||\
       (memcmp(Compare_buff_1, table4, 5) == 0)||\
       (memcmp(Compare_buff_1, table1, sizeof(table1)-1) == 0)||\
	   (memcmp(Compare_buff_1, table3, sizeof(table3)) == 0))
    {
        ;
    }
    else
    {
				Display_tInfo.tDisplayTaskState = DISPLAY_TASK_INIT;
        
		}

}
TDisplayInfoDef* Display_tGetInfo(void)
{
    return  &Display_tInfo;
}
