#include "sw_timer.h"


//Declaration of the module locale (static) variables
static volatile uint32_t TMR_ulTimer[TMR_NUM]  = {0};	


/* Timer interrupt handler function
 * time base = 10ms
 * */
void TMR_vTickUpdate(void)
{
	unsigned char ucIndex;

	for(ucIndex=0; ucIndex<TMR_NUM; ucIndex++)
	{
		if(TMR_ulTimer[ucIndex])	//counter > 0
		{
			TMR_ulTimer[ucIndex]--;
		}
	}
}

//check timer16 item is over or not
uint8_t TMR_bIsTimeExpired(TSWTimerIndex tTimerIndex)
{
	return (0 == TMR_ulTimer[tTimerIndex]);
}

//set timer16 time value
void TMR_vSetTime(TSWTimerIndex tTimerIndex, uint32_t ulValue)
{
	TMR_ulTimer[tTimerIndex] = ulValue;
}

//get timer16 current value
uint32_t TMR_ulGetTimerValue(TSWTimerIndex tTimerIndex)
{
	return TMR_ulTimer[tTimerIndex];
}
