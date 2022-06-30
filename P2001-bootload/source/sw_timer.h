#ifndef __SWTIMER_H_
#define __SWTIMER_H_

#include <stdint.h>



//TMR16 timing array
typedef enum _TSWTimer16Index{
	TMR_SW_WATCH_DOG,
	TMR_CAN_TX_TIMEOUT,
	TMR_IAP_POLLING,
	TMR_FOTA_POLLING,
	TMR_YMODEM_POLLING,
	TMR_TEST,
	TMR_NUM
}TSWTimerIndex;


/* PUBLIC DECLARATIONS *********************************************************************************************/
#define TMR_TIME_BASE			100		//!<  base time, us
#define TMR_TIME_US2TICKS(x)	((x) / TMR_TIME_BASE)	//!< convert time from millisecond to real time ticks
#define TMR_TIME_MS2TICKS(x)	((x) * (1000 / TMR_TIME_BASE))	//!< convert time from millisecond to real time ticks
#define TMR_TIME_SEC2TICKS(x)   ((x) * (1000000 / TMR_TIME_BASE))	//!< convert time from millisecond to real time ticks
#define TMR_TIME_MIN2TICKS(x)   ((x) * (60000000/ TMR_TIME_BASE))	//!< convert time from millisecond to real time ticks
//extern void TMR_vTimer3Init(void);										//no this function

extern void TMR_vTickUpdate(void);										//Timer interrupt handler function
extern uint8_t TMR_bIsTimeExpired(TSWTimerIndex tTimerIndex);			//check timer16 item is over or not
extern void TMR_vSetTime(TSWTimerIndex tTimerIndex, uint32_t ulValue);	//set timer16 time value
extern uint32_t TMR_ulGetTimerValue(TSWTimerIndex tTimerIndex);			//get timer16 current value





























#endif
