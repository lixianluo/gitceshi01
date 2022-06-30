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
#include "app_error.h"

/*˽�б���-------------------------------------------------------*/
static uint8_t  Compare_buff_1[16] = {0};
static uint8_t  Compare_buff_2[16] = {0};
static DisplayTaskDef DisplayTaskState;
static uint32_t systick_100ms,systick_s, systick_min, systick_hrs;

/*������ʾ-------------------------------------------------------------------------*/
static const uint8_t company[] = { "Taung           " };
static const uint8_t version[] = { "P2001_0_0_V0.0.2" };
/*----------------------------------------------------------------------------------*/


/*������ʾ--------------------------------------------------------------------------*/
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
static const uint8_t time[] = { "OpHrs:" };
/*------------------------------------------------------------------------------------*/


/*�ػ���ʾ----------------------------------------------------------------------------*/
static const uint8_t close[] = { "Please wait ... " };
/*------------------------------------------------------------------------------------*/


/*���������ʾ------------------------------------------------------------------------*/
static const uint8_t error1[][17] =
{
    {"     Battery     "},{" Empty  Voltage  "},
    {"     Battery     "},{" Under  Voltage  "},
    {"     Battery     "},{" Over   Voltage  "},

    
    {"     Suction     "},{" Over   Current  "},
    {"     Branch      "},{" Over   Current  "},

    {"     Suction     "},{" Open   Circuit  "},
    {"     Branch      "},{" Open   Circuit  "},
};

/*------------------------------------------------------------------------------------*/


/*������ʾ----------------------------------------------------------------------------*/
static const uint8_t clear[16] = { 0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20,0x20 };//����д��
static const uint8_t clear_1[14] = { 0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 ,0x20 };//�͵�����˸
/*------------------------------------------------------------------------------------*/

/*˽�к���-------------------------------------------------------*/
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
    /* ����Display_vTaskHandler_entry�߳� */
    tid1 = rt_thread_create("Display_vTaskHandler_entry",
        Display_vTaskHandler_entry,
        RT_NULL,
        1024,
        6,
        20);
    /* �����ɹ��������߳� */
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
            case DISPLAY_TASK_INIT:                  //LCD1602��ʼ����ʾ
            {
                IIC_LCD1602_Write_Init();                   //1602д���ʼ��
                IIC_LCD1602_WIFI_Init();                    //1602wifi���ų�ʼ��
                TMR_vSetTime_100msValue(TMR_DISPLAY_START_VERSION, 15);
                TMR_vSetTime_100msValue(TMR_DISPLAY_START_LOGO, 30);
                TMR_vSetTime_100msValue(TMR_DISPLAY_WIFI, 100);
                DisplayTaskState.tTaskState = DISPLAY_TASK_INIT_DONE;
                break;
            }
            case DISPLAY_TASK_INIT_DONE:             //LCD1602��ʼ�����
            {
                break;
            }
            case DISPLAY_TASK_START_UP:             //LCD1602������ʾ
            {
                Display_Start_up();
                if (TMR_bIsTimeExpired(TMR_DISPLAY_START_LOGO))
                {
                    DisplayTaskState.tTaskState = DISPLAY_TASK_START_UP_DONE;
                }
                break;
            }
            case DISPLAY_TASK_START_UP_DONE:             //LCD1602������ʾ���           
            {
                break;	
            }
            case DISPLAY_TASK_RUN:                 //LCD1602������ʾ(����+�ۼ�ʱ��)
            {
                Display_Run();
                break;
            }
            case DISPLAY_TASK_ERROR:                //LCD1602���������ʾ
            {
                Display_Error();
                break;
            }
            case DISPLAY_TASK_CLOSE:                //LCD1602�ػ���ʾ
            {                
                Display_Close();
               
                break;
            }
            case DISPLAY_TASK_REBOOT:               //LCD1602����
            {
                IIC_LCD1602_Write_Init();                   //1602д���ʼ��
                IIC_LCD1602_WIFI_Init();                    //1602wifi���ų�ʼ��
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
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(clear), (uint8_t*)clear);  //����д���
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //����д���
}

static void Display_Start_up(void)
{
    if (!TMR_bIsTimeExpired(TMR_DISPLAY_START_VERSION))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(company) - 1, (uint8_t*)company); //����д��Taung

        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //����д���
    }
     
    else if (!TMR_bIsTimeExpired(TMR_DISPLAY_START_LOGO))
    {
        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(version) - 1, (uint8_t*)version); //����д��汾

        IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //����д���

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
        if (systick_100ms < 5)
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[2]), (uint8_t*)battery[2]);
        }
        else
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(clear_1), (uint8_t*)clear_1);
        }
        DisplayTaskState.batter_info = 2;
    }
    if ((ADC_ptGetInfo()->supply_voltage >= 22.2f) && (ADC_ptGetInfo()->supply_voltage < 22.4f))
    {
        if (systick_100ms < 5)
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[1]), (uint8_t*)battery[1]);
        }
        else
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(clear_1), (uint8_t*)clear_1);
        }
        DisplayTaskState.batter_info = 1;
    }
    if (ADC_ptGetInfo()->supply_voltage < 22.2f)
    {
        if (systick_100ms < 5)
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(battery[0]), (uint8_t*)battery[0]);
        }
        else
        {
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(clear_1), (uint8_t*)clear_1);
        }
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
static void Display_WIFI_Filp(void)	//500ms��500ms��
{
    if (systick_100ms < 5)			
    {
        IIC_LCD1602_WIFI_Display();
    }
    else
    {
        IIC_LCD1602_WIFI_Not_Display();
    }
}
static void Display_WIFI_ON(void)
{
    IIC_LCD1602_WIFI_Display();
}
static void Display_WIFI_OFF(void)
{
    IIC_LCD1602_WIFI_Not_Display();
}
static void Display_Error(void)
{
    static uint8_t i = 0;
    if (Error_GetInfo() >>(i/2)&0x01)	    //�ж��Ƿ��д�����
    {		
        if (TMR_bIsTimeExpired(TMR_DISPLAY_ERROR))
        {
            TMR_vSetTime_100msValue(TMR_DISPLAY_ERROR, 20);
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(error1[i]) - 1, (uint8_t*)error1[i]);
            i++;
            IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(error1[i]) - 1, (uint8_t*)error1[i]);
            i++;
        }
        if (i >= ERROR_NUM * 2) { i = 0; }
    }
    else
    {
        i++;
    }
}

static void Display_Close(void)
{
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(close) - 1, (uint8_t*)close);
    IIC_LCD1602_Write_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(clear), (uint8_t*)clear);  //����д���
    rt_thread_mdelay(1500);
    Display_Empty();
}
static void DisplayErrorHandler(void)
{
    IIC_LCD1602_Read_String(LCD_DDRAM_ADDR_LINE1_START, sizeof(Compare_buff_1), (uint8_t*)Compare_buff_1);
    IIC_LCD1602_Read_String(LCD_DDRAM_ADDR_LINE2_START, sizeof(Compare_buff_2), (uint8_t*)Compare_buff_2);
    switch (DisplayTaskState.tTaskState)
    {
        case DISPLAY_TASK_START_UP:
        {
             if ((memcmp(Compare_buff_1, company, 16) != 0) && \
                 (memcmp(Compare_buff_1, version, 16) != 0) && \
                 (memcmp(Compare_buff_1, clear, 16) != 0))
             {
                 DisplayTaskState.tTaskState = DISPLAY_TASK_REBOOT;
             }
             else
             {
                 return;
             }
            break;
        }
        case DISPLAY_TASK_RUN:
        {
            if ((memcmp(Compare_buff_1, battery[0], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[1], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[2], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[3], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[4], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[5], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[6], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[7], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[8], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[9], 14) != 0) && \
                (memcmp(Compare_buff_1, battery[10], 14) != 0) && \
                (memcmp(Compare_buff_1, clear, 14) != 0))
            {
                DisplayTaskState.tTaskState = DISPLAY_TASK_REBOOT;
            }
      
            else if ((memcmp(Compare_buff_2, time, 6) != 0) && \
                    (memcmp(Compare_buff_2, clear, 14) != 0))
            {
                DisplayTaskState.tTaskState = DISPLAY_TASK_REBOOT;
            }
            else
            {
                return;
            }

            break;
        }
        case DISPLAY_TASK_ERROR:
        {

            if ((memcmp(Compare_buff_1, error1[0], 16) != 0) && \
                (memcmp(Compare_buff_1, error1[2], 16) != 0) && \
                (memcmp(Compare_buff_1, clear, 14) != 0))
            {
                DisplayTaskState.tTaskState = DISPLAY_TASK_REBOOT;
            }

            else if ((memcmp(Compare_buff_2, error1[1], 16) != 0) && \
                     (memcmp(Compare_buff_2, error1[3], 16) != 0) && \
                     (memcmp(Compare_buff_2, clear, 14) != 0))
            {
                DisplayTaskState.tTaskState = DISPLAY_TASK_REBOOT;
            }
            else
            {
                return;
            }
            break;
        }
        default:break;
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
