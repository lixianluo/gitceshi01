#ifndef _APP_OTA_H_
#define _APP_OTA_H_
#include "stdint.h"

#define OTA_RX_QUEUE	8

#define OTA_TX_BUFFER_SIZE		80
#define OTA_RX_BUFFER_SIZE		80


#define OTA_MSG_HEADER0			0xFF

#define OTA_MSG_HEADER1			0xEE

#define OTA_MESSAGE_HAND		0x41


typedef struct {
	uint8_t ucHeader0;
	uint8_t ucHeader1;
	uint8_t ucPayloadLen;
	uint8_t ucPayload[OTA_TX_BUFFER_SIZE];
}TOTAMsgObjDef;

typedef struct {
	uint8_t msgid;
	void(*pvServiceFunc)(TOTAMsgObjDef* tObject);
}TOTAMsgTbDef;




typedef struct {
	uint8_t ucRxData[OTA_RX_BUFFER_SIZE];
	uint32_t ulRxByteCount;
}TOTARxMessageObjDef;

enum {
	OTA_MSG_MESSAGE_HAND,
	OTA_MSG_ID_NUM,
};

enum {
	First_Handshake = 0,
	Second_Handshake,

};

/*¶ÔÍâº¯Êý------------------------------------------------------*/
extern void OTA_vTransmitMessage(uint8_t ucMessageIndex);
/*--------------------------------------------------------------*/

#endif // !_APP_OTA_H_
