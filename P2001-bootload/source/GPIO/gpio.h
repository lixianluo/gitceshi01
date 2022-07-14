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


/* ����----------------------------------------------------------------------------*/
#define SWITCH_B3_PORT						(PortC)		//��ת����B3����
#define SWITCH_B3_PIN						(Pin09)		//��ת����B3����

#define SWITCH_A2_PORT						(PortC)		//��ת����A2����
#define SWITCH_A2_PIN						(Pin08)		//��ת����A2����

#define SWITCH_A1_PORT						(PortC)		//��ת����A1����
#define SWITCH_A1_PIN						(Pin07)		//��ת����A1����

#define HAND_PORT_1							(PortA)		//���ֿ���1
#define HAND_PIN_1							(Pin01)		//���ֿ���1


#define HAND_PORT_2							(PortA)		//���ֿ���2
#define HAND_PIN_2							(Pin03)		//���ֿ���2


extern void GPIO_vInit(void);




























#endif
