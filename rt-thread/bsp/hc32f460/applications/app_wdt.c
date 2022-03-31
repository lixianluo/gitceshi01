#include "app_wdt.h"
#include <rtthread.h>
#include "hc32f460_wdt.h"

static void WDT_vTaskHandler_Entry(void* parameter);
int WDT_iTaskHandler(void)
{
    rt_thread_t tid1;
    /* 创建WDT_vTaskHandler_entry程 */
    tid1 = rt_thread_create("WDT_vTaskHandler_entry",
        WDT_vTaskHandler_Entry,
        RT_NULL,
        512,
        8,
        20);
    /* 创建成功则启动线程 */
    if (tid1 != RT_NULL)rt_thread_startup(tid1);

    return 0;
}
INIT_APP_EXPORT(WDT_iTaskHandler);


static void WDT_vTaskHandler_Entry(void* parameter)
{
    while (1)
    {
        WDT_RefreshCounter();
        rt_thread_mdelay(30);
    }

}
