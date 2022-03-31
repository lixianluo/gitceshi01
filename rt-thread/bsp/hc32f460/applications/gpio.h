#ifndef _GPIO_H_
#define _GPIO_H_

/*LED引脚定义*/
#define LED1_PORT							(PortC)
#define LED1_PIN							(Pin14)

#define LED2_PORT							(PortC)
#define LED2_PIN							(Pin15)

#define GREEN_TOGGLE()						(PORT_Toggle(LED1_PORT, LED1_PIN))
#define RED_TOGGLE()						(PORT_Toggle(LED2_PORT, LED2_PIN))

#define IIC_SDA_PORT						(PortC)		//模拟IIC_SDA
#define IIC_SDA_PIN							(Pin13)		//模拟IIC_SDA

#define IIC_SCL_PORT						(PortH)		//模拟IIC_SCL
#define IIC_SCL_PIN							(Pin02)		//模拟IIC_SCL



#define BRU_MOTOR_BRAKE_PORT				(PortA)		//盘刷电机刹车
#define BRU_MOTOR_BRAKE_PIN					(Pin00)

#define DRIVE_MOTOR_START_PORT				(PortC)		//把手开关（驱动电机启动信号）
#define DRIVE_MOTOR_START_PIN               (Pin08)

#define SUCTION_MOTOR_START_PORT			(PortC)		//刮条限位（吸风电机启动信号）
#define SUCTION_MOTOR_START_PIN             (Pin09)

#define RELAY_PORT							(PortC)		//继电器
#define RELAY_PIN							(Pin02)


#define SOLENOID_PORT						(PortB)		//电磁阀
#define SOLENOID_PIN						(Pin13)

#define FOR_BACK_PORT						(PortC)		//前进后退按钮（正反转）
#define FOR_BACK_PIN						(Pin07)
	
#define BRU_MOTOR_START_PORT				(PortC)		//盘刷限位
#define BRU_MOTOR_START_PIN					(Pin06)


#define BRUSH_CURRENT_IT_PORT				(PortA)		//盘刷电机电流中断
#define BRUSH_CURRENT_IT_PIN				(Pin02)

#define SUCTION_CURRENT_IT_PORT				(PortB)		//吸风电机电流中断
#define SUCTION_CURRENT_IT_PIN				(Pin09)

#define DRIVE_CURRENT_IT_PORT				(PortB)		//驱动电机电流中断
#define DRIVE_CURRENT_IT_PIN				(Pin12)

extern void GPIO_vInit(void);
extern void BRUSH_CURRENT_IT_Init(void);
#endif
