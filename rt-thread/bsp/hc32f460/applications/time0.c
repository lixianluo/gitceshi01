#include "time0.h"
#include "board.h"

void TIMER0_vInit(void)
{
    stc_tim0_base_init_t stcTimerCfg;
    stc_tim0_trigger_init_t StcTimer0TrigInit;

    MEM_ZERO_STRUCT(stcTimerCfg);
    MEM_ZERO_STRUCT(StcTimer0TrigInit);

    ///**Timer01 A set as uart1 timeout clock source--------------------------------------------------*/
    ///**Timer Unit 1 A, Timer Unit 1 B*/
    //PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM01, Enable);          //打开TIM01的时钟 TIM01A对应串口1   TIM01B对应串口2
    ///**Timer Unit 2 A, Timer Unit 2 B*/                           //打开TIM02的时钟 TIM02A对应串口3   TIM02B对应串口4
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM02, Enable);

    ///* Clear CNTAR register for channel A */
    //TIMER0_WriteCntReg(M4_TMR01, Tim0_ChannelA, 0u);

    ///* Config register for channel A */
    //stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    //stcTimerCfg.Tim0_AsyncClockSource = Tim0_LRC;
    //stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv0;
    //stcTimerCfg.Tim0_CmpValue = 8u;    //!< bigger is better, (1/32000) * 8 = 250us           //超时时间统一为250us
    //TIMER0_BaseInit(M4_TMR01, Tim0_ChannelA, &stcTimerCfg);

    ///* Clear compare flag */
    //TIMER0_ClearFlag(M4_TMR01, Tim0_ChannelA);

    ///* Config timer0 hardware trigger */
    //StcTimer0TrigInit.Tim0_InTrigEnable = false;
    //StcTimer0TrigInit.Tim0_InTrigClear = true;
    //StcTimer0TrigInit.Tim0_InTrigStart = true;
    //StcTimer0TrigInit.Tim0_InTrigStop = false;
    //TIMER0_HardTriggerInit(M4_TMR01, Tim0_ChannelA, &StcTimer0TrigInit);

    /**Timer01 B set as uart2 timeout clock source--------------------------------------------------*/

    /* Clear CNTAR register for channel A */
    //TIMER0_WriteCntReg(M4_TMR01, Tim0_ChannelB, 0u);

    ///* Config register for channel A */
    //stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    //stcTimerCfg.Tim0_AsyncClockSource = Tim0_LRC;
    //stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv0;
    //stcTimerCfg.Tim0_CmpValue = 8u;
    //TIMER0_BaseInit(M4_TMR01, Tim0_ChannelB, &stcTimerCfg);

    ///* Clear compare flag */
    //TIMER0_ClearFlag(M4_TMR01, Tim0_ChannelB);

    ///* Config timer0 hardware trigger */
    //StcTimer0TrigInit.Tim0_InTrigEnable = false;
    //StcTimer0TrigInit.Tim0_InTrigClear = true;
    //StcTimer0TrigInit.Tim0_InTrigStart = true;
    //StcTimer0TrigInit.Tim0_InTrigStop = false;
    //TIMER0_HardTriggerInit(M4_TMR01, Tim0_ChannelB, &StcTimer0TrigInit);


    /**Timer02 A set as uart3 timeout clock source--------------------------------------------------*/

    /* Clear CNTAR register for channel A */
    TIMER0_WriteCntReg(M4_TMR02, Tim0_ChannelA, 0u);

    /* Config register for channel A */
    stcTimerCfg.Tim0_CounterMode = Tim0_Async;
    stcTimerCfg.Tim0_AsyncClockSource = Tim0_LRC;
    stcTimerCfg.Tim0_ClockDivision = Tim0_ClkDiv0;
    stcTimerCfg.Tim0_CmpValue = 8u;
    TIMER0_BaseInit(M4_TMR02, Tim0_ChannelA, &stcTimerCfg);

    /* Clear compare flag */
    TIMER0_ClearFlag(M4_TMR02, Tim0_ChannelA);

    /* Config timer0 hardware trigger */
    StcTimer0TrigInit.Tim0_InTrigEnable = false;
    StcTimer0TrigInit.Tim0_InTrigClear = true;
    StcTimer0TrigInit.Tim0_InTrigStart = true;
    StcTimer0TrigInit.Tim0_InTrigStop = false;
    TIMER0_HardTriggerInit(M4_TMR02, Tim0_ChannelA, &StcTimer0TrigInit);

    ///**Timer02 B set as uart4 timeout clock source--------------------------------------------------*/
    ///* Clear CNTAR register for channel B */
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
}