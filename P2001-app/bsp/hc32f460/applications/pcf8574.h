#ifndef __PCF8574_H
#define __PCF8574_H

#include "myiic.h"




#define PCF8574_ADDR 	0X4E	//PCF8574地址(左移了一位)



uint8_t PCF8574_Init(void);
uint8_t PCF8574_ReadOneByte(void);
uint8_t PCF8574_WriteOneByte(uint8_t DataToWrite);
void PCF8574_WriteBit(uint8_t bit, uint8_t sta);
uint8_t PCF8574_ReadBit(uint8_t bit);
#endif

