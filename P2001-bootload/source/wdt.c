#include "wdt.h"
#include "hc32f46x_wdt.h"


/**
 *******************************************************************************
 ** \brief WDT configure
 **
 ** \param [in]  None
 **
 ** \retval None
 **
 ******************************************************************************/
void WDT_vConfig(void)
{
    stc_wdt_init_t stcWdtInit;

    /* configure structure initialization */
    MEM_ZERO_STRUCT(stcWdtInit);

    stcWdtInit.enClkDiv = WdtPclk3Div8192;   //!< 32Mhz / 8192
    stcWdtInit.enCountCycle = WdtCountCycle16384;        //!< 16384 * 8192 / 32000000 = 4.194s
    stcWdtInit.enRefreshRange = WdtRefresh0To100Pct;
    stcWdtInit.enSleepModeCountEn = Disable;
    stcWdtInit.enRequestType = WdtTriggerResetRequest;
    WDT_Init(&stcWdtInit);
}