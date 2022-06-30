#include "timer.h"
#include "hc32f46x_timer0.h"
#include "hc32f46x_timer4_cnt.h"
#include "hc32f46x_timera.h"

//#include "hc32f460keta.h"
#include "hc32f46x_interrupts.h"
#include "hc32f46x_pwc.h"

#include "sw_timer.h"
#include "gpio.h"

/**
 *******************************************************************************
 ** \brief Zero match interrupt handler
 **
 ******************************************************************************/
static void Timer41ZeroMatchIrqCb(void)
{
    TMR_vTickUpdate();
    /* Clear Timer4-CNT zero interrupt flag */
    TIMER4_CNT_ClearIrqFlag(M4_TMR41, Timer4CntZeroMatchInt);
}


/**
 *******************************************************************************
 ** \brief Zero match interrupt handler
 **
 ******************************************************************************/
static void Timer42ZeroMatchIrqCb(void)
{
    //TMS_vUpdateOverFlowCount();
    /* Clear Timer4-CNT zero interrupt flag */
    TIMER4_CNT_ClearIrqFlag(M4_TMR42, Timer4CntZeroMatchInt);
}

void TIMER0_vInit(void)
{
    stc_tim0_base_init_t stcTimerCfg;
    stc_tim0_trigger_init_t StcTimer0TrigInit;

    MEM_ZERO_STRUCT(stcTimerCfg);
    MEM_ZERO_STRUCT(StcTimer0TrigInit);

    /**Timer Unit 1 A, Timer Unit 1 B*/
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM01, Enable);          //打开TIM01的时钟 TIM01A对应串口1   TIM01B对应串口2
    /**Timer Unit 2 A, Timer Unit 2 B*/                           //打开TIM02的时钟 TIM02A对应串口3   TIM02B对应串口4
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable);

    /**Timer01 A set as uart1 timeout clock source--------------------------------------------------*/

    /* Clear CNTAR register for channel A */
    /*TIMER0_WriteCntReg(M4_TMR01, Tim0_ChannelA, 0u);*/

    /* Config register for channel A */
    //stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    //stcTimerCfg.Tim0_AsyncClockSource = Tim0_LRC;
    //stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv0;
    //stcTimerCfg.Tim0_CmpValue = 8u;    //!< bigger is better, (1/32000) * 8 = 250us           //超时时间统一为250us
    //TIMER0_BaseInit(M4_TMR01, Tim0_ChannelA, &stcTimerCfg);

    /* Clear compare flag */
   /* TIMER0_ClearFlag(M4_TMR01, Tim0_ChannelA);*/

    /* Config timer0 hardware trigger */
    /*StcTimer0TrigInit.Tim0_InTrigEnable = false;
    StcTimer0TrigInit.Tim0_InTrigClear = true;
    StcTimer0TrigInit.Tim0_InTrigStart = true;
    StcTimer0TrigInit.Tim0_InTrigStop = false;
    TIMER0_HardTriggerInit(M4_TMR01, Tim0_ChannelA, &StcTimer0TrigInit);*/
    /*----------------------------------------------------------------------------------------------*/

    /**Timer01 B set as uart2 timeout clock source--------------------------------------------------*/

    /* Clear CNTAR register for channel A */
    TIMER0_WriteCntReg(M4_TMR01, Tim0_ChannelB, 0u);

    /* Config register for channel A */
    stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    stcTimerCfg.Tim0_AsyncClockSource = Tim0_LRC;
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv0;
    stcTimerCfg.Tim0_CmpValue = 8u;
    TIMER0_BaseInit(M4_TMR01, Tim0_ChannelB, &stcTimerCfg);

    /* Clear compare flag */
   /* TIMER0_ClearFlag(M4_TMR01, Tim0_ChannelB);*/

    /* Config timer0 hardware trigger */
    StcTimer0TrigInit.Tim0_InTrigEnable = false;
    StcTimer0TrigInit.Tim0_InTrigClear = true;
    StcTimer0TrigInit.Tim0_InTrigStart = true;
    StcTimer0TrigInit.Tim0_InTrigStop = false;
    TIMER0_HardTriggerInit(M4_TMR01, Tim0_ChannelB, &StcTimer0TrigInit);
    /*----------------------------------------------------------------------------------------------*/

    /**Timer02 A set as uart3 timeout clock source--------------------------------------------------*/

    /* Clear CNTAR register for channel A */
    //TIMER0_WriteCntReg(M4_TMR02, Tim0_ChannelA, 0u);

    ///* Config register for channel A */
    //stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    //stcTimerCfg.Tim0_AsyncClockSource = Tim0_LRC;
    //stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv0;
    //stcTimerCfg.Tim0_CmpValue = 8u;
    //TIMER0_BaseInit(M4_TMR02, Tim0_ChannelA, &stcTimerCfg);

    ///* Clear compare flag */
    //TIMER0_ClearFlag(M4_TMR02, Tim0_ChannelA);

    ///* Config timer0 hardware trigger */
    //StcTimer0TrigInit.Tim0_InTrigEnable = false;
    //StcTimer0TrigInit.Tim0_InTrigClear = true;
    //StcTimer0TrigInit.Tim0_InTrigStart = true;
    //StcTimer0TrigInit.Tim0_InTrigStop = false;
    //TIMER0_HardTriggerInit(M4_TMR02, Tim0_ChannelA, &StcTimer0TrigInit);
    /*----------------------------------------------------------------------------------------------*/

    /**Timer02 B set as uart4 timeout clock source--------------------------------------------------*/
    /* Clear CNTAR register for channel B */
    //TIMER0_WriteCntReg(M4_TMR02, Tim0_ChannelB, 0u);

    ///* Config register for channel B */
    //stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    //stcTimerCfg.Tim0_AsyncClockSource = Tim0_LRC;
    //stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv0;
    //stcTimerCfg.Tim0_CmpValue = 8u;
    //TIMER0_BaseInit(M4_TMR02, Tim0_ChannelB, &stcTimerCfg);

    ///* Clear compare flag */
    //TIMER0_ClearFlag(M4_TMR02, Tim0_ChannelB);

    ///* Config timer0 hardware trigger */
    //StcTimer0TrigInit.Tim0_InTrigEnable = false;
    //StcTimer0TrigInit.Tim0_InTrigClear = true;
    //StcTimer0TrigInit.Tim0_InTrigStart = true;
    //StcTimer0TrigInit.Tim0_InTrigStop = false;
    //TIMER0_HardTriggerInit(M4_TMR02, Tim0_ChannelB, &StcTimer0TrigInit);
    /*----------------------------------------------------------------------------------------------*/
}

void TIMER4_vInit(void)
{
    stc_irq_regi_conf_t stcIrqRegiCfg;
    stc_timer4_cnt_init_t stcCntInit;

    /* Clear structures */
    MEM_ZERO_STRUCT(stcCntInit);
    MEM_ZERO_STRUCT(stcIrqRegiCfg);

    /* Enable peripheral clock */
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM41, Enable);
    //PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM42, Enable);

    /**Unit1 for software timer generation*/

    /* Timer4-CNT : Initialize CNT configuration structure */
    stcCntInit.enBufferCmd = Disable;
    stcCntInit.enClk = Timer4CntPclk;
    stcCntInit.enClkDiv = Timer4CntPclkDiv64;    /* Timer4-CNT clock divide */
    stcCntInit.u16Cycle = 100 - 1;             /* Timer4-CNT cycle */
    stcCntInit.enCntMode = Timer4CntSawtoothWave;
    stcCntInit.enZeroIntCmd = Enable;    /* Enable zero match interrupt */
    stcCntInit.enPeakIntCmd = Disable;   /* Disable peak match interrupt */
    stcCntInit.enZeroIntMsk = Timer4CntIntMask0;
    stcCntInit.enPeakIntMsk = Timer4CntIntMask0;
    TIMER4_CNT_Init(M4_TMR41, &stcCntInit); /* Initialize Timer4-CNT */

    /**Unit2 for timestamp generation*/
    /* Timer4-CNT : Initialize CNT configuration structure */
    //stcCntInit.enBufferCmd = Disable;
    //stcCntInit.enClk = Timer4CntPclk;
    //stcCntInit.enClkDiv = Timer4CntPclkDiv64;    /* Timer4-CNT clock divide */
    //stcCntInit.u16Cycle = 50000 - 1;             /* Timer4-CNT cycle */
    //stcCntInit.enCntMode = Timer4CntSawtoothWave;
    //stcCntInit.enZeroIntCmd = Enable;    /* Enable zero match interrupt */
    //stcCntInit.enPeakIntCmd = Disable;   /* Disable peak match interrupt */
    //stcCntInit.enZeroIntMsk = Timer4CntIntMask0;
    //stcCntInit.enPeakIntMsk = Timer4CntIntMask0;
    //TIMER4_CNT_Init(M4_TMR42, &stcCntInit); /* Initialize Timer4-CNT */

    /* Set Timer4-CNT IRQ */
    stcIrqRegiCfg.enIRQn = Int005_IRQn;     //!< 
    stcIrqRegiCfg.pfnCallback = &Timer41ZeroMatchIrqCb;
    stcIrqRegiCfg.enIntSrc = INT_TMR41_GUDF;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_01);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);


    //stcIrqRegiCfg.enIRQn = Int006_IRQn;     //!< 
    //stcIrqRegiCfg.pfnCallback = &Timer42ZeroMatchIrqCb;
    //stcIrqRegiCfg.enIntSrc = INT_TMR42_GUDF;
    //enIrqRegistration(&stcIrqRegiCfg);
    //NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_02);
    //NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    //NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);


    /* Start Timer4-CNT */
    TIMER4_CNT_ClearCountVal(M4_TMR41);
    //TIMER4_CNT_ClearCountVal(M4_TMR42);

    TIMER4_CNT_Start(M4_TMR41);
    //TIMER4_CNT_Start(M4_TMR42);
}


