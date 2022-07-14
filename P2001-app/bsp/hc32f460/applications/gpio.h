#ifndef _GPIO_H_
#define _GPIO_H_


/* ���----------------------------------------------------------------------------*/
#define BLUE_PORT							(PortC)		//ϵͳ����
#define BLUE_PIN							(Pin14)		//ϵͳ����

#define RED_PORT							(PortC)		//ϵͳ���
#define RED_PIN								(Pin15)		//ϵͳ���


/* ԭ��ͼRGB ����PB7 ���PB5 �̵�PB6  ԭ��ͼ������ �����Ϊ�� */
//#define RGB_BLUE_PORT						(PortB)		//RGB����
//#define RGB_BLUE_PIN						(Pin07)		//RGB����  

#define VALVE_PORT							(PortC)		//��ŷ�
#define VALVE_PIN							(Pin06)		//��ŷ�

#define POWEN_PORT							(PortA)		//��Դ��������
#define POWEN_PIN							(Pin08)		//��Դ��������

#define BRUSH_EN_PORT						(PortB)     //��ˢ���ʹ�ܿ�
#define BRUSH_EN_PIN						(Pin04)		//��ˢ����ܿ�

#define SUCTION_EN_PORT						(PortB)     //������ʹ�ܿ�
#define SUCTION_EN_PIN						(Pin12)		//�������ܿ�

/* ����----------------------------------------------------------------------------*/
#define SWITCH_B3_PORT						(PortC)		//��ת����B3����
#define SWITCH_B3_PIN						(Pin09)		//��ת����B3����

#define SWITCH_A2_PORT						(PortC)		//��ת����A2����
#define SWITCH_A2_PIN						(Pin08)		//��ת����A2����

#define SWITCH_A1_PORT						(PortC)		//��ת����A1����
#define SWITCH_A1_PIN						(Pin07)		//��ת����A1����

#define HAND_PORT_1							(PortA)		//���ֿ���1
#define HAND_PIN_1							(Pin01)		//���ֿ���1

#define HAND_PORT_2							(PortA)		//���ֿ���1
#define HAND_PIN_2							(Pin03)		//���ֿ���1

/*ģ��ͨѶ-------------------------------------------------------------------------*/
#define IIC_SDA_PORT						(PortC)		//ģ��IIC_SDA
#define IIC_SDA_PIN							(Pin13)		//ģ��IIC_SDA

#define IIC_SCL_PORT						(PortH)		//ģ��IIC_SCL
#define IIC_SCL_PIN							(Pin02)		//ģ��IIC_SCL


/*Ӧ��------------------------------------------------------------------------------*/
#define BLUE_TOGGLE()						(PORT_Toggle(BLUE_PORT, BLUE_PIN))

#define RED_TOGGLE()						(PORT_Toggle(RED_PORT, RED_PIN))

#define POW_ENABLE()						(PORT_SetBits(POWEN_PORT, POWEN_PIN))
#define	POW_DISABLE()						(PORT_ResetBits(POWEN_PORT, POWEN_PIN))

#define	BRUSH_ENABLE()						(PORT_SetBits(BRUSH_EN_PORT, BRUSH_EN_PIN))
#define	BRUSH_DISABLE()						(PORT_ResetBits(BRUSH_EN_PORT, BRUSH_EN_PIN))


#define	SUCTION_ENABLE()					(PORT_SetBits(SUCTION_EN_PORT, SUCTION_EN_PIN))
#define	SUCTION_DISABLE()					(PORT_ResetBits(SUCTION_EN_PORT, SUCTION_EN_PIN))

#define	VALVE_ENABLE()						(PORT_SetBits(VALVE_PORT, VALVE_PIN))
#define	VALVE_DISABLE()						(PORT_ResetBits(VALVE_PORT, VALVE_PIN))



/*----------------------------------------------------------------------------------*/








extern void GPIO_vInit(void);

#endif
