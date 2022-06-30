#ifndef _TIMEA_H_
#define _TIMEA_H_
#include <stdint.h>
#include "hc32_common.h"







/* 吸风电机为PA9 TIMA_1_PWM_2  原理图有误*/
#define SUCTION_MOTOR_PWM						(M4_TMRA1)
#define SUCTION_MOTOR_PWM_CLOCK					(PWC_FCG2_PERIPH_TIMA1)
#define SUCTION_MOTOR_PWM_CH					(TimeraCh2)
#define SUCTION_MOTOR_PWM_PORT					(PortA)
#define SUCTION_MOTOR_PWM_PIN					(Pin09)
#define SUCTION_MOTOR_PWM_FUNC					(Func_Tima0)




/* 盘刷电机为PB3 TIMA_2_PWM_2  原理图有误*/
#define BRUSH_MOTOR_PWM							(M4_TMRA2)
#define BRUSH_MOTOR_PWM_CLOCK					(PWC_FCG2_PERIPH_TIMA2)
#define BRUSH_MOTOR_PWM_CH						(TimeraCh2)
#define BRUSH_MOTOR_PWM_PORT					(PortB)
#define BRUSH_MOTOR_PWM_PIN						(Pin03)
#define BRUSH_MOTOR_PWM_FUNC					(Func_Tima0)


/* RGB灯为PB7 TIMA_4_PWM_2 */
#define RGB_BLUE_PWM							(M4_TMRA4)
#define RGB_BLUE_PWM_CLOCK						(PWC_FCG2_PERIPH_TIMA4)
#define RGB_BLUE_PWM_CH							(TimeraCh2)
#define RGB_BLUE_PWM_PORT						(PortB)
#define RGB_BLUE_PWM_PIN						(Pin07)
#define RGB_BLUE_PWM_FUNC						(Func_Tima0)




#define TIMERA_PERVAL_VAL						 6400U
#define TIMERA_RGB_PERVAL_VAL					 200U

typedef enum 
{
	Brush_Motor = 1,
	Suction_Motor,
	RGB_BULE,
}Motor_Swicth;

extern void Timera_vInit(void);
extern void Timera_Motor_Contorl(Motor_Swicth motor, en_functional_state_t control, uint8_t percent);
extern void Timera_RGB_Contorl(en_functional_state_t control, uint16_t percent);
extern void TIMER0_vInit(void);

#endif // !_TIMEA_H_
