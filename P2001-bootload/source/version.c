#include "version.h"

//#if defined(__CC_ARM)
//static const uint8_t VER_ucString[] __attribute__((at(0x0803F800))) = {
//#elif defined(__IAR_SYSTEMS_ICC__)
//#pragma location = 0x0803F800 
//__root static const uint8_t VER_ucString[] = {
//#else
//static const uint8_t VER_ucString[] __attribute__((at(0x0803F800))) = {
//#endif
//	VER_STR_MATERIAL_NUMBER
//		VER_STR_SEPARATOR
//	VER_STR_CREATION_TIMESTAMP
//		VER_STR_SEPARATOR
//	VER_STR_HW_VERSION
//		VER_STR_SEPARATOR
//	VER_STR_SW_VERSION
//		VER_STR_SEPARATOR
//};


/******************************************************************************
* Put variables/functions at an absolute location in IAR
* 
* Case 1:
*		#pragma location = 0x0803F800 
*		__root static const uint8_t VER_ucString[] = {
*			//static const uint8_t VER_ucString[] __attribute__((at(0x0803F800))) = {
*			VER_STR_MATERIAL_NUMBER
*				VER_STR_SEPARATOR
*			VER_STR_CREATION_TIMESTAMP
*				VER_STR_SEPARATOR
*			VER_STR_HW_VERSION
*				VER_STR_SEPARATOR
*			VER_STR_SW_VERSION
*				VER_STR_SEPARATOR
*		};
* 
* Case 2:
*		__root const uint8_t VER_ucString[] @ 0x0803F900 = {
*			//static const uint8_t VER_ucString[] __attribute__((at(0x0803F800))) = {
*			VER_STR_MATERIAL_NUMBER
*				VER_STR_SEPARATOR
*			VER_STR_CREATION_TIMESTAMP
*				VER_STR_SEPARATOR
*			VER_STR_HW_VERSION
*				VER_STR_SEPARATOR
*			VER_STR_SW_VERSION
*				VER_STR_SEPARATOR
*		};
*
* Case 3:
*		__root const uint8_t ver_test @ 0x0803F800 = 0x55;
*
* Case 4: variables at user-defined sections
*		__root const uint8_t str[] @".USRINFO" = "Hello, sector test!";
* 
* Case 5: function at absolute address
*		__root static void ver_test(void) @".USRINFO"
*		{
*			while (1)
*			{
*
*			}
*		}
* 
* Case 6: place variables or functions in a specific section
*	#pragma location="timer1_area"
*	uint8_t timer1;
*	#pragma location="timer2_area"
*	uint8_t timer2;
*	#pragma location="timer2_area"
*	uint8_t timer4;
*
*	#pragma location="timer1_area"
*	uint8_t timer3;
*	#pragma location="timer1_area"
*	uint8_t timer6;
*
*	#pragma location="timer2_area"
*	uint8_t timer5;
********************************************************************************/

