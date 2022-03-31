#include "lcd1602.h"
#include "board.h"
#include "pcf8574.h"
#include "myiic.h"


void IIC_LCD1602_Write_Cmd(uint8_t cmd)
{
	PCF8574_WriteOneByte((cmd|0X0F) & 0xFC);  //0xfc :1111 1100 BG =1 , EN = 1, R/W = 0 , RS = 0
	PCF8574_WriteOneByte(0xF8);	//0xf8 :1111 1000 BG =1 , EN = 0, R/W = 0 , RS = 0
	PCF8574_WriteOneByte(((cmd<<4)|0X0F) & 0xFC);
	PCF8574_WriteOneByte(0xF8);
}


void IIC_LCD1602_Write_Data(uint8_t data)
{
	PCF8574_WriteOneByte((data|0X0F) & 0xFD);
	PCF8574_WriteOneByte(0xF9);
	PCF8574_WriteOneByte(((data<<4)|0X0F) & 0xFD);
	PCF8574_WriteOneByte(0xF8);
}




void IIC_LCD1602_Write_Init(void)
{
	rt_thread_mdelay(500);
	PCF8574_Init();				  //PCF8574初始化
	
	IIC_LCD1602_Write_Cmd(0x33);  //设置显示 4 线
	IIC_LCD1602_Write_Cmd(0x32);  //设置显示 4 线控制;
	IIC_LCD1602_Write_Cmd(0x28);  //设置显示 16*2方式 5*7点阵
	IIC_LCD1602_Write_Cmd(0x06);  //地址移动，数据不变
	IIC_LCD1602_Write_Cmd(0x0C);  //开启显示，不显示光标
	IIC_LCD1602_Write_Cmd(0x01);  //清屏
	//IIC_LCD1602_Write_Cmd(0x80);  //起始地址
	rt_thread_mdelay(100);
}

void IIC_LCD1602_Write_String(uint8_t ucAddr, uint8_t ucDatalen, uint8_t* pucBuf)
{
	IIC_LCD1602_Write_Cmd(ucAddr | 0x80);
	for (uint8_t i = 0; i < ucDatalen; i++)
	{
		IIC_LCD1602_Write_Data(*pucBuf++);
	}
}

void IIC_LCD1602_Read_String(uint8_t ucAddr, uint8_t ucDatalen, uint8_t* pucBuf)
{
	uint8_t  add,temp1,temp2;
	add = ucAddr|0x80;
	IIC_LCD1602_Write_Cmd(add);
	for (uint8_t i = 0; i < ucDatalen; i++)
	{
		PCF8574_WriteOneByte(0x08);
		PCF8574_WriteOneByte(0xFF);	
		temp1 = PCF8574_ReadOneByte();
		PCF8574_WriteOneByte(0x08);
		PCF8574_WriteOneByte(0xFF);
		temp2 = PCF8574_ReadOneByte();
		*pucBuf++ = (temp1&0xF0)|(temp2>>4);  
		
	}
	PCF8574_WriteOneByte(0xF8);
}


void IIC_LCD1602_Write_Number(uint8_t ucAddr, uint8_t num)
{
	uint8_t Num[] = {"0123456789"};
	IIC_LCD1602_Write_Cmd(ucAddr | 0x80);
	IIC_LCD1602_Write_Data(Num[num / 100]);
	IIC_LCD1602_Write_Data(Num[num % 100/10]);
	IIC_LCD1602_Write_Data(Num[num % 10]);
}


