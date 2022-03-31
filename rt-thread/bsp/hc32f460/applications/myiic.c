#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "myiic.h"



/*IIC free*/
void IIC_Init(void)
{   
	SDA_OUT();
	IIC_SDA(1);
	IIC_SCL(1);	
}

//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();
	IIC_SDA(1);
	IIC_SCL(1);     //IIC free
	rt_hw_us_delay(5);
	IIC_SDA(0);
	rt_hw_us_delay(5);
	IIC_SCL(0);	
}	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	IIC_SCL(0);
	IIC_SDA(0);
	rt_hw_us_delay(5);
	IIC_SCL(1);
	rt_hw_us_delay(5);
	IIC_SDA(1);
	

}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime = 0;
	SDA_IN();      //SDA����Ϊ����
	IIC_SDA(1);
	rt_hw_us_delay(1);
	IIC_SCL(1);
	rt_hw_us_delay(1);
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL(0);//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void IIC_Ack(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(0); 
	rt_hw_us_delay(2);
	IIC_SCL(1);
	rt_hw_us_delay(2);
	IIC_SCL(0);
}
//������ACKӦ��		    
void IIC_NAck(void)
{
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(1);
	rt_hw_us_delay(2);
	IIC_SCL(1);
	rt_hw_us_delay(2);
	IIC_SCL(0);
}					 				     
//IIC����һ���ֽ�
		  
void IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	SDA_OUT(); 	    
    IIC_SCL(0) ;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        IIC_SDA((txd&0x80)>>7);
        txd<<=1; 	  
		rt_hw_us_delay(2);   //��TEA5767��������ʱ���Ǳ����
		IIC_SCL(1);
		rt_hw_us_delay(2);
		IIC_SCL(0);
		rt_hw_us_delay(2);
    }
	
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t IIC_Read_Byte(uint8_t ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
	{
        IIC_SCL(0);
		rt_hw_us_delay(2);
		IIC_SCL(1);
        receive<<=1;
        if(READ_SDA)receive++;   
		rt_hw_us_delay(2);
    }					 
    if (!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK   
    return receive;
}



