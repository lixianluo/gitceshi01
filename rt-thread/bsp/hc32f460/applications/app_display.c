#include "app_display.h"
#include "board.h"
#include <rtthread.h>
#include "lcd1602.h"
#include "adc.h"
#include "app_key.h"
#include "stdlib.h"
#include "main.h"
#include "flash.h"
#include "gpio.h"
#include "hc32f460_gpio.h"

/*私有变量-------------------------------------------------------*/
static uint8_t  Compare_buff_1[16] = {0};
static uint8_t  Compare_buff_2[16] = {0};
static DisplayTaskDef DisplayTaskState;
static uint32_t systick_100ms,systick_s, systick_min, systick_hrs;

static const uint8_t company[] = { "Taung           " };
static const uint8_t clear[16] = { 0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20,0x20 };//清屏写入

static const uint8_t time[] = { "OpHrs:" };

static const uint8_t close[] = { "Please wait ... " };

static const uint8_t version[] = {"P2001_0_0_V0.0.1"};

static const uint8_t battery[11][14] =
{
    {0x45,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46,0x20,0x20 },//E__________F
    {0x45,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46,0x20,0x20 },//E*_________F
    {0x45,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46,0x20,0x20 },//E**________F
    {0x45,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46,0x20,0x20 },//E***_______F
    {0x45,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46,0x20,0x20 },//E****______F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x46,0x20,0x20 },//E*****_____F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x46,0x20,0x20 },//E******____F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x46,0x20,0x20 },//E*******___F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x46,0x20,0x20 },//E********__F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x46,0x20,0x20 },//E*********_F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x46,0x20,0x20 },//E**********F 
};
/*---------------------------------------------------------------*/


/*私有函数-------------------------------------------------------*/
static void Display_vTaskHandler_entry(void* parameter);
static void Display_Start_up(void);
static void Display_Run(void);
static void Display_UseTime(void);
static void Display_Empty(void);
static void Display_Error(void);
static void DisplayErrorHandler(void);
static void Display_Close(void);
static void Display_WIFI_ON(void); 
static void Display_WIFI_OFF(void);
static void	Display_WIFI_Filp(void);
/*---------------------------------------------------------------*/
int Display_iTaskHandler(void)
{
    rt_thread_t tid1;
    /* 创建Display_vTaskHandler_entry线程 */
    tid1 = rt_thread_create("Display_vTaskHandler_entry",
        Display_vTaskHandler_entry,
        RT_NULL,
        1024,
        6,
        20);
    /* 创建成功则启动线程 */
    if (tid1 != RT_NULL)rt_thread_startup(tid1);
   
    
    return 0;
}
INIT_APP_EXPORT(Display_iTaskHandler);


static void Display_vTaskHandler_entry(void* parameter)
{
    while (1)
    {
        systick_100ms = Main_ptGetInfo()->systick_100ms;
        systick_s = Main_ptGetInfo()->systick_s;
        systick_min = Main_ptGetInfo()->systick_min;
        systick_hrs = Main_ptGetInfo()->systick_hrs;
     
        switch (DisplayTaskState.tTaskState)
        {
            case DISPLAY_TASK_INIT:                  //LCD1602初始化显示
            {
                IIC_LCD1602_Write_Init();                   //1602写入初始化
                IIC_LCE1602_WIFI_Init();                    //1602wifi符号初始化
                TMR_vSetTime_100msValue(TMR_DISPLAY_START_VERSION, 15);
                TMR_vSetTime_100msValue(TMR_DISPLAY_START_LOGO, 30);
                TMR_vSetTime_100msValue(TMR_DISPLAY_WIFI, 100);
                DisplayTaskState.tTaskState = DISPLAY_TASK_INIT_DONE;
                break;
            }
            case DISPLAY_TASK_INIT_DONE:             //LCD1602初始化完成
            {
                break;
            }
            case DISPLAY_TASK_START_UP:             //LCD1602开机显示
            {
                Display_Start_up();
                if (TMR_bIsTimeExpired(TMR_DISPLAY_START_LOGO))
                {
                    DisplayTaskState.tTaskState = DISPLAY_TASK_START_UP_DONE;
                }
                break;
            }
            case DISPLAY_TASK_START_UP_DONE:             //LCD1602开机显示完成           {
                break;
						{
							
            }
            case DISPLAY_TASK_RUN:                 //LCD1602空闲显示(电量+累计时间)
            {
                Display_Run();
                break;
            }
            case DISPLAY_TASK_ERROR:                //LCD1602错误代码显示
            {
                Display_Error();
                break;
            }
            case DISPLAY_TASK_CLOSE:                //LCD1602关机显示
            {                
                Display_Close();
               
                break;
            }
            case DISPLAY_TASK_REBOOT:               //LCD1602重启
            {
                IIC_LCD1602_Write_Init();                   //1602写入初始化
                IIC_LCE1602_WIFI_Init();                    //1602wifi符号初始化
                DisplayTaskState.tTaskState = DISPLAY_TASK_RUN;
                break;
            }              
            default:break;
        }
      if((systick_s % 3)==0)
	  {
	      DisplayErrorHandler();
	  }
      
	    rt_thread_mdelay(10);
    }
}


static void Display_Empty(void)
{
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(clear), (uint8_t*)clear);  //上行写入空
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //下行写入空
}

static void Display_Start_up(void)
{
    if (!TMR_bIsTimeExpired(TMR_DISPLAY_START_VERSION))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(company) - 1, (uint8_t*)company); //上行写入Taung

        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //下行写入空
    }
     
    else if (!TMR_bIsTimeExpired(TMR_DISPLAY_START_LOGO))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(version) - 1, (uint8_t*)version); //上行写入版本

        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //下行写入空

    }
}
static void Display_Run(void)
{
    
    if (ADC_ptGetInfo()->supply_voltage >= 24.5f)
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[10]), (uint8_t*)battery[10]);
        DisplayTaskState.batter_info = 10;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 24.3f) && (ADC_ptGetInfo()->supply_voltage < 24.5f))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[9]), (uint8_t*)battery[9]);
        DisplayTaskState.batter_info = 9;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 24.1f) && (ADC_ptGetInfo()->supply_voltage < 24.3f))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[8]), (uint8_t*)battery[8]);
        DisplayTaskState.batter_info = 8;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 23.7f) && (ADC_ptGetInfo()->supply_voltage < 24.1f))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[7]), (uint8_t*)battery[7]);
        DisplayTaskState.batter_info = 7;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 23.3f) && (ADC_ptGetInfo()->supply_voltage < 23.7f))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[6]), (uint8_t*)battery[6]);
        DisplayTaskState.batter_info = 6;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 23.0f) && (ADC_ptGetInfo()->supply_voltage < 23.3f))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[5]), (uint8_t*)battery[5]);
        DisplayTaskState.batter_info = 5;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 22.8f) && (ADC_ptGetInfo()->supply_voltage < 23.0f))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[4]), (uint8_t*)battery[4]);
        DisplayTaskState.batter_info = 4;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 22.6f) && (ADC_ptGetInfo()->supply_voltage < 22.8f))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[3]), (uint8_t*)battery[3]);
        DisplayTaskState.batter_info = 3;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 22.4f) && (ADC_ptGetInfo()->supply_voltage < 22.6f))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[2]), (uint8_t*)battery[2]);
        DisplayTaskState.batter_info = 2;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 22.2f) && (ADC_ptGetInfo()->supply_voltage < 22.4f))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[1]), (uint8_t*)battery[1]);
        DisplayTaskState.batter_info = 1;
    }
    if (ADC_ptGetInfo()->supply_voltage < 22.2f)
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[0]), (uint8_t*)battery[0]);
        DisplayTaskState.batter_info = 0;
    }
    Display_UseTime();

    if (DisplayTaskState.wifi_switch == 1)
    {
        Display_WIFI_ON();
    }
    else if (TMR_bIsTimeExpired(TMR_DISPLAY_WIFI))
    {
        Display_WIFI_OFF();
    }
    else
    {
        Display_WIFI_Filp();
    }
 
    
 
}

static void Display_UseTime(void)
{

    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(time)-1, (uint8_t*)time);

    IIC_LCD1602_Write_Number(LCD_DDRAM_ADDR_TIME_HRS_START, systick_hrs);
    
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_TIME_LIN_START, 1, (uint8_t*)&time[5]);

    IIC_LCD1602_Write_Number2(LCD_DDRAM_ADDR_TIME_MIN_START, systick_min);
}
static void Display_WIFI_Filp(void)	//500ms亮500ms灭
{
    if (systick_100ms < 5)			
    {
        IIC_LCE1602_WIFI_Display();

    }
    else
    {
        IIC_LCE1602_WIFI_Not_Display();
    }
}
static void Display_WIFI_ON(void)
{
    IIC_LCE1602_WIFI_Display();
}
static void Display_WIFI_OFF(void)
{
    IIC_LCE1602_WIFI_Not_Display();
}
static void Display_Error(void)
{
    ;

}
static void Display_Close(void)
{
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(close) - 1, (uint8_t*)close);
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //下行写入空
    rt_thread_mdelay(1500);
    Display_Empty();
}
static void DisplayErrorHandler(void)
{
    IIC_LCD1602_Read_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(Compare_buff_1), (uint8_t*)Compare_buff_1);
    IIC_LCD1602_Read_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(Compare_buff_2), (uint8_t*)Compare_buff_2);
    if ((memcmp(Compare_buff_1, company, 10) == 0)||\
       (memcmp(Compare_buff_1, battery, 1) == 0)||\
       (memcmp(Compare_buff_1, version, 10) == 0) ||\
       (memcmp(Compare_buff_1, close, 10) == 0) ||\
	   (memcmp(Compare_buff_1, clear, 5) == 0))
    {
        return;
    }
    else
    {
        
		DisplayTaskState.tTaskState = DISPLAY_TASK_INIT;
	}

}

uint32_t APP_Display_read_hrs_time(uint32_t ulData)
{
    return Main_ptGetInfo()->systick_hrs = ulData;
}
uint32_t APP_Display_read_min_time(uint32_t ulData)
{
    return Main_ptGetInfo()->systick_min = ulData;
}
uint32_t APP_Display_save_hrs_time(void)
{
    return Main_ptGetInfo()->systick_hrs;
}
uint32_t APP_Display_save_min_time(void)
{
    return Main_ptGetInfo()->systick_min;
}
DisplayTaskDef* Display_ptGetInfo(void)
{
    return &DisplayTaskState;
}
