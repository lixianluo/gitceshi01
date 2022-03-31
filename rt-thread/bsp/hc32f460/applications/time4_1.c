#include "time4_1.h"
#include <rthw.h>
#include <rtthread.h>
#include "board.h"


void Time4_1_vInit(void)
{
    stc_timer4_cnt_init_t stcCntInit;
    stc_timer4_oco_init_t stcOcoInit;
    stc_timer4_pwm_init_t stcPwmInit;
    stc_oco_low_ch_compare_mode_t stcLowChCmpMode;
    stc_oco_high_ch_compare_mode_t stcHighChCmpMode;
    uint16_t u8OcoOccrVal = (TIMER4_1_CNT_CYCLE_VAL * 0.5);
    

    /* Clear structures */
    MEM_ZERO_STRUCT(stcCntInit);
    MEM_ZERO_STRUCT(stcOcoInit);
    MEM_ZERO_STRUCT(stcLowChCmpMode);
    MEM_ZERO_STRUCT(stcHighChCmpMode);
    MEM_ZERO_STRUCT(stcPwmInit);

    /* Enable peripheral clock */
    PWC_Fcg2PeriphClockCmd(PWC_FCG2_PERIPH_TIM41, Enable);

    /* Timer4 CNT : Initialize CNT configuration structure */
    stcCntInit.enBufferCmd = Disable;
    stcCntInit.enClk = Timer4CntPclk;
    stcCntInit.enClkDiv = Timer4CntPclkDiv1;  /* CNT clock divide */
    stcCntInit.u16Cycle = TIMER4_1_CNT_CYCLE_VAL;
    stcCntInit.enCntMode = Timer4CntSawtoothWave;
    stcCntInit.enZeroIntCmd = Disable;
    stcCntInit.enPeakIntCmd = Disable;
    stcCntInit.enZeroIntMsk = Timer4CntIntMask0;
    stcCntInit.enPeakIntMsk = Timer4CntIntMask0;
    TIMER4_CNT_Init(TIMER4_1_UNIT, &stcCntInit); /* Initialize CNT */

    /* Timer4 OCO : Initialize OCO configuration structure */
    stcOcoInit.enOccrBufMode = OccrBufDisable;
    stcOcoInit.enOcmrBufMode = OcmrBufDisable;
    stcOcoInit.enPortLevel = OcPortLevelLow;
    stcOcoInit.enOcoIntCmd = Disable;
    TIMER4_OCO_Init(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_V, &stcOcoInit); /* Initialize OCO high channel */
    TIMER4_OCO_Init(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_V, &stcOcoInit);         /* Initialize OCO low channel */

    /* Timer4 OCO : Initialize OCO configuration structure */
    stcOcoInit.enOccrBufMode = OccrBufDisable;
    stcOcoInit.enOcmrBufMode = OcmrBufDisable;
    stcOcoInit.enPortLevel = OcPortLevelLow;
    stcOcoInit.enOcoIntCmd = Disable;
    TIMER4_OCO_Init(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_W, &stcOcoInit); /* Initialize OCO high channel */
    TIMER4_OCO_Init(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_W, &stcOcoInit);         /* Initialize OCO low channel */





    if (!(TIMER4_1_OCO_HIGH_CH_V % 2))
    {
        /* ocmr[15:0] = 0x0FFF */
        stcHighChCmpMode.enCntZeroMatchOpState = OcoOpOutputHigh; //< b11~b10      11->01    
        stcHighChCmpMode.enCntZeroNotMatchOpState = OcoOpOutputLow;//< b15~b14       00->10
        stcHighChCmpMode.enCntUpCntMatchOpState = OcoOpOutputHigh;//< b9~b8        11->01 
        stcHighChCmpMode.enCntPeakMatchOpState = OcoOpOutputHigh;//< b7~b6         11->01
        stcHighChCmpMode.enCntPeakNotMatchOpState = OcoOpOutputLow;//< b13~b12       00->10
        stcHighChCmpMode.enCntDownCntMatchOpState = OcoOpOutputHigh;//< b5~b4      11->01

        stcHighChCmpMode.enCntZeroMatchOcfState = OcoOcfSet;
        stcHighChCmpMode.enCntUpCntMatchOcfState = OcoOcfSet;
        stcHighChCmpMode.enCntPeakMatchOcfState = OcoOcfSet;
        stcHighChCmpMode.enCntDownCntMatchOcfState = OcoOcfSet;

        stcHighChCmpMode.enMatchConditionExtendCmd = Disable;

        TIMER4_OCO_SetHighChCompareMode(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_V, &stcHighChCmpMode);  /* Set OCO high channel compare mode */

    }
    if (!(TIMER4_1_OCO_HIGH_CH_W % 2))
    {
        stcHighChCmpMode.enCntZeroMatchOpState = OcoOpOutputLow;     //< b11~b10
        stcHighChCmpMode.enCntZeroNotMatchOpState = OcoOpOutputHigh;    //< b15~b14
        stcHighChCmpMode.enCntUpCntMatchOpState = OcoOpOutputLow;      //< b9~b8
        stcHighChCmpMode.enCntPeakMatchOpState = OcoOpOutputLow;     //< b7~b6 
        stcHighChCmpMode.enCntPeakNotMatchOpState = OcoOpOutputHigh;       //< b13~b12 
        stcHighChCmpMode.enCntDownCntMatchOpState = OcoOpOutputLow;   //< b5~b4 

        stcHighChCmpMode.enCntZeroMatchOcfState = OcoOcfSet;
        stcHighChCmpMode.enCntUpCntMatchOcfState = OcoOcfSet;
        stcHighChCmpMode.enCntPeakMatchOcfState = OcoOcfSet;
        stcHighChCmpMode.enCntDownCntMatchOcfState = OcoOcfSet;

        stcHighChCmpMode.enMatchConditionExtendCmd = Disable;

        TIMER4_OCO_SetHighChCompareMode(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_W, &stcHighChCmpMode);  /* Set OCO high channel compare mode */

    }




    /*************Timer4 OCO ocmr1[31:0] = 0x0FF0 0FFF*****************************/
    if (TIMER4_1_OCO_LOW_CH_V % 2)
    {
        /* OCMR[31:0] Ox 0FF0 0FFF    0000 1111 1111 0000   0000 1111 1111 1111 此处11和00已全改为 11->10 00 ->01*/
        stcLowChCmpMode.enCntZeroLowMatchHighMatchLowChOpState = OcoOpOutputHigh;         /* bit[27:26]  11 */
        stcLowChCmpMode.enCntZeroLowMatchHighNotMatchLowChOpState = OcoOpOutputHigh;      /* bit[11:10]  11 */
        stcLowChCmpMode.enCntZeroLowNotMatchHighMatchLowChOpState = OcoOpOutputLow;         /* bit[31:30]  00 */
        stcLowChCmpMode.enCntZeroLowNotMatchHighNotMatchLowChOpState = OcoOpOutputLow;      /* bit[15:14]  00 */

        stcLowChCmpMode.enCntUpCntLowMatchHighMatchLowChOpState = OcoOpOutputHigh;        /* bit[25:24]  11 */
        stcLowChCmpMode.enCntUpCntLowMatchHighNotMatchLowChOpState = OcoOpOutputHigh;     /* bit[9:8]    11 */
        stcLowChCmpMode.enCntUpCntLowNotMatchHighMatchLowChOpState = OcoOpOutputLow;        /* bit[19:18]  00 */

        stcLowChCmpMode.enCntPeakLowMatchHighMatchLowChOpState = OcoOpOutputHigh;         /* bit[23:22]  11 */
        stcLowChCmpMode.enCntPeakLowMatchHighNotMatchLowChOpState = OcoOpOutputHigh;      /* bit[7:6]    11 */
        stcLowChCmpMode.enCntPeakLowNotMatchHighMatchLowChOpState = OcoOpOutputLow;         /* bit[29:28]  00 */
        stcLowChCmpMode.enCntPeakLowNotMatchHighNotMatchLowChOpState = OcoOpOutputLow;      /* bit[13:12]  00 */

        stcLowChCmpMode.enCntDownLowMatchHighMatchLowChOpState = OcoOpOutputHigh;         /* bit[21:20]  11 */
        stcLowChCmpMode.enCntDownLowMatchHighNotMatchLowChOpState = OcoOpOutputHigh;      /* bit[5:4]    11 */
        stcLowChCmpMode.enCntDownLowNotMatchHighMatchLowChOpState = OcoOpOutputLow;         /* bit[17:16]  00 */

        stcLowChCmpMode.enCntZeroMatchOcfState = OcoOcfSet;    /* bit[3] 1 */
        stcLowChCmpMode.enCntUpCntMatchOcfState = OcoOcfSet;   /* bit[2] 1 */
        stcLowChCmpMode.enCntPeakMatchOcfState = OcoOcfSet;    /* bit[1] 1 */
        stcLowChCmpMode.enCntDownCntMatchOcfState = OcoOcfSet; /* bit[0] 1 */

        TIMER4_OCO_SetLowChCompareMode(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_V, &stcLowChCmpMode);  /* Set OCO low channel compare mode */

    }
    if (TIMER4_1_OCO_LOW_CH_W % 2)
    {
        /* OCMR[31:0] Ox 0FF0 0FFF    0000 1111 1111 0000   0000 1111 1111 1111 */
        stcLowChCmpMode.enCntZeroLowMatchHighMatchLowChOpState = OcoOpOutputLow;         /* bit[27:26]  11 */
        stcLowChCmpMode.enCntZeroLowMatchHighNotMatchLowChOpState = OcoOpOutputLow;      /* bit[11:10]  11 */
        stcLowChCmpMode.enCntZeroLowNotMatchHighMatchLowChOpState = OcoOpOutputHigh;         /* bit[31:30]  00 */
        stcLowChCmpMode.enCntZeroLowNotMatchHighNotMatchLowChOpState = OcoOpOutputHigh;      /* bit[15:14]  00 */

        stcLowChCmpMode.enCntUpCntLowMatchHighMatchLowChOpState = OcoOpOutputLow;        /* bit[25:24]  11 */
        stcLowChCmpMode.enCntUpCntLowMatchHighNotMatchLowChOpState = OcoOpOutputLow;     /* bit[9:8]    11 */
        stcLowChCmpMode.enCntUpCntLowNotMatchHighMatchLowChOpState = OcoOpOutputHigh;        /* bit[19:18]  00 */

        stcLowChCmpMode.enCntPeakLowMatchHighMatchLowChOpState = OcoOpOutputLow;         /* bit[23:22]  11 */
        stcLowChCmpMode.enCntPeakLowMatchHighNotMatchLowChOpState = OcoOpOutputLow;      /* bit[7:6]    11 */
        stcLowChCmpMode.enCntPeakLowNotMatchHighMatchLowChOpState = OcoOpOutputHigh;         /* bit[29:28]  00 */
        stcLowChCmpMode.enCntPeakLowNotMatchHighNotMatchLowChOpState = OcoOpOutputHigh;      /* bit[13:12]  00 */

        stcLowChCmpMode.enCntDownLowMatchHighMatchLowChOpState = OcoOpOutputLow;         /* bit[21:20]  11 */
        stcLowChCmpMode.enCntDownLowMatchHighNotMatchLowChOpState = OcoOpOutputLow;      /* bit[5:4]    11 */
        stcLowChCmpMode.enCntDownLowNotMatchHighMatchLowChOpState = OcoOpOutputHigh;         /* bit[17:16]  00 */

        stcLowChCmpMode.enCntZeroMatchOcfState = OcoOcfSet;    /* bit[3] 1 */
        stcLowChCmpMode.enCntUpCntMatchOcfState = OcoOcfSet;   /* bit[2] 1 */
        stcLowChCmpMode.enCntPeakMatchOcfState = OcoOcfSet;    /* bit[1] 1 */
        stcLowChCmpMode.enCntDownCntMatchOcfState = OcoOcfSet; /* bit[0] 1 */


        TIMER4_OCO_SetLowChCompareMode(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_W, &stcLowChCmpMode);  /* Set OCO low channel compare mode */


    }

    /* Set OCO compare value */
    TIMER4_OCO_WriteOccr(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_V, u8OcoOccrVal);
    TIMER4_OCO_WriteOccr(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_V, u8OcoOccrVal);

    TIMER4_OCO_WriteOccr(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_W, u8OcoOccrVal);
    TIMER4_OCO_WriteOccr(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_W, u8OcoOccrVal);


    /* Enable OCO */
    TIMER4_OCO_OutputCompareCmd(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_V, Enable);
    TIMER4_OCO_OutputCompareCmd(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_V, Enable);

    TIMER4_OCO_OutputCompareCmd(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_W, Enable);
    TIMER4_OCO_OutputCompareCmd(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_W, Enable);

    /* Initialize PWM I/O */
    PORT_SetFunc(TIMER4_1_PWM_H_PORT, TIMER4_1_PWM_H_PIN, Func_Tim4, Disable);
    PORT_SetFunc(TIMER4_1_PWM_L_PORT, TIMER4_1_PWM_L_PIN, Func_Tim4, Disable);

    PORT_SetFunc(TIMER4_1_PWM_W_H_PORT, TIMER4_1_PWM_W_H_PIN, Func_Tim4, Disable);
    PORT_SetFunc(TIMER4_1_PWM_W_L_PORT, TIMER4_1_PWM_W_L_PIN, Func_Tim4, Disable);


    /* Timer4 PWM: Initialize PWM configuration structure */
    stcPwmInit.enRtIntMaskCmd = Enable;
    stcPwmInit.enClkDiv = PwmPlckDiv1;
    stcPwmInit.enOutputState = PwmHPwmLHold;
    stcPwmInit.enMode = PwmThroughMode;
    TIMER4_PWM_Init(TIMER4_1_UNIT, TIMER4_1_PWM_CH, &stcPwmInit); /* Initialize timer4 pwm */
    TIMER4_PWM_Init(TIMER4_1_UNIT, TIMER4_1_PWM_CH_W, &stcPwmInit);

    /* Clear && Start CNT */
    TIMER4_CNT_ClearCountVal(TIMER4_1_UNIT);
    TIMER4_CNT_Start(TIMER4_1_UNIT);
}

void Time4_1_Drive_Motor_Contorl( int16_t percent1) //输出范围(-1600-1600) 当到1600时 默认输出1440 全功率的95%
{
    uint16_t value = 0;
    if (percent1 > 0)
    {
     
        value = (percent1 <= 1520) ? percent1 + 1600 : 1520 + 1600;
    }
    else if (percent1 == 0)
    {
        value = 1600;
    }
    else if (percent1 < 0)
    {
      
        value = (percent1 >= -1520) ? 1600 + percent1 : 1600 - 1520;
    }
    //uint16_t vlaue = (percent < 2000) ? percent : 1000;                     
    uint16_t count = ((TIMER4_1_CNT_CYCLE_VAL / 3200) * value);
    //TIMER4_CNT_Stop(TIMER4_1_UNIT);
    TIMER4_OCO_WriteOccr(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_V, count);
    TIMER4_OCO_WriteOccr(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_V, count);
    TIMER4_OCO_WriteOccr(TIMER4_1_UNIT, TIMER4_1_OCO_HIGH_CH_W, count);
    TIMER4_OCO_WriteOccr(TIMER4_1_UNIT, TIMER4_1_OCO_LOW_CH_W, count);
    //TIMER4_CNT_ClearCountVal(TIMER4_1_UNIT);
	  //TIMER4_CNT_Start(TIMER4_1_UNIT);
}
