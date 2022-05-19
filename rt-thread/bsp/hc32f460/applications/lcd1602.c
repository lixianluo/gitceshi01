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
	rt_thread_mdelay(100);
	PCF8574_Init();				  //PCF8574初始化
	IIC_LCD1602_Write_Cmd(0x33);  //设置显示 4 线
	IIC_LCD1602_Write_Cmd(0x32);  //设置显示 4 线控制;
	IIC_LCD1602_Write_Cmd(0x28);  //设置显示 16*2方式 5*7点阵

	IIC_LCD1602_Write_Cmd(0x06);  //地址右移动，数据不变
	IIC_LCD1602_Write_Cmd(0x0C);  //开启显示，不显示光标
	IIC_LCD1602_Write_Cmd(0x01);  //清屏

	rt_thread_mdelay(100);
}

void IIC_LCD1602_Write_String(uint8_t ucAddr, uint8_t ucDatalen, uint8_t* pucBuf)
{
	IIC_LCD1602_Write_Cmd(ucAddr | 0x80);
	
	for (uint8_t i = 0; i < ucDatalen; i++)
	{
		if (*pucBuf == 0x00) { *pucBuf = 0x20; }
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


void IIC_LCD1602_Write_Number(uint8_t ucAddr, uint32_t num)
{
	uint8_t Num[] = {"0123456789"};
	IIC_LCD1602_Write_Cmd(ucAddr | 0x80);
	if(num < 10)									//1位
	{
		IIC_LCD1602_Write_Data(0x20);				//写入空格1
		IIC_LCD1602_Write_Data(0x20);				//写入空格2
		IIC_LCD1602_Write_Data(0x20);				//写入空格3
		IIC_LCD1602_Write_Data(0x20);				//写入空格4
		IIC_LCD1602_Write_Data(0x20);				//写入空格5
		IIC_LCD1602_Write_Data(0x20);				//写入空格6
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 100)							//2位
	{
		IIC_LCD1602_Write_Data(0x20);				//写入空格1
		IIC_LCD1602_Write_Data(0x20);				//写入空格2
		IIC_LCD1602_Write_Data(0x20);				//写入空格3
		IIC_LCD1602_Write_Data(0x20);				//写入空格4
		IIC_LCD1602_Write_Data(0x20);				//写入空格5
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 1000)							//3位
	{
		IIC_LCD1602_Write_Data(0x20);				//写入空格1
		IIC_LCD1602_Write_Data(0x20);				//写入空格2
		IIC_LCD1602_Write_Data(0x20);				//写入空格3
		IIC_LCD1602_Write_Data(0x20);				//写入空格4
		IIC_LCD1602_Write_Data(Num[num / 100]);
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 10000)							//4位
	{
		IIC_LCD1602_Write_Data(0x20);				//写入空格1
		IIC_LCD1602_Write_Data(0x20);				//写入空格2
		IIC_LCD1602_Write_Data(0x20);				//写入空格3
		IIC_LCD1602_Write_Data(Num[num / 1000]);
		IIC_LCD1602_Write_Data(Num[num % 1000 / 100]);
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 100000)							//5位
	{
		IIC_LCD1602_Write_Data(0x20);				//写入空格1
		IIC_LCD1602_Write_Data(0x20);				//写入空格2
		IIC_LCD1602_Write_Data(Num[num / 10000]);
		IIC_LCD1602_Write_Data(Num[num % 10000 / 1000]);
		IIC_LCD1602_Write_Data(Num[num % 1000 / 100]);
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 1000000)							//6位
	{
		IIC_LCD1602_Write_Data(0x20);				//写入空格1
		IIC_LCD1602_Write_Data(Num[num / 100000]);
		IIC_LCD1602_Write_Data(Num[num % 100000 / 10000]);
		IIC_LCD1602_Write_Data(Num[num % 10000 / 1000]);
		IIC_LCD1602_Write_Data(Num[num % 1000 / 100]);
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 10000000)							//7位
	{
		IIC_LCD1602_Write_Data(Num[num / 1000000]);
		IIC_LCD1602_Write_Data(Num[num % 1000000 / 100000]);
		IIC_LCD1602_Write_Data(Num[num % 100000 / 10000]);
		IIC_LCD1602_Write_Data(Num[num % 10000 / 1000]);
		IIC_LCD1602_Write_Data(Num[num % 1000 / 100]);
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
}


void IIC_LCD1602_Write_Number2(uint8_t ucAddr, uint8_t num)
{
	uint8_t Num[] = { "0123456789" };
	IIC_LCD1602_Write_Cmd(ucAddr | 0x80);

	IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
	IIC_LCD1602_Write_Data(Num[num % 10]);
	
}

/*
CGRAM可以存储8个自定义字符，他们在存储器中的首地址分别为:0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78.
把字模数据写入CGRAM，就需要从首地址中选一个，并将字模数据送入，切记要把CGRAM中的自定义字符再送入DDRAM， 这样自定义字符才能够正常显示。

以下CGRAM存储器中首地址所对应的索引 
0x00:第一个(0x40) 
0x01:第二个(0x48) 
0x02:第三个(0x50) 
0x03:第四个(0x58) 
0x04:第五个(0x60) 
0x05:第六个(0x68) 
0x06:第七个(0x70) 
0x07:第八个(0x78)
把CGRAM中的自定义字符再送入DDRAM ，只需将CGRAM存储器中首地址相对应的索引写入DDRAM即可。
*/



static const uint8_t User1[] = { 0x00,0x00,0x00,0x01,0x05,0x15,0x15,0x15 };//自定义WIFI字符
static const uint8_t User2[] = { 0x01,0x05,0x15,0x15,0x15,0x15,0x15,0x15 };//自定义WIFI字符

void IIC_LCE1602_WIFI_Init(void)
{
	IIC_LCD1602_Write_Cmd(0x40); //设定 CGRAM 地址
	for (uint8_t i = 0; i < 8; i++)
	{
		IIC_LCD1602_Write_Data(User1[i]); //写入自定义图形 
	}
	IIC_LCD1602_Write_Cmd(0x48); //设定 CGRAM 地址
	for (uint8_t j = 0; j < 8; j++)
	{
		IIC_LCD1602_Write_Data(User2[j]); //写入自定义图形 
	}
}
/*
显示WIFI的图标
*/
void IIC_LCE1602_WIFI_Display(void)
{
	IIC_LCD1602_Write_Cmd(0x8E); //设定屏幕上的显示位置
	IIC_LCD1602_Write_Data(0x00); //从 CGRAM 里取出自定义图形显示
	IIC_LCD1602_Write_Cmd(0x8F); //设定屏幕上的显示位置
	IIC_LCD1602_Write_Data(0x01); //从 CGRAM 里取出自定义图形显示
}

void IIC_LCE1602_WIFI_Not_Display(void)
{
	IIC_LCD1602_Write_Cmd(0x8E); //设定屏幕上的显示位置
	IIC_LCD1602_Write_Data(0x20); //写入空
	IIC_LCD1602_Write_Cmd(0x8F); //设定屏幕上的显示位置
	IIC_LCD1602_Write_Data(0x20); //写入空
}
