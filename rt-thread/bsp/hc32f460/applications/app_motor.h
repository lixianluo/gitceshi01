#ifndef _APP_MOTOR_H_
#define _APP_MOTOR_H_

#define SPEED_ADD_TIME					(200)  //5ms����һ���߳� 1000ms���ٶȶ�ʱ��
#define SPEED_DEC_TIME					(100)  //5ms����һ���߳� 500ms���ٶȶ�ʱ��
#define SPEED_RATIO_MAX					(4096)



typedef struct {
	uint16_t v_add;
	uint16_t v_dec;
	int16_t v_rt_speed;
	int16_t v_next_speed;

}TspeedInfo;

extern TspeedInfo* Drive_Motor_ptGetInfo(void);

#endif 
