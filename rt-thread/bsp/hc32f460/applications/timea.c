#include "timea.h"
#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "gpio.h"


void Timera_vInit(void)
{
    stc_timera_base_init_t      stcTimeraInit;
    stc_timera_compare_init_t   stcTimerCompareInit;


    /* configuration structure initialization */
    MEM_ZERO_STRUCT(stcTimeraInit);
    MEM_ZERO_STRUCT(stcTimerCompareInit);

    /* Configuration peripheral clock */
    PWC_Fcg2PeriphClockCmd(BRUSH_MOTOR_PWM_CLOCK, Enable);
    PWC_Fcg2PeriphClockCmd(SUCTION_MOTOR_PWM_CLOCK, Enable);
    PWC_Fcg0PeriphClockCmd(PWC_FCG0_PERIPH_AOS, Enable);

    /* Configuration TIMERA compare pin */
    PORT_SetFunc(BRUSH_MOTOR_PWM_PORT, BRUSH_MOTOR_PWM_PIN, BRUSH_MOTOR_PWM_FUNC, Disable);
    PORT_SetFunc(SUCTION_MOTOR_PWM_PORT, SUCTION_MOTOR_PWM_PIN, SUCTION_MOTOR_PWM_FUNC, Disable);

    /* Configuration timera  base structure */
    stcTimeraInit.enClkDiv = TimeraPclkDiv1;
    stcTimeraInit.enCntMode = TimeraCountModeSawtoothWave;
    stcTimeraInit.enCntDir = TimeraCountDirUp;
    stcTimeraInit.enSyncStartupEn = Disable;
    stcTimeraInit.u16PeriodVal = TIMERA_PERVAL_VAL;        //freq: 10KHz
    TIMERA_BaseInit(BRUSH_MOTOR_PWM, &stcTimeraInit);
    TIMERA_BaseInit(SUCTION_MOTOR_PWM, &stcTimeraInit);
    /* Configuration timera  compare structure */
    stcTimerCompareInit.u16CompareVal = 0;
    stcTimerCompareInit.enStartCountOutput = TimeraCountStartOutputHigh;
    stcTimerCompareInit.enStopCountOutput = TimeraCountStopOutputHigh;
    stcTimerCompareInit.enCompareMatchOutput = TimeraCompareMatchOutputLow;
    stcTimerCompareInit.enPeriodMatchOutput = TimeraPeriodMatchOutputHigh;
    stcTimerCompareInit.enSpecifyOutput = TimeraSpecifyOutputInvalid;
    stcTimerCompareInit.enCacheEn = Disable;
    stcTimerCompareInit.enTriangularTroughTransEn = Disable;
    stcTimerCompareInit.enTriangularCrestTransEn = Disable;
    stcTimerCompareInit.u16CompareCacheVal = 0;
    TIMERA_CompareInit(BRUSH_MOTOR_PWM, BRUSH_MOTOR_PWM_CH, &stcTimerCompareInit);
    
    /* Configure Channel  */  
    MEM_ZERO_STRUCT(stcTimerCompareInit);
    stcTimerCompareInit.u16CompareVal = 0;
    stcTimerCompareInit.enStartCountOutput = TimeraCountStartOutputLow;
    stcTimerCompareInit.enStopCountOutput = TimeraCountStopOutputLow;
    stcTimerCompareInit.enCompareMatchOutput = TimeraCompareMatchOutputHigh;
    stcTimerCompareInit.enPeriodMatchOutput = TimeraPeriodMatchOutputLow;
    stcTimerCompareInit.enSpecifyOutput = TimeraSpecifyOutputInvalid;
    stcTimerCompareInit.enCacheEn = Disable;
    stcTimerCompareInit.enTriangularTroughTransEn = Disable;
    stcTimerCompareInit.enTriangularCrestTransEn = Disable;
    stcTimerCompareInit.u16CompareCacheVal = 6400;
    TIMERA_CompareInit(SUCTION_MOTOR_PWM, SUCTION_MOTOR_PWM_CH, &stcTimerCompareInit);

    TIMERA_CompareCmd(SUCTION_MOTOR_PWM, SUCTION_MOTOR_PWM_CH, Disable);
    TIMERA_CompareCmd(BRUSH_MOTOR_PWM, BRUSH_MOTOR_PWM_CH, Disable);

    TIMERA_Cmd(BRUSH_MOTOR_PWM, Enable);
    TIMERA_Cmd(SUCTION_MOTOR_PWM, Enable);
}

void Timera_Motor_Contorl(Motor_Swicth motor, en_functional_state_t control ,uint8_t percent) //percentȡֵ0-200
{
    
    
    switch (motor)
    {
        case Brush_Motor:
        {
            uint16_t count1 = ((TIMERA_PERVAL_VAL / 200) * (200 - percent));  //盘刷电机 PWM为反相，PWM为0时则电机最高速
            TIMERA_SetCompareValue(BRUSH_MOTOR_PWM, BRUSH_MOTOR_PWM_CH, count1);
            TIMERA_CompareCmd(BRUSH_MOTOR_PWM, BRUSH_MOTOR_PWM_CH, control);
            /*if (control == Enable)
            {
                PORT_ResetBits(BRU_MOTOR_BRAKE_PORT, BRU_MOTOR_BRAKE_PIN);
            }
            else
            {
                PORT_SetBits(BRU_MOTOR_BRAKE_PORT, BRU_MOTOR_BRAKE_PIN);
            }*/
            break;
        }
        case Suction_Motor:
        {
            uint16_t count2 = ((TIMERA_PERVAL_VAL / 200) * (200 - percent)); //吸风电机 PWM为反相，PWM为0时则电机最高速
            TIMERA_SetCompareValue(SUCTION_MOTOR_PWM, SUCTION_MOTOR_PWM_CH, count2);
            TIMERA_CompareCmd(SUCTION_MOTOR_PWM, SUCTION_MOTOR_PWM_CH, control);
            break;
        }
        default:break;
    }
}

//void Suction_Motor_Contorl(en_functional_state_t control, uint8_t percent)
//{
//    uint16_t count1 = ((TIMERA_PERVAL_VAL / 200) * percent);
//}

     
