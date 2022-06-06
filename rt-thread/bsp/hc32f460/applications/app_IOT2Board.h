#ifndef APP_IOT2BOARD_H_
#define APP_IOT2BOARD_H_
#include "stdint.h"

#define IOT_RX_QUEUE			8

#define IOT_TX_BUFFER_SIZE		80
#define IOT_RX_BUFFER_SIZE		80
#define IOT_RX_BUFFER_MIN_SIZE	8



#define IOT_BUFFER_HEAD			0xA5
#define IOT_BUFFER_TAIL			0x5A



#define DEF_SET(VAR1,VAR2)		((VAR1) |= (VAR2))		//位操作：设置位
#define DEF_RES(VAR1,VAR2)		((VAR1) &=~ (VAR2))		//位操作：清除位
#define DEF_TEST(VAR1,VAR2)		(((VAR1) & (VAR2)) == (VAR2))	


#define IOT_INFO_REPORT			0x0A


typedef struct {
	uint8_t head;
	uint8_t msgid;
	uint8_t len;
	uint8_t deviceid;
	uint8_t seq;
	uint8_t payload[IOT_TX_BUFFER_SIZE];
	uint16_t checksum;
	uint8_t	tail;
}TIOTMsgObjDef;



typedef struct {
	uint8_t msgid;
	void(*pvServiceFunc)(TIOTMsgObjDef* tObject);
}TIOTMsgTbDef;


typedef struct {
	uint8_t ucRxData[IOT_RX_BUFFER_SIZE];
	uint32_t ulRxByteCount;
}TIOTRxMessageObjDef;

/**发送的第几条消息枚举*/
enum {
	IOT_MSG_INFO_REPORT,
	IOT_MSG_ID_NUM,
};

/*对外函数----------------------------------------------------*/
extern void Usart3RxIrqCallback(void);

extern void IOT_vTransmitMessage(uint8_t ucMessageIndex);
/*-------------------------------------------------------------*/



#endif // !USE_TTL_H_
