#ifndef _GPIO_H_
#define _GPIO_H_


/* ���----------------------------------------------------------------------------*/
#define LED1_PORT							(PortC)
#define LED1_PIN							(Pin14)

#define LED2_PORT							(PortC)
#define LED2_PIN							(Pin15)

#define BLUE_TOGGLE()						(PORT_Toggle(LED1_PORT, LED1_PIN))
#define RED_TOGGLE()						(PORT_Toggle(LED2_PORT, LED2_PIN))



#define VALVE_PORT							(PortC)		//��ŷ�
#define VALVE_PIN							(Pin06)		//��ŷ�

#define POWEN_PORT							(PortA)		//��Դ��������
#define POWEN_PIN							(Pin08)		//��Դ��������

#define PWM_EN_PORT							(PortB)     //����ʹ�ܿ�
#define PWM_EN_PIN							(Pin04)		//����ʹ�ܿ�



/* ����----------------------------------------------------------------------------*/
#define BRUSH_START_PORT					(PortC)		//��ˢ����
#define BRUSH_START_PIN						(Pin08)		//��ˢ����

#define SUCTION_START_PORT					(PortC)		//��������
#define SUCTION_START_PIN					(Pin09)		//��������

#define ALL_START_PORT						(PortC)		//��ˢ + ��������
#define ALL_START_PIN						(Pin07)		//��ˢ + ��������

#define HAND_PORT							(PortA)		//���ֿ���
#define HAND_PIN							(Pin02)		//���ֿ���


/*ģ��ͨѶ-------------------------------------------------------------------------*/
#define IIC_SDA_PORT						(PortC)		//ģ��IIC_SDA
#define IIC_SDA_PIN							(Pin13)		//ģ��IIC_SDA

#define IIC_SCL_PORT						(PortH)		//ģ��IIC_SCL
#define IIC_SCL_PIN							(Pin02)		//ģ��IIC_SCL

extern void GPIO_vInit(void);

#endif
