#ifndef _MYIIC_H
#define _MYIIC_H
#include "gpio.h"
#include "hc32f460_gpio.h"






//IO��������
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


//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(uint8_t txd);	//IIC����һ���ֽ�
uint8_t IIC_Read_Byte(uint8_t ack);	//IIC��ȡһ���ֽ�
uint8_t IIC_Wait_Ack(void); 		//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�


#endif

