#ifndef _TIMEA_H_
#define _TIMEA_H_
#include <stdint.h>
#include "hc32_common.h"
/* TIMERA_2 unit and clock definition */
#define BRUSH_MOTOR_PWM							(M4_TMRA2)
#define BRUSH_MOTOR_PWM_CLOCK					(PWC_FCG2_PERIPH_TIMA2)

/* TIMERA_2 channel 2 Port/Pin definition */
#define BRUSH_MOTOR_PWM_CH						(TimeraCh2)
#define BRUSH_MOTOR_PWM_PORT					(PortA)
#define BRUSH_MOTOR_PWM_PIN						(Pin01)
#define BRUSH_MOTOR_PWM_FUNC					(Func_Tima0)


/* TIMERA_6 unit and clock definition */
#define SUCTION_MOTOR_PWM						(M4_TMRA6)
#define SUCTION_MOTOR_PWM_CLOCK					(PWC_FCG2_PERIPH_TIMA6)

/* TIMERA_6 channel 7 Port/Pin definition */
#define SUCTION_MOTOR_PWM_CH					(TimeraCh7)
#define SUCTION_MOTOR_PWM_PORT					(PortB)
#define SUCTION_MOTOR_PWM_PIN					(Pin05)
#define SUCTION_MOTOR_PWM_FUNC					(Func_Tima1)

#define TIMERA_PERVAL_VAL						 6400U

typedef enum 
{
	Brush_Motor = 1,
	Suction_Motor,
}Motor_Swicth;

extern void Timera_vInit(void);
extern void Timera_Motor_Contorl(Motor_Swicth motor, en_functional_state_t control, uint8_t percent);


#endif // !_TIMEA_H_
