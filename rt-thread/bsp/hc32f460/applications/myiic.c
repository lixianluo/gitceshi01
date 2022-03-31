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

//产生IIC起始信号
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
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL(0);
	IIC_SDA(0);
	rt_hw_us_delay(5);
	IIC_SCL(1);
	rt_hw_us_delay(5);
	IIC_SDA(1);
	

}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime = 0;
	SDA_IN();      //SDA设置为输入
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
	IIC_SCL(0);//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
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
//不产生ACK应答		    
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
//IIC发送一个字节
		  
void IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	SDA_OUT(); 	    
    IIC_SCL(0) ;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        IIC_SDA((txd&0x80)>>7);
        txd<<=1; 	  
		rt_hw_us_delay(2);   //对TEA5767这三个延时都是必须的
		IIC_SCL(1);
		rt_hw_us_delay(2);
		IIC_SCL(0);
		rt_hw_us_delay(2);
    }
	
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t IIC_Read_Byte(uint8_t ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
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
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}



