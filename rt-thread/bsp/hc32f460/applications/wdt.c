#include "wdt.h"
#include "hc32f460_wdt.h"




void WDT_vConfig(void)
{
    stc_wdt_init_t stcWdtInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcWdtInit);

    stcWdtInit.enClkDiv = WdtPclk3Div1024;   //!< 32Mhz / 1024
    stcWdtInit.enCountCycle = WdtCountCycle4096;        //!< 4096 * 1024 / 32000000 = 131ms
    stcWdtInit.enRefreshRange = WdtRefresh0To100Pct;
    stcWdtInit.enSleepModeCountEn = Disable;
    stcWdtInit.enRequestType = WdtTriggerResetRequest;
    WDT_Init(&stcWdtInit);
    //WDT_RefreshCounter();
}
