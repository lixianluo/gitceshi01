#ifndef _TIME4_1_H_
#define _TIME4_1_H_

#include <stdint.h>


#define TIMER4_1_UNIT                     (M4_TMR41)
#define TIMER4_1_CNT_CYCLE_VAL            (6400u)        /* Timer4 counter cycle value */

/* Timer4 OCO */
#define TIMER4_1_OCO_HIGH_CH_V              (Timer4OcoOvh)  /* only Timer4OcoOuh  Timer4OcoOvh  Timer4OcoOwh */
#define TIMER4_1_OCO_LOW_CH_V				  (Timer4OcoOvl)
#define TIMER4_1_OCO_HIGH_CH_W              (Timer4OcoOwh)  /* only Timer4OcoOuh  Timer4OcoOvh  Timer4OcoOwh */
#define TIMER4_1_OCO_LOW_CH_W				  (Timer4OcoOwl)
/* Timer4 PWM */
#define TIMER4_1_PWM_CH                   (Timer4PwmV)    /* only Timer4PwmU  Timer4PwmV  Timer4PwmW */
#define TIMER4_1_PWM_CH_W                   (Timer4PwmW)    /* only Timer4PwmU  Timer4PwmV  Timer4PwmW */

/* Define port and pin for Timer4Pwm */
#define TIMER4_1_PWM_H_PORT               (PortA)         /* TIM4_1_OUH_B:PE9   TIM4_1_OVH_B:PE11   TIM4_1_OWH_B:PE13 */
#define TIMER4_1_PWM_H_PIN                (Pin09)
#define TIMER4_1_PWM_L_PORT               (PortB)         /* TIM4_1_OUL_B:PE8   TIM4_1_OVL_B:PE10   TIM4_1_OWL_B:PE12 */
#define TIMER4_1_PWM_L_PIN                (Pin14)

#define TIMER4_1_PWM_W_H_PORT               (PortA)         /* TIM4_1_OUH_B:PE9   TIM4_1_OVH_B:PE11   TIM4_1_OWH_B:PE13 */
#define TIMER4_1_PWM_W_H_PIN                (Pin10)
#define TIMER4_1_PWM_W_L_PORT               (PortB)         /* TIM4_1_OUL_B:PE8   TIM4_1_OVL_B:PE10   TIM4_1_OWL_B:PE12 */
#define TIMER4_1_PWM_W_L_PIN                (Pin15)

typedef enum {
	stop = 0,
	Forward,
	Backward
}For_Back;

extern void Time4_1_vInit(void);
extern void Time4_1_Drive_Motor_Contorl(int16_t percent); //Êä³ö·¶Î§(-1600-1600)


#endif // !_TIME4_1_H_
