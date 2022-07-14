#ifndef _GPIO_H
#define _GPIO_H

#include <stdint.h>
#include "hc32f46x_gpio.h"


/* LED0 Port/Pin definition */
#define LED0_PORT        (PortC)
#define LED0_PIN         (Pin14)

/* LED1 Port/Pin definition */
#define LED1_PORT        (PortC)
#define LED1_PIN         (Pin15)


/* 输入----------------------------------------------------------------------------*/
#define SWITCH_B3_PORT						(PortC)		//旋转开关B3引脚
#define SWITCH_B3_PIN						(Pin09)		//旋转开关B3引脚

#define SWITCH_A2_PORT						(PortC)		//旋转开关A2引脚
#define SWITCH_A2_PIN						(Pin08)		//旋转开关A2引脚

#define SWITCH_A1_PORT						(PortC)		//旋转开关A1引脚
#define SWITCH_A1_PIN						(Pin07)		//旋转开关A1引脚

#define HAND_PORT_1							(PortA)		//把手开关1
#define HAND_PIN_1							(Pin01)		//把手开关1


#define HAND_PORT_2							(PortA)		//把手开关2
#define HAND_PIN_2							(Pin03)		//把手开关2


extern void GPIO_vInit(void);




























#endif
