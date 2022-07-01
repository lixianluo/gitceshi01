#ifndef _GPIO_H_
#define _GPIO_H_


/* 输出----------------------------------------------------------------------------*/
#define BLUE_PORT							(PortC)		//系统蓝灯
#define BLUE_PIN							(Pin14)		//系统蓝灯

#define RED_PORT							(PortC)		//系统红灯
#define RED_PIN								(Pin15)		//系统红灯


/* 原理图RGB 蓝灯PB7 红灯PB5 绿灯PB6  原理图有问题 按软件为主 */
//#define RGB_BLUE_PORT						(PortB)		//RGB蓝灯
//#define RGB_BLUE_PIN						(Pin07)		//RGB蓝灯  

#define VALVE_PORT							(PortC)		//电磁阀
#define VALVE_PIN							(Pin06)		//电磁阀

#define POWEN_PORT							(PortA)		//电源保持引脚
#define POWEN_PIN							(Pin08)		//电源保持引脚

#define BRUSH_EN_PORT						(PortB)     //盘刷电机使能口
#define BRUSH_EN_PIN						(Pin04)		//盘刷电机能口

#define SUCTION_EN_PORT						(PortB)     //吸风电机使能口
#define SUCTION_EN_PIN						(Pin12)		//吸风电机能口

/* 输入----------------------------------------------------------------------------*/
#define SWITCH_B3_PORT						(PortC)		//旋转开关B3引脚
#define SWITCH_B3_PIN						(Pin09)		//旋转开关B3引脚

#define SWITCH_A2_PORT						(PortC)		//旋转开关A2引脚
#define SWITCH_A2_PIN						(Pin08)		//旋转开关A2引脚

#define SWITCH_A1_PORT						(PortC)		//旋转开关A1引脚
#define SWITCH_A1_PIN						(Pin07)		//旋转开关A1引脚

#define HAND_PORT							(PortA)		//把手开关
#define HAND_PIN							(Pin02)		//把手开关


/*模拟通讯-------------------------------------------------------------------------*/
#define IIC_SDA_PORT						(PortC)		//模拟IIC_SDA
#define IIC_SDA_PIN							(Pin13)		//模拟IIC_SDA

#define IIC_SCL_PORT						(PortH)		//模拟IIC_SCL
#define IIC_SCL_PIN							(Pin02)		//模拟IIC_SCL


/*应用------------------------------------------------------------------------------*/
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
