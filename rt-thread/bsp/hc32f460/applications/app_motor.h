#ifndef _APP_MOTOR_H_
#define _APP_MOTOR_H_





typedef struct {
	uint16_t v_add;
	uint16_t v_dec;
	int16_t v_rt_speed;
	int16_t v_next_speed;
}TspeedInfo;

/*¶ÔÍâº¯Êý----------------------------------------------------*/
TspeedInfo* ptApp_Motor_Branch(void);

TspeedInfo* ptApp_Motor_Suction(void);
/*-------------------------------------------------------------*/



#endif 
