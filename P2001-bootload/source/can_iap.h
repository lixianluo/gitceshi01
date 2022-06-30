#ifndef _CAN_IAP_H
#define _CAN_IAP_H

#include "defines.h"

enum iap_code {
    IAP_CODE_NONE = 0x00,
    IAP_CODE_SOH = 0x01,
    IAP_CODE_STX = 0x02,
    IAP_CODE_EOT = 0x04,
    IAP_CODE_ACK = 0x06,
    IAP_CODE_NAK = 0x15,
    IAP_CODE_CAN = 0x18,
    IAP_CODE_C = 0x43,
};

/* RYM error code
 *
 * We use the rt_err_t to return error values. We take use of current error
 * codes available in RTT and append ourselves.
 */
 /* timeout on handshake */
#define IAP_ERR_TMO  0x70
/* wrong code, wrong SOH, STX etc. */
#define IAP_ERR_CODE 0x71
/* wrong sequence number */
#define IAP_ERR_SEQ  0x72
/* wrong CRC checksum */
#define IAP_ERR_CRC  0x73
/* not enough data received */
#define IAP_ERR_DSZ  0x74
/* the transmission is aborted by user */
#define IAP_ERR_CAN  0x75


#define IAP_MAGIC_WORD_BEGIN            0xf0a55a0f
#define IAP_MAGIC_WORD_END              0xa5f00f5a

///* how many ticks wait for chars between packet. */
//#ifndef IAP_WAIT_CHR_TICK
//#define IAP_WAIT_CHR_TICK (RT_TICK_PER_SECOND * 3)
//#endif
///* how many ticks wait for between packet. */
//#ifndef IAP_WAIT_PKG_TICK
//#define IAP_WAIT_PKG_TICK (RT_TICK_PER_SECOND * 3)
//#endif
///* how many ticks between two handshake code. */
//#ifndef IAP_CHD_INTV_TICK
//#define IAP_CHD_INTV_TICK (RT_TICK_PER_SECOND * 3)
//#endif


enum iap_stage {
    IAP_STAGE_NONE,
    /* set when C is send */
    IAP_STAGE_ESTABLISHING,
    /* set when we've got the packet 0 and sent ACK and second C */
    IAP_STAGE_ESTABLISHED,
    /* set when the sender respond to our second C and recviever got a real
    * data packet. */
    IAP_STAGE_TRANSMITTING,
    /* set when the sender send a EOT */
    IAP_STAGE_FINISHING,
    /* set when transmission is really finished, i.e., after the NAK, C, final
    * NULL packet stuff. */
    IAP_STAGE_FINISHED,

    IAP_SERVER_STAGE_OFFLINE,   //!< can iap server mode to forward to othet nodes
    IAP_SERVER_STAGE_ESTABLISHING,
    IAP_SERVER_STAGE_TRANSMITTING,
    IAP_SERVER_STAGE_FINISHING,
    IAP_SERVER_STAGE_FINISHED
};

//struct iap_ctx;
///* when receiving files, the buf will be the data received from ymodem protocol
// * and the len is the data size.
// *
// * TODO:
// * When sending files, the len is the buf size in RYM. The callback need to
// * fill the buf with data to send. Returning RYM_CODE_EOT will terminate the
// * transfer and the buf will be discarded. Any other return values will cause
// * the transfer continue.
// */
//typedef enum iap_code(*iap_callback)(struct iap_ctx* ctx, rt_uint8_t* buf, rt_size_t len);
//
//
///* Currently RYM only support one transfer session(ctx) for simplicity.
// *
// * In case we could support multiple sessions in The future, the first argument
// * of APIs are (struct rym_ctx*).
// */
//struct iap_ctx
//{
//    iap_callback on_data;
//    iap_callback on_begin;
//    iap_callback on_end;
//    /* When error happened, user need to check this to get when the error has
//     * happened. */
//    enum iap_stage stage;
//    /* user could get the error content through this */
//    uint8_t* buf;
//    uint8_t next_packet_index;
//    uint8_t protocal;    //!< 0: ymodem; 1: xmodem_128
//    struct rt_semaphore sem;
//};
//




enum {
    CAN_IAP_CLENT_MODE = 1,
    CAN_IAP_CLENT_MODE_DATA,
    CAN_IAP_SERVER_MODE
};











extern void can_iap(void);
extern void can_start_iap(uint8_t sem_value);
extern void can_iap_task_init(void);

extern uint8_t can_is_data_received(void);
extern void can_set_received_data(uint32_t msg_id, uint8_t* msg_data);
extern void can_set_target_device(uint8_t device_id);
extern uint32_t can_get_msg_exd_id(void);
extern uint8_t can_iap_get_task_state(void);


extern void hw_cpu_reset(void);















#endif