#ifndef _MYIIC_H
#define _MYIIC_H
#include "gpio.h"
#include "hc32f460_gpio.h"






//IO方向设置
#define SDA_IN()  {PORT_Unlock();\
					M4_PORT->PCRC13_f.POUTE = 0U;\
					PORT_Lock(); }


#define SDA_OUT()  {PORT_Unlock();\
					M4_PORT->PCRC13_f.POUTE = 1U;\
					PORT_Lock(); }



#define IIC_SDA(n) {PORT_Unlock();\
					M4_PORT->PCRC13_f.POUT = n;\
					PORT_Lock(); }

#define IIC_SCL(n) {PORT_Unlock();\
					M4_PORT->PCRH2_f.POUT = n;\
					PORT_Lock(); }

#define	READ_SDA   (M4_PORT->PCRC13_f.PIN)


//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);	//IIC发送一个字节
uint8_t IIC_Read_Byte(uint8_t ack);	//IIC读取一个字节
uint8_t IIC_Wait_Ack(void); 		//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号


#endif

