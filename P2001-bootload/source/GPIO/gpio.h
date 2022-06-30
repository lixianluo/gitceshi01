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


//#define RS485_1_DE_PORT   (PortA)
//#define RS485_1_DE_PIN    (Pin10)
//
//#define RS485_2_DE_PORT   (PortC)
//#define RS485_2_DE_PIN    (Pin09)
//
//#define CAMERA_SYNC_PORT  (PortD)
//#define CAMERA_SYNC_PIN	  (Pin02)





extern void GPIO_vInit(void);




























#endif
