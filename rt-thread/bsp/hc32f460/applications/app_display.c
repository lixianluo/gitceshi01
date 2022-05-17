#include "app.display.h"
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
static uint8_t  Compare_buff_2[16] = { 0 };
static DisplayTaskDef DisplayTaskState;
static uint32_t systick_100ms,systick_s, systick_min, systick_hrs;


static uint32_t save_systick_hrs, save_systick_min;
static  rt_timer_t hundreds_millisecond_timer;

static const uint8_t company[] = { "Taung           " };
static const uint8_t clear[16] = { 0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20,0x20 };//清屏写入

static const uint8_t time[] = { "OpHrs:" };

static const uint8_t close[] = { "Please wait ... " };

static const uint8_t version[] = {"P2001_0_0_V0.0.1"};

static const uint8_t battery[11][12] =
{
    {0x45,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//E__________F
    {0x45,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//E*_________F
    {0x45,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//E**________F
    {0x45,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//E***_______F
    {0x45,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//E****______F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x5F,0x46 },//E*****_____F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x5F,0x46 },//E******____F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x5F,0x46 },//E*******___F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x5F,0x46 },//E********__F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x5F,0x46 },//E*********_F
    {0x45,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x46 },//E**********F 
};
/*---------------------------------------------------------------*/


/*私有函数-------------------------------------------------------*/
static void Display_vTaskHandler_entry(void* parameter);
static void Display_Init(void);
static void Display_Idle(void);
static void Display_UseTime(void);
static void Display_Empty(void);
static void Display_Error(void);
static void Display_WIFI(void);
static void DisplayErrorHandler(void);
static void Display_Close(void);

static void hundreds_millisecond_timeout(void* parameter);
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
   
    /*100ms定时器*/
    hundreds_millisecond_timer = rt_timer_create("hundreds_millisecond", hundreds_millisecond_timeout,
        RT_NULL, 10,
        RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
    if (hundreds_millisecond_timer != RT_NULL)rt_timer_start(hundreds_millisecond_timer);

    return 0;
}
INIT_APP_EXPORT(Display_iTaskHandler);

static void hundreds_millisecond_timeout(void* parameter)
{
    systick_100ms++;
    if (systick_100ms >= 10)systick_100ms = 0;
}
static void Display_vTaskHandler_entry(void* parameter)
{
    uint8_t off_signal = 0;
    rt_thread_mdelay(500);
    while (1)
    {
        systick_s = Main_ptGetInfo()->systick_s;
        systick_min = Main_ptGetInfo()->systick_min;
        systick_hrs = Main_ptGetInfo()->systick_hrs;
        off_signal = Key_ptGetInfo()->Off_key_Flag;
        switch (DisplayTaskState)
        {
            case DISPLAY_TASK_INIT:                  //LCD1602初始化显示
            {
                PORT_SetBits(PWM_EN_PORT, PWM_EN_PIN);	    //打开电机使能口
                PORT_SetBits(POWEN_PORT, POWEN_PIN);		//打开电源使能口
                IIC_LCD1602_Write_Init();                   //1602写入初始化
                IIC_LCE1602_WIFI_Init();                    //1602wifi符号初始化
                Display_Init();  
                DisplayTaskState = DISPLAY_TASK_IDLE;
				
                break;
            }
            case DISPLAY_TASK_IDLE:                 //LCD1602空闲显示(电量+累计时间)
            {
                if (Flash_ptGetInfo()->tTaskState == FLASH_TASK_READ_DONE)
                {
                    Display_Idle();
                }
                if (off_signal == 1)							//机器关机
                {
                    DisplayTaskState = DISPLAY_TASK_CLOSING;
                }
                break;
            }
            case DISPLAY_TASK_RUN_ERROR:            //LCD1602错误代码显示
            {

                break;
            }
            case DISPLAY_TASK_CLOSING:                //LCD1602关机显示
            {
                Display_Close();
                Flash_ptGetInfo()->tTaskState = FLASH_TASK_SAVE;
                DisplayTaskState = DISPLAY_TASK_CLOSE;
                break;
            }
            case DISPLAY_TASK_CLOSE:                //LCD1602关机显示
            {
                if (Flash_ptGetInfo()->tTaskState == FLASH_TASK_SAVE_DONE)
                {
                    rt_thread_mdelay(1500);
                    PORT_ResetBits(PWM_EN_PORT, PWM_EN_PIN);	    //断开电机使能口
                    PORT_ResetBits(POWEN_PORT, POWEN_PIN);		    //断开电源使能口
                }
                
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

static void Display_Init(void)
{
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(company)-1, (uint8_t*)company); //上行写入Taung
		
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //下行写入空

    rt_thread_mdelay(1500);

    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(version) - 1, (uint8_t*)version); //上行写入版本
    
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //下行写入空
    
    rt_thread_mdelay(1500);

    Display_Empty();
}
static void Display_Idle(void)
{
    //if ((Key_ptGetInfo()->Brush_key_Flag == 1) || (Key_ptGetInfo()->Suction_key_Flag == 1) ) //工作模式电量显示
    //{
        if (ADC_ptGetInfo()->supply_voltage >= 24.5f)
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[10]), (uint8_t*)battery[10]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 24.3f) && (ADC_ptGetInfo()->supply_voltage < 24.5f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[9]), (uint8_t*)battery[9]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 24.1f) && (ADC_ptGetInfo()->supply_voltage < 24.3f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[8]), (uint8_t*)battery[8]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.7f) && (ADC_ptGetInfo()->supply_voltage < 24.1f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[7]), (uint8_t*)battery[7]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.3f) && (ADC_ptGetInfo()->supply_voltage < 23.7f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[6]), (uint8_t*)battery[6]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 23.0f) && (ADC_ptGetInfo()->supply_voltage < 23.3f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[5]), (uint8_t*)battery[5]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 22.8f) && (ADC_ptGetInfo()->supply_voltage < 23.0f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[4]), (uint8_t*)battery[4]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 22.6f) && (ADC_ptGetInfo()->supply_voltage < 22.8f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[3]), (uint8_t*)battery[3]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 22.4f) && (ADC_ptGetInfo()->supply_voltage < 22.6f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[2]), (uint8_t*)battery[2]);
        }
        if ((ADC_ptGetInfo()->supply_voltage >= 22.2f) && (ADC_ptGetInfo()->supply_voltage < 22.4f))
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[1]), (uint8_t*)battery[1]);
        }
        if (ADC_ptGetInfo()->supply_voltage < 22.2f)
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[0]), (uint8_t*)battery[0]);
        }
			
    //}
    //else                                                                                                                   //待机模式电量显示
    //{
    //    if (ADC_ptGetInfo()->supply_voltage >= 24.9f)
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[10]), (uint8_t*)battery[10]);
    //    }
    //    if ((ADC_ptGetInfo()->supply_voltage >= 24.7f) && (ADC_ptGetInfo()->supply_voltage < 24.9f))
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[9]), (uint8_t*)battery[9]);
    //    }
    //    if ((ADC_ptGetInfo()->supply_voltage >= 24.5f) && (ADC_ptGetInfo()->supply_voltage < 24.7f))
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[8]), (uint8_t*)battery[8]);
    //    }
    //    if ((ADC_ptGetInfo()->supply_voltage >= 24.3f) && (ADC_ptGetInfo()->supply_voltage < 24.5f))
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[7]), (uint8_t*)battery[7]);
    //    }
    //    if ((ADC_ptGetInfo()->supply_voltage >= 24.1f) && (ADC_ptGetInfo()->supply_voltage < 24.3f))
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[6]), (uint8_t*)battery[6]);
    //    }
    //    if ((ADC_ptGetInfo()->supply_voltage >= 23.9f) && (ADC_ptGetInfo()->supply_voltage < 24.1f))
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[5]), (uint8_t*)battery[5]);
    //    }
    //    if ((ADC_ptGetInfo()->supply_voltage >= 23.7f) && (ADC_ptGetInfo()->supply_voltage < 23.9f))
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[4]), (uint8_t*)battery[4]);
    //    }
    //    if ((ADC_ptGetInfo()->supply_voltage >= 23.5f) && (ADC_ptGetInfo()->supply_voltage < 23.7f))
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[3]), (uint8_t*)battery[3]);
    //    }
    //    if ((ADC_ptGetInfo()->supply_voltage >= 23.3f) && (ADC_ptGetInfo()->supply_voltage < 23.5f))
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[2]), (uint8_t*)battery[2]);
    //    }
    //    if ((ADC_ptGetInfo()->supply_voltage >= 23.1f) && (ADC_ptGetInfo()->supply_voltage < 23.3f))
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[1]), (uint8_t*)battery[1]);
    //    }
    //    if (ADC_ptGetInfo()->supply_voltage < 23.1f)
    //    {
    //        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[0]), (uint8_t*)battery[0]);
    //    }
    //}
    Display_UseTime();

    Display_WIFI();
 
}

static void Display_UseTime(void)
{

    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(time)-1, (uint8_t*)time);

    IIC_LCD1602_Write_Number(LCD_DDRAM_ADDR_TIME_HRS_START, systick_hrs);
    
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_TIME_LIN_START, 1, (uint8_t*)&time[5]);

    IIC_LCD1602_Write_Number2(LCD_DDRAM_ADDR_TIME_MIN_START, systick_min);
}
static void Display_WIFI(void)	//500ms亮500ms灭
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
static void Display_Error(void)
{
    ;

}
static void Display_Close(void)
{
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(close) - 1, (uint8_t*)close);
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //下行写入空
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
		DisplayTaskState = DISPLAY_TASK_INIT;
        
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
