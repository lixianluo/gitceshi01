#ifndef _YMODEM_OTA_H
#define _YMODEM_OTA_H

#include "defines.h"

/* The word "RYM" is stand for "Real-YModem". */

enum rym_code {
	RYM_CODE_NONE = 0x00,
	RYM_CODE_SOH = 0x01,
	RYM_CODE_STX = 0x02,
	RYM_CODE_EOT = 0x04,
	RYM_CODE_ACK = 0x06,
	RYM_CODE_NAK = 0x15,
	RYM_CODE_CAN = 0x18,
	RYM_CODE_C = 0x43,
};

/* RYM error code
 *
 * We use the rt_err_t to return error values. We take use of current error
 * codes available in RTT and append ourselves.
 */
 /* timeout on handshake */
#define RYM_ERR_TMO  0x70
/* wrong code, wrong SOH, STX etc. */
#define RYM_ERR_CODE 0x71
/* wrong sequence number */
#define RYM_ERR_SEQ  0x72
/* wrong CRC checksum */
#define RYM_ERR_CRC  0x73
/* not enough data received */
#define RYM_ERR_DSZ  0x74
/* the transmission is aborted by user */
#define RYM_ERR_CAN  0x75


enum rym_stage {
	RYM_STAGE_NONE,
	/* set when C is send */
	RYM_STAGE_ESTABLISHING,
	/* set when we've got the packet 0 and sent ACK and second C */
	RYM_STAGE_ESTABLISHED,
	/* set when the sender respond to our second C and recviever got a real
	* data packet. */
	RYM_STAGE_TRANSMITTING,
	/* set when the sender send a EOT */
	RYM_STAGE_FINISHING,
	/* set when transmission is really finished, i.e., after the NAK, C, final
	* NULL packet stuff. */

	RYM_STAGE_FINISH_PACKET,
	RYM_STAGE_FINISHED,
	RYM_STAGE_RESET,
	RYM_STAGE_IDLE
};

extern void ymodem_ota(void);

extern void ymodem_ota_set_received_data(uint32_t msg_len, uint8_t* msg_data);
extern void ymodem_start_ota(void);
extern void ymodem_ota_task_init(void);
extern uint8_t ymodem_is_ota_data_received(void);
















#endif