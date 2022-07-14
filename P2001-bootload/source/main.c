/**
 *******************************************************************************
 * @file  main.c
 * @brief Main program template.
 @verbatim
   Change Logs:
   Date             Author          Notes
   2020-06-30       Zhangxl         First version
 @endverbatim
 *******************************************************************************
 * Copyright (C) 2016, Huada Semiconductor Co., Ltd. All rights reserved.
 *
 * This software is owned and published by:
 * Huada Semiconductor Co., Ltd. ("HDSC").
 *
 * BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
 * BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 *
 * This software contains source code for use with HDSC
 * components. This software is licensed by HDSC to be adapted only
 * for use in systems utilizing HDSC components. HDSC shall not be
 * responsible for misuse or illegal use of this software for devices not
 * supported herein. HDSC is providing this software "AS IS" and will
 * not be responsible for issues arising from incorrect user implementation
 * of the software.
 *
 * Disclaimer:
 * HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 * REGARDING THE SOFTWARE (INCLUDING ANY ACCOMPANYING WRITTEN MATERIALS),
 * ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
 * WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
 * WARRANTY OF NONINFRINGEMENT.
 * HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
 * LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
 * INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
 * SAVINGS OR PROFITS,
 * EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
 * INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
 * FROM, THE SOFTWARE.
 *
 * This software may be replicated in part or whole for the licensed use,
 * with the restriction that this Disclaimer and Copyright notice must be
 * included with each copy of this software, whether used in part or whole,
 * at all times.
 *******************************************************************************
 */

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "hc32_ddl.h"
#include "hc32f46x_sram.h"
#include "hc32f46x_clk.h"
#include "hc32f46x_efm.h"
#include "system_hc32f460keta.h"
#include "hc32f46x_utility.h"
#include "hc32f46x_gpio.h"
#include "hc32f46x_pwc.h"
#include "hc32f46x_crc.h"
#include "hc32f46x_icg.h"

#include "task.h"
#include "Timer/timer.h"
#include "sw_timer.h"
#include "GPIO/gpio.h"
#include "hc32f46x_wdt.h"
#include "wdt.h"
#include "CAN/can.h"
#include "flash.h"
#include "can_iap.h"
#include "UART/uart.h"
#include "ymodem_ota.h"
/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/
/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
 /**
 * @brief ICG parameters configuration
 */
 /* The ICG area is filled with F by default, HRC = 16MHZ,
    Please modify this value as required */
#if defined ( __GNUC__ ) && !defined (__CC_ARM) /* GNU Compiler */
const uint32_t u32ICG[] __attribute__((section(".icg_sec"))) =
#elif defined (__CC_ARM)
const uint32_t u32ICG[] __attribute__((at(0x400))) =
#elif defined (__ICCARM__)
__root const uint32_t u32ICG[] @ 0x400 =
#else
#error "unsupported compiler!!"
#endif
{
    /* ICG 0~ 3 */
    ICG0_REGISTER_CONSTANT,
    ICG1_REGISTER_CONSTANT,
    ICG2_REGISTER_CONSTANT,
    ICG3_REGISTER_CONSTANT,
    /* ICG 4~ 7 */
    ICG4_REGISTER_CONSTANT,
    ICG5_REGISTER_CONSTANT,
    ICG6_REGISTER_CONSTANT,
    ICG7_REGISTER_CONSTANT,
};


static void CrcConfig(void);
static void system_clock_init(void);

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

 /**
  *******************************************************************************
  ** \brief SysTick interrupt callback function.
  **
  ** \param None
  **
  ** \retval None
  **
  ******************************************************************************/
void SysTick_IrqHandler(void)
{
    SysTick_IncTick();
}

/**
 * @brief  Main function of template project
 * @param  None
 * @retval int32_t return value, if needed
 */
int32_t main(void)
{
    system_clock_init();
	SystemCoreClockUpdate();

    /**SysTick configuration */
    SysTick_Init(1000u);
    PORT_DebugPortSetting(TDO_SWO, Disable);
    PORT_DebugPortSetting(TRST, Disable);
    TIMER4_vInit(); //!< as software timer
    GPIO_vInit();

    TIMER0_vInit(); //!< used as uart timeout interrupt
    UART2_vInit();

    //MX_CAN_Init();

    CrcConfig();

    WDT_vConfig();
    /* First refresh to start WDT */
    WDT_RefreshCounter();
    /* add your code here */

    while(1)
    {
        TASK_vScheduleTasks();
        WDT_RefreshCounter();

        if (TMR_bIsTimeExpired(TMR_SW_WATCH_DOG))
        {
            /**OTA state*/
            if (ymodem_is_ota_data_received())
            {
                TMR_vSetTime(TMR_SW_WATCH_DOG, TMR_TIME_MS2TICKS(50));
            }
            else
            {
                TMR_vSetTime(TMR_SW_WATCH_DOG, TMR_TIME_MS2TICKS(200));
            }
            //PORT_Toggle(LED1_PORT, LED1_PIN);   //!< red
            PORT_Toggle(LED0_PORT, LED0_PIN);   //!< blue
        }
    }
}

/**
 *******************************************************************************
 ** \brief  CRC initial configuration.
 **
 ** \param  None.
 **
 ** \retval None.
 **
 ******************************************************************************/
static void CrcConfig(void)
{
    /* 1. Enable CRC. */
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_CRC, Enable);

    /* 2. Initializes CRC here or before every CRC calculation. */
    CRC_Init(CRC_SEL_32B | CRC_REFIN_ENABLE | CRC_REFOUT_ENABLE | CRC_XOROUT_ENABLE);
}


static void system_clock_init(void)
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


/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
