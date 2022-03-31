#ifndef _GPIO_H_
#define _GPIO_H_

/*LED���Ŷ���*/
#define LED1_PORT							(PortC)
#define LED1_PIN							(Pin14)

#define LED2_PORT							(PortC)
#define LED2_PIN							(Pin15)

#define GREEN_TOGGLE()						(PORT_Toggle(LED1_PORT, LED1_PIN))
#define RED_TOGGLE()						(PORT_Toggle(LED2_PORT, LED2_PIN))

#define IIC_SDA_PORT						(PortC)		//ģ��IIC_SDA
#define IIC_SDA_PIN							(Pin13)		//ģ��IIC_SDA

#define IIC_SCL_PORT						(PortH)		//ģ��IIC_SCL
#define IIC_SCL_PIN							(Pin02)		//ģ��IIC_SCL



#define BRU_MOTOR_BRAKE_PORT				(PortA)		//��ˢ���ɲ��
#define BRU_MOTOR_BRAKE_PIN					(Pin00)

#define DRIVE_MOTOR_START_PORT				(PortC)		//���ֿ��أ�������������źţ�
#define DRIVE_MOTOR_START_PIN               (Pin08)

#define SUCTION_MOTOR_START_PORT			(PortC)		//������λ�������������źţ�
#define SUCTION_MOTOR_START_PIN             (Pin09)

#define RELAY_PORT							(PortC)		//�̵���
#define RELAY_PIN							(Pin02)


#define SOLENOID_PORT						(PortB)		//��ŷ�
#define SOLENOID_PIN						(Pin13)

#define FOR_BACK_PORT						(PortC)		//ǰ�����˰�ť������ת��
#define FOR_BACK_PIN						(Pin07)
	
#define BRU_MOTOR_START_PORT				(PortC)		//��ˢ��λ
#define BRU_MOTOR_START_PIN					(Pin06)


#define BRUSH_CURRENT_IT_PORT				(PortA)		//��ˢ��������ж�
#define BRUSH_CURRENT_IT_PIN				(Pin02)

#define SUCTION_CURRENT_IT_PORT				(PortB)		//�����������ж�
#define SUCTION_CURRENT_IT_PIN				(Pin09)

#define DRIVE_CURRENT_IT_PORT				(PortB)		//������������ж�
#define DRIVE_CURRENT_IT_PIN				(Pin12)

extern void GPIO_vInit(void);
extern void BRUSH_CURRENT_IT_Init(void);
#endif
