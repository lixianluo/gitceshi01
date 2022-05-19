#ifndef _GPIO_H_
#define _GPIO_H_


/* 输出----------------------------------------------------------------------------*/
#define LED1_PORT							(PortC)
#define LED1_PIN							(Pin14)

#define LED2_PORT							(PortC)
#define LED2_PIN							(Pin15)

#define BLUE_TOGGLE()						(PORT_Toggle(LED1_PORT, LED1_PIN))
#define RED_TOGGLE()						(PORT_Toggle(LED2_PORT, LED2_PIN))



#define VALVE_PORT							(PortC)		//电磁阀
#define VALVE_PIN							(Pin06)		//电磁阀

#define POWEN_PORT							(PortA)		//电源保持引脚
#define POWEN_PIN							(Pin08)		//电源保持引脚

#define PWM_EN_PORT							(PortB)     //大功率使能口
#define PWM_EN_PIN							(Pin04)		//大功率使能口



/* 输入----------------------------------------------------------------------------*/
#define BRUSH_START_PORT					(PortC)		//盘刷启动
#define BRUSH_START_PIN						(Pin08)		//盘刷启动

#define SUCTION_START_PORT					(PortC)		//吸风启动
#define SUCTION_START_PIN					(Pin09)		//吸风启动

#define ALL_START_PORT						(PortC)		//盘刷 + 吸风启动
#define ALL_START_PIN						(Pin07)		//盘刷 + 吸风启动

#define HAND_PORT							(PortA)		//把手开关
#define HAND_PIN							(Pin01)		//把手开关


/*模拟通讯-------------------------------------------------------------------------*/
#define IIC_SDA_PORT						(PortC)		//模拟IIC_SDA
#define IIC_SDA_PIN							(Pin13)		//模拟IIC_SDA

#define IIC_SCL_PORT						(PortH)		//模拟IIC_SCL
#define IIC_SCL_PIN							(Pin02)		//模拟IIC_SCL

extern void GPIO_vInit(void);

#endif
