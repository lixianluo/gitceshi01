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
	PCF8574_Init();				  //PCF8574��ʼ��
	IIC_LCD1602_Write_Cmd(0x33);  //������ʾ 4 ��
	IIC_LCD1602_Write_Cmd(0x32);  //������ʾ 4 �߿���;
	IIC_LCD1602_Write_Cmd(0x28);  //������ʾ 16*2��ʽ 5*7����

	IIC_LCD1602_Write_Cmd(0x06);  //��ַ���ƶ������ݲ���
	IIC_LCD1602_Write_Cmd(0x0C);  //������ʾ������ʾ���
	IIC_LCD1602_Write_Cmd(0x01);  //����

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
	if(num < 10)									//1λ
	{
		IIC_LCD1602_Write_Data(0x20);				//д��ո�1
		IIC_LCD1602_Write_Data(0x20);				//д��ո�2
		IIC_LCD1602_Write_Data(0x20);				//д��ո�3
		IIC_LCD1602_Write_Data(0x20);				//д��ո�4
		IIC_LCD1602_Write_Data(0x20);				//д��ո�5
		IIC_LCD1602_Write_Data(0x20);				//д��ո�6
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 100)							//2λ
	{
		IIC_LCD1602_Write_Data(0x20);				//д��ո�1
		IIC_LCD1602_Write_Data(0x20);				//д��ո�2
		IIC_LCD1602_Write_Data(0x20);				//д��ո�3
		IIC_LCD1602_Write_Data(0x20);				//д��ո�4
		IIC_LCD1602_Write_Data(0x20);				//д��ո�5
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 1000)							//3λ
	{
		IIC_LCD1602_Write_Data(0x20);				//д��ո�1
		IIC_LCD1602_Write_Data(0x20);				//д��ո�2
		IIC_LCD1602_Write_Data(0x20);				//д��ո�3
		IIC_LCD1602_Write_Data(0x20);				//д��ո�4
		IIC_LCD1602_Write_Data(Num[num / 100]);
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 10000)							//4λ
	{
		IIC_LCD1602_Write_Data(0x20);				//д��ո�1
		IIC_LCD1602_Write_Data(0x20);				//д��ո�2
		IIC_LCD1602_Write_Data(0x20);				//д��ո�3
		IIC_LCD1602_Write_Data(Num[num / 1000]);
		IIC_LCD1602_Write_Data(Num[num % 1000 / 100]);
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 100000)							//5λ
	{
		IIC_LCD1602_Write_Data(0x20);				//д��ո�1
		IIC_LCD1602_Write_Data(0x20);				//д��ո�2
		IIC_LCD1602_Write_Data(Num[num / 10000]);
		IIC_LCD1602_Write_Data(Num[num % 10000 / 1000]);
		IIC_LCD1602_Write_Data(Num[num % 1000 / 100]);
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 1000000)							//6λ
	{
		IIC_LCD1602_Write_Data(0x20);				//д��ո�1
		IIC_LCD1602_Write_Data(Num[num / 100000]);
		IIC_LCD1602_Write_Data(Num[num % 100000 / 10000]);
		IIC_LCD1602_Write_Data(Num[num % 10000 / 1000]);
		IIC_LCD1602_Write_Data(Num[num % 1000 / 100]);
		IIC_LCD1602_Write_Data(Num[num % 100 / 10]);
		IIC_LCD1602_Write_Data(Num[num % 10]);
	}
	else if (num < 10000000)							//7λ
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
CGRAM���Դ洢8���Զ����ַ��������ڴ洢���е��׵�ַ�ֱ�Ϊ:0x40,0x48,0x50,0x58,0x60,0x68,0x70,0x78.
����ģ����д��CGRAM������Ҫ���׵�ַ��ѡһ����������ģ�������룬�м�Ҫ��CGRAM�е��Զ����ַ�������DDRAM�� �����Զ����ַ����ܹ�������ʾ��

����CGRAM�洢�����׵�ַ����Ӧ������ 
0x00:��һ��(0x40) 
0x01:�ڶ���(0x48) 
0x02:������(0x50) 
0x03:���ĸ�(0x58) 
0x04:�����(0x60) 
0x05:������(0x68) 
0x06:���߸�(0x70) 
0x07:�ڰ˸�(0x78)
��CGRAM�е��Զ����ַ�������DDRAM ��ֻ�轫CGRAM�洢�����׵�ַ���Ӧ������д��DDRAM���ɡ�
*/



static const uint8_t User1[] = { 0x00,0x00,0x00,0x01,0x05,0x15,0x15,0x15 };//�Զ���WIFI�ַ�
static const uint8_t User2[] = { 0x01,0x05,0x15,0x15,0x15,0x15,0x15,0x15 };//�Զ���WIFI�ַ�

void IIC_LCD1602_WIFI_Init(void)
{
	IIC_LCD1602_Write_Cmd(0x40); //�趨 CGRAM ��ַ
	for (uint8_t i = 0; i < 8; i++)
	{
		IIC_LCD1602_Write_Data(User1[i]); //д���Զ���ͼ�� 
	}
	IIC_LCD1602_Write_Cmd(0x48); //�趨 CGRAM ��ַ
	for (uint8_t j = 0; j < 8; j++)
	{
		IIC_LCD1602_Write_Data(User2[j]); //д���Զ���ͼ�� 
	}
}
/*
��ʾWIFI��ͼ��
*/
void IIC_LCD1602_WIFI_Display(void)
{
	IIC_LCD1602_Write_Cmd(0x8E); //�趨��Ļ�ϵ���ʾλ��
	IIC_LCD1602_Write_Data(0x00); //�� CGRAM ��ȡ���Զ���ͼ����ʾ
	IIC_LCD1602_Write_Cmd(0x8F); //�趨��Ļ�ϵ���ʾλ��
	IIC_LCD1602_Write_Data(0x01); //�� CGRAM ��ȡ���Զ���ͼ����ʾ
}

void IIC_LCD1602_WIFI_Not_Display(void)
{
	IIC_LCD1602_Write_Cmd(0x8E); //�趨��Ļ�ϵ���ʾλ��
	IIC_LCD1602_Write_Data(0x20); //д���
	IIC_LCD1602_Write_Cmd(0x8F); //�趨��Ļ�ϵ���ʾλ��
	IIC_LCD1602_Write_Data(0x20); //д���
}
