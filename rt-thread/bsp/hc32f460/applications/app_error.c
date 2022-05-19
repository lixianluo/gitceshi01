#include "app_error.h"
#include <rtthread.h>
#include "adc.h"


static uint8_t uiErrorCode = 0;

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
    ;
}

static void vErrorCodeAdd(ErrorCode codenum)
{
    uiErrorCode = uiErrorCode | (0x0001 << codenum);
}

static void vErrorCodeDel(ErrorCode codenum)
{
    uiErrorCode = uiErrorCode & (~(0x0001 << codenum));
}
