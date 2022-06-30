#ifndef _TIMER_H
#define _TIMER_H






#define TIMER_UNIT_ENCODER_MAIN		M4_TMRA2	//!< main wheel
#define TIMER_UNIT_ENCODER_LEFT		M4_TMRA5	//!< left wheel
#define TIMER_UNIT_ENCODER_RIGHT	M4_TMRA3	//!< right wheel








extern void TIMER0_vInit(void);
extern void TIMER4_vInit(void);





#endif
