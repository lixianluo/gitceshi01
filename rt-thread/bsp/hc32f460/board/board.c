/*
 * Copyright (C) 2021, lizhengyang
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author          Notes
 * 2021-09-02      lizhengyang     first version
 */
#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "gpio.h"
#include "timea.h"
#include "adc.h"
#include "wdt.h"
#include "time0.h"
#include "usart.h"


void rt_os_tick_callback(void)
{
    rt_interrupt_enter();

    rt_tick_increase();

    rt_interrupt_leave();
}
void SysClkConfig(void)
{
    en_clk_sys_source_t       enSysClkSrc;
    stc_clk_sysclk_cfg_t      stcSysClkCfg;
    stc_clk_mpll_cfg_t        stcMpllCfg;

    MEM_ZERO_STRUCT(enSysClkSrc);
    MEM_ZERO_STRUCT(stcSysClkCfg);
    MEM_ZERO_STRUCT(stcMpllCfg);

    /* Set bus clk div. */
    stcSysClkCfg.enHclkDiv = ClkSysclkDiv1;   // 128MHz
    stcSysClkCfg.enExclkDiv = ClkSysclkDiv2;  // 64MHz
    stcSysClkCfg.enPclk0Div = ClkSysclkDiv1;  // 128MHz
    stcSysClkCfg.enPclk1Div = ClkSysclkDiv2;  // 64MHz
    stcSysClkCfg.enPclk2Div = ClkSysclkDiv4;  // 32MHz
    stcSysClkCfg.enPclk3Div = ClkSysclkDiv4;  // 32MHz
    stcSysClkCfg.enPclk4Div = ClkSysclkDiv2;  // 64MHz
    CLK_SysClkConfig(&stcSysClkCfg);

    CLK_HrcCmd(Enable);       //Enable HRC

        /* MPLL config. */
    stcMpllCfg.pllmDiv = 2ul;   //HRC 16M / 2
    stcMpllCfg.plln = 32ul;  //8M*32 = 256M
    stcMpllCfg.PllpDiv = 2ul;   //MLLP = 128M
    stcMpllCfg.PllqDiv = 2ul;   //MLLQ = 128M
    stcMpllCfg.PllrDiv = 2ul;   //MLLR = 128M
    CLK_SetPllSource(ClkPllSrcHRC);
    CLK_MpllConfig(&stcMpllCfg);

    /* flash read wait cycle setting */
    EFM_Unlock();
    EFM_SetLatency(EFM_LATENCY_4);
    EFM_Lock();

    /* Enable MPLL. */
    CLK_MpllCmd(Enable);
    /* Wait MPLL ready. */
    while (Set != CLK_GetFlagStatus(ClkFlagMPLLRdy))
    {
        ;
    }

    /* Switch system clock source to MPLL. */
    CLK_SetSysClkSource(CLKSysSrcMPLL);
}
void SysTick_Handler(void)
{
    rt_os_tick_callback();
}

/**
 * This function will initial your board.
 */
void rt_hw_board_init(void)
{
    

    SysClkConfig();
    SysTick_Init(RT_TICK_PER_SECOND);
    
    /**disable Jtag*/
    PORT_DebugPortSetting(TDO_SWO, Disable);
    PORT_DebugPortSetting(TRST, Disable);
   	GPIO_vInit();
    TIMER0_vInit();
	Timera_vInit();
	Adc_vConfig();
    UART3_vInit();
    WDT_vConfig();
    /* Call components board initial (use INIT_BOARD_EXPORT()) */

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif
   

#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

}

void rt_hw_us_delay(rt_uint32_t us)
{
    uint32_t start, now, delta, reload, us_tick;
    start = SysTick->VAL;
    reload = SysTick->LOAD;
    us_tick = SystemCoreClock / 1000000UL;

    do
    {
        now = SysTick->VAL;
        delta = start > now ?  start - now : reload + start - now;
    }
    while (delta < us_tick * us);
}

