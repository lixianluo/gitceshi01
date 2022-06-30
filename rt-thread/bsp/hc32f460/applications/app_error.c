#include "app_error.h"
#include <rtthread.h>
#include "adc.h"
#include "app_motor.h"


static uint32_t uiErrorCode = 0;

static void Error_TaskHandler(void);
static void vErrorCodeAdd(ErrorCode codenum);
static void vErrorCodeDel(ErrorCode codenum);

static void ERROR_vTaskHandler_Entry(void* parameter);
int ERROR_iTaskHandler(void)
{
    rt_thread_t tid1;
    /* 创建ERROR_vTaskHandler_entry程 */
    tid1 = rt_thread_create("ERROR_vTaskHandler_entry",
        ERROR_vTaskHandler_Entry,
        RT_NULL,
        1024,
        4,
        20);
    /* 创建成功则启动线程 */
    if (tid1 != RT_NULL)rt_thread_startup(tid1);

    return 0;
}
//INIT_APP_EXPORT(ERROR_iTaskHandler);


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
        /*增加错误代码------------------------*/
        vErrorCodeAdd(Battery_Under);
        /*------------------------------------*/

        /*删除错误代码------------------------*/
        vErrorCodeDel(Battery_Empty);
        vErrorCodeDel(Battery_Over);
        /*------------------------------------*/
    }
    if ((battery_voltage > 19.2f) && (battery_voltage < 22.0f))
    {
        /*增加错误代码------------------------*/
        vErrorCodeAdd(Battery_Empty);
        /*------------------------------------*/

        /*删除错误代码------------------------*/
        vErrorCodeDel(Battery_Under);
        vErrorCodeDel(Battery_Over);
        /*------------------------------------*/
    }
    if (battery_voltage > 30.5f)
    {
        /*增加错误代码------------------------*/
        vErrorCodeAdd(Battery_Over);
        /*------------------------------------*/

        /*删除错误代码------------------------*/
        vErrorCodeDel(Battery_Empty);
        vErrorCodeDel(Battery_Under);
        /*------------------------------------*/
    }
    if ((suction_speed > 150) && (suction_current < 2.0f))
    {
        /*增加错误代码------------------------*/
        vErrorCodeAdd(Suction_Open);
        /*------------------------------------*/

        /*删除错误代码------------------------*/
        vErrorCodeDel(Suction_Over_Current);
        /*------------------------------------*/
    }
    if ((suction_speed > 150) && (suction_current > 18.0f))
    {
        /*增加错误代码------------------------*/
        vErrorCodeAdd(Suction_Over_Current);
        /*------------------------------------*/

        /*删除错误代码------------------------*/
        vErrorCodeDel(Suction_Open);
        /*------------------------------------*/
    }
    if ((branch_speed > 150) && (branch_current < 2.0f))
    {
        /*增加错误代码------------------------*/
        vErrorCodeAdd(Branch_Open);
        /*------------------------------------*/

        /*删除错误代码------------------------*/
        vErrorCodeDel(Branch_Over_Current);
        /*------------------------------------*/
    }
    if ((branch_speed > 150) && (branch_current > 18.0f))
    {
        /*增加错误代码------------------------*/
        vErrorCodeAdd(Branch_Over_Current);
        /*------------------------------------*/

        /*删除错误代码------------------------*/
        vErrorCodeDel(Branch_Open);
        /*------------------------------------*/
    }
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
