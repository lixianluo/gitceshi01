#include "pcf8574.h"
#include "board.h"
	
uint8_t PCF8574_Init(void)
{
    uint8_t temp = 1;
    IIC_Stop();
	IIC_Init();
    IIC_Start();
    IIC_Send_Byte(PCF8574_ADDR|0X00);   //����������ַ0X4E,д���� 
    temp = IIC_Wait_Ack();
    IIC_Stop();
    return temp;
}


//��ȡPCF8574��8λIOֵ
//����ֵ:����������
uint8_t PCF8574_ReadOneByte(void)
{				  
	uint8_t temp=0;		  	    																 
    IIC_Start();    	 	   
	IIC_Send_Byte(PCF8574_ADDR|0X01);   //�������ģʽ			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();							//����һ��ֹͣ����	    
	return temp;
}
//��PCF8574д��8λIOֵ  
//DataToWrite:Ҫд�������
uint8_t PCF8574_WriteOneByte(uint8_t DataToWrite)
{				   
    uint8_t temp =1;
    uint8_t temp1 = 1;
    IIC_Start();  
    IIC_Send_Byte(PCF8574_ADDR|0X00);   //����������ַ0X4E,д���� 	 
    temp =	IIC_Wait_Ack();	    										  		   
	IIC_Send_Byte(DataToWrite);    	 	//�����ֽ�							   
    temp1 = IIC_Wait_Ack();
    IIC_Stop();							//����һ��ֹͣ���� 
    if ((temp == 0) && (temp1 == 0))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

//����PCF8574ĳ��IO�ĸߵ͵�ƽ
//bit:Ҫ���õ�IO���,0~7
//sta:IO��״̬;0��1
void PCF8574_WriteBit(uint8_t bit, uint8_t sta)
{
    uint8_t data;
    data=PCF8574_ReadOneByte(); //�ȶ���ԭ��������
    if(sta==0)data&=~(1<<bit);     
    else data|=1<<bit;
    PCF8574_WriteOneByte(data); //д���µ�����
}

//��ȡPCF8574��ĳ��IO��ֵ
//bit��Ҫ��ȡ��IO���,0~7
//����ֵ:��IO��ֵ,0��1
uint8_t PCF8574_ReadBit(uint8_t bit)
{
    uint8_t data;
    data=PCF8574_ReadOneByte(); //�ȶ�ȡ���8λIO��ֵ 
    if(data&(1<<bit))return 1;
    else return 0;   
}  
