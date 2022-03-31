#ifndef	LCD_1602_H_
#define LCD_1602_H_

#include "stdint.h"


#define LCD_DDRAM_ADDR_LINE1_START		0x00
#define LCD_DDRAM_ADDR_LINE2_START		0x40


extern void IIC_LCD1602_Write_Init(void);
extern void IIC_LCD1602_Write_String(uint8_t ucAddr, uint8_t ucDatalen, uint8_t* pucBuf);
extern void IIC_LCD1602_Write_Number(uint8_t ucAddr, uint8_t num);
void IIC_LCD1602_Read_String(uint8_t ucAddr, uint8_t ucDatalen, uint8_t* pucBuf);
#endif // LCD_1602_H_

/*
PCF8574 P0 -> RS
		P1 -> RW
		P2 -> CS
		P3 -> ±³¹âµÆ
		P4 -> DB4
		P5 -> DB5
		P6 -> DB6
		P7 -> DB7

*/
