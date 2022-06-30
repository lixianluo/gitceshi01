#include "can_iap.h"

#include <stdio.h>
#include <stdbool.h>

#include "can.h"
#include "Board2Board.h"
#include "sw_timer.h"
#include "flash.h"
#include "tnd_fota.h"

static uint8_t can_iap_sem = 0;
static uint8_t can_task_state = 0;
static uint8_t next_packet_index = 0;
static uint32_t iap_update_file_cur_size = 0;
static uint8_t can_msg_transfer_count = 0;
static uint8_t can_iap_target_device = 0;
static uint8_t can_iap_handshake = 0;
static uint32_t can_iap_next_data_address = 0;


typedef struct
{
    uint32_t msg_id;
    uint8_t msg_data[8];
}_iap_can_msg;

static _iap_can_msg iap_can_msg;

static int32_t _iap_do_handshake(void)
{
    TIapCmdDef iap_cmd_temp;

    /* send C every second, so the sender could know we are waiting for it. */
    if (TMR_bIsTimeExpired(TMR_IAP_POLLING))
    {
        TMR_vSetTime(TMR_IAP_POLLING, TMR_TIME_MS2TICKS(20));

        iap_cmd_temp.opcode = CAN_IAP_STATUS;
        iap_cmd_temp.node_addr = CAN_NODE_ID;
        iap_cmd_temp.status = IAP_CODE_C;
        set_iap_cmd(&iap_cmd_temp);
        (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer

        CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);
    }

    /**wait can message*/
    if (iap_can_msg.msg_id != 0)
    {
        /**0x1000 0000 + node id << 16*/
        if ((iap_can_msg.msg_id & 0xFFFF0000) == (0x10000000 + ((uint32_t)CAN_NODE_ID << 16)))    //!< iap data
        {
            /* packet sequence sanity check */
            if ((((iap_can_msg.msg_id >> 8) & 0xFF) != 0) || ((iap_can_msg.msg_id & 0xFF) != 0xFF))
            {
                return -IAP_ERR_SEQ;
            }
            else
            {
                /* congratulations, check passed. */
                iap_update_file_cur_size = 0;
                /**erase flash */
                FLASH_vEraseSector(FLASH_DOWNLOAD_START_ADDR, FLASH_DOWNLOAD_SIZE);

                can_iap_sem = 2;    //!< data received

                /**send nack to retransmit first packey again*/
                iap_cmd_temp.opcode = CAN_IAP_STATUS;
                iap_cmd_temp.node_addr = CAN_NODE_ID;
                iap_cmd_temp.status = IAP_CODE_NAK;
                set_iap_cmd(&iap_cmd_temp);
                (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer
                CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);

                return 0;   //!< first packet received
            }

        }
        else if ((iap_can_msg.msg_id & 0xFFFF0000) == 0x11000000)   //!< iap cmd
        {
            if ((iap_can_msg.msg_data[0] == CAN_IAP_STATUS) && (iap_can_msg.msg_data[1] == CAN_NODE_ID) && (iap_can_msg.msg_data[2] == IAP_CODE_EOT))
            {
                return IAP_ERR_CAN;
            }
        }
    }

    return 1;   //!< not finished, continue
}

static int32_t _iap_do_trans(void)
{
    TIapCmdDef iap_cmd_temp;
    enum iap_code code;

    /**wait can message*/
    if (iap_can_msg.msg_id != 0)
    {
        /**0x1000 0000 + node id << 16*/
        if ((iap_can_msg.msg_id & 0xFFFF0000) == (0x10000000 + ((uint32_t)CAN_NODE_ID << 16)))    //!< iap data
        {
            /* packet sequence sanity check */
            if ((((iap_can_msg.msg_id >> 8) & 0xFF) + (iap_can_msg.msg_id & 0xFF)) != 0xFF)
            {
                code = IAP_CODE_NAK;
            }
            else
            {
                /* congratulations, check passed. */
                /**ignore repeat packet*/
                if (next_packet_index == ((iap_can_msg.msg_id >> 8) & 0xFF))
                {
                    next_packet_index = (next_packet_index + 1) % 256;

                    /**write to flash*/
                    FLASH_vWriteBytes(FLASH_DOWNLOAD_START_ADDR + iap_update_file_cur_size, iap_can_msg.msg_data, 8u);
                    iap_update_file_cur_size += 8;
                }

                code = IAP_CODE_ACK;
            }
        }
        else if ((iap_can_msg.msg_id & 0xFFFF0000) == 0x11000000)   //!< iap cmd
        {
            if ((iap_can_msg.msg_data[0] == CAN_IAP_STATUS) && (iap_can_msg.msg_data[1] == CAN_NODE_ID) && (iap_can_msg.msg_data[2] == IAP_CODE_EOT))
            {
                return 0;   //!< end of transmission
            }
        }
        else
        {
            code = IAP_CODE_NAK;
        }
    }
    else
    {
        return 1;   //!< not received, continue
    }

    switch (code)
    {
        case IAP_CODE_CAN:
        {
            iap_cmd_temp.opcode = CAN_IAP_STATUS;
            iap_cmd_temp.node_addr = CAN_NODE_ID;
            iap_cmd_temp.status = IAP_CODE_CAN;
            set_iap_cmd(&iap_cmd_temp);
            (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer
            CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);

            return -IAP_ERR_CAN;
        }

        case IAP_CODE_ACK:
        {
            iap_cmd_temp.opcode = CAN_IAP_STATUS;
            iap_cmd_temp.node_addr = CAN_NODE_ID;
            iap_cmd_temp.status = IAP_CODE_ACK;
            set_iap_cmd(&iap_cmd_temp);
            (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer
            CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);
            break;
        }

        case IAP_CODE_NAK:
        {
            iap_cmd_temp.opcode = CAN_IAP_STATUS;
            iap_cmd_temp.node_addr = CAN_NODE_ID;
            iap_cmd_temp.status = IAP_CODE_NAK;
            set_iap_cmd(&iap_cmd_temp);
            (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer
            CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);
            break;
        }

        default:
            // wrong code
            break;
    }

    return 1;
}

static int32_t _iap_do_fin(void)
{
    /* put the last ACK */
    TIapCmdDef iap_cmd_temp;

    iap_cmd_temp.opcode = CAN_IAP_STATUS;
    iap_cmd_temp.node_addr = CAN_NODE_ID;
    iap_cmd_temp.status = IAP_CODE_ACK;
    set_iap_cmd(&iap_cmd_temp);
    (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer
    CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);

    return 0;
}

static int32_t _iap_server_set_offline(void)
{
    if (TMR_bIsTimeExpired(TMR_IAP_POLLING))
    {
        TMR_vSetTime(TMR_IAP_POLLING, TMR_TIME_MS2TICKS(50));

        CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_NMT_CTRL);

        if (can_msg_transfer_count)
        {
            can_msg_transfer_count--;
        }
    }



    return (can_msg_transfer_count != 0);
}

static int32_t _iap_server_do_handshake(void)
{
    TIapCmdDef iap_cmd_temp;

    /* send C every second, so the sender could know we are waiting for it. */
    if (TMR_bIsTimeExpired(TMR_IAP_POLLING))
    {
        TMR_vSetTime(TMR_IAP_POLLING, TMR_TIME_MS2TICKS(1000));

        iap_cmd_temp.opcode = can_iap_handshake;
        iap_cmd_temp.node_addr = can_iap_target_device;
        set_iap_cmd(&iap_cmd_temp);
        (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer

        CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);
    }

    /**wait can message*/
    if (iap_can_msg.msg_id != 0)
    {
        if ((iap_can_msg.msg_id & 0xFFFF0000) == 0x11000000)   //!< iap cmd
        {
            if ((iap_can_msg.msg_data[0] == CAN_IAP_REQUEST_ACK) && (iap_can_msg.msg_data[1] == can_iap_target_device) && (iap_can_msg.msg_data[2] == 0))
            {
                can_iap_handshake = CAN_IAP_REQUEST_ACK_CONFIRM;

                iap_cmd_temp.opcode = CAN_IAP_REQUEST_ACK_CONFIRM;
                iap_cmd_temp.node_addr = can_iap_target_device;
                set_iap_cmd(&iap_cmd_temp);
                (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer

                CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);
            }
            else if ((iap_can_msg.msg_data[0] == CAN_IAP_STATUS) && (iap_can_msg.msg_data[1] == can_iap_target_device) && (iap_can_msg.msg_data[2] == IAP_CODE_C))
            {
                return 0;   //!< data transfer request received
            }
        }
    }

    return 1;   //!< not finished, continue
}

static int32_t _iap_server_do_trans(void)
{
    enum iap_code code;

    /**wait can message*/
    if (iap_can_msg.msg_id != 0)
    {
        if ((iap_can_msg.msg_id & 0xFFFF0000) == 0x11000000)   //!< iap cmd
        {
            if ((iap_can_msg.msg_data[0] == CAN_IAP_STATUS) && (iap_can_msg.msg_data[1] == can_iap_target_device) && (iap_can_msg.msg_data[2] == IAP_CODE_ACK))
            {
                code = IAP_CODE_ACK;
            }
            else if ((iap_can_msg.msg_data[0] == CAN_IAP_STATUS) && (iap_can_msg.msg_data[1] == can_iap_target_device) && (iap_can_msg.msg_data[2] == IAP_CODE_NAK))
            {
                code = IAP_CODE_NAK;
            }
            else if ((iap_can_msg.msg_data[0] == CAN_IAP_STATUS) && (iap_can_msg.msg_data[1] == can_iap_target_device) && (iap_can_msg.msg_data[2] == IAP_CODE_C))
            {
                code = IAP_CODE_C;
            }
            else if ((iap_can_msg.msg_data[0] == CAN_IAP_STATUS) && (iap_can_msg.msg_data[1] == can_iap_target_device) && (iap_can_msg.msg_data[2] == IAP_CODE_EOT))
            {
                return 0;   //!< end of transmission
            }
        }
        else
        {
            code = IAP_CODE_NAK;
        }
    }
    else
    {
        return 1;   //!< not received, continue
    }

    switch (code)
    {
        case IAP_CODE_CAN:
        {
            return 0;
        }

        case IAP_CODE_ACK:
        {
            next_packet_index = (next_packet_index + 1) % 256;  //!< next packet index
            can_iap_next_data_address += 8u;    //!< next packet data address 

            /**check if file end reach*/
            if (can_iap_next_data_address >= (FLASH_DOWNLOAD_START_ADDR + FOTA_ulGetAppFileSize()))
            {
                TIapCmdDef iap_cmd_temp;

                iap_cmd_temp.opcode = CAN_IAP_STATUS;
                iap_cmd_temp.node_addr = can_iap_target_device;
                iap_cmd_temp.status = IAP_CODE_EOT;
                set_iap_cmd(&iap_cmd_temp);
                (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer
                CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_CMD);

                return 0;
            }

            set_iap_data((uint8_t*)can_iap_next_data_address);

            (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer
            CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_DATA);
            break;
        }

        case IAP_CODE_NAK:
        {
            set_iap_data((uint8_t*)can_iap_next_data_address);

            (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer
            CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_DATA);
            break;
        }

        case IAP_CODE_C:
        {
            next_packet_index = 0;
            can_iap_next_data_address = FLASH_DOWNLOAD_START_ADDR;
            set_iap_data((uint8_t*)can_iap_next_data_address);

            (void)memset(&iap_can_msg, 0, sizeof(_iap_can_msg));    //!< clear msg buffer
            CAN_vTransmitMessage(CAN_NODE_BOARDS, B2B_MSGIDX_SEND_IAP_DATA);

            break;
        }

        default:
            // wrong code
            break;
    }

    return 1;
}


static int32_t _iap_server_do_fin(void)
{
    /* receive the last ACK */
    if (iap_can_msg.msg_id != 0)
    {
        if ((iap_can_msg.msg_id & 0xFFFF0000) == 0x11000000)   //!< iap cmd
        {
            if ((iap_can_msg.msg_data[0] == CAN_IAP_STATUS) && (iap_can_msg.msg_data[1] == can_iap_target_device) && (iap_can_msg.msg_data[2] == IAP_CODE_ACK))
            {
                return 0;   //!< data transfer request received
            }
        }
    }

    return 1;   //!< not finished, continue
}

void can_iap(void)
{
    if (!can_iap_sem) return;

    switch (can_task_state)
    {
        case IAP_STAGE_NONE:
        case IAP_STAGE_ESTABLISHING:  //IAP 建立阶段
        {
            if (0 == _iap_do_handshake())
            {
                can_task_state = IAP_STAGE_TRANSMITTING;
                next_packet_index = 0;
            }

            if (CAN_IAP_SERVER_MODE == can_iap_sem)
            {
                can_task_state = IAP_SERVER_STAGE_OFFLINE;
                can_msg_transfer_count = 8;
            }

            break;
        }

        case IAP_STAGE_TRANSMITTING: //IAP 传递阶段
        {
            if (0 == _iap_do_trans())
            {
                can_task_state = IAP_STAGE_FINISHING;
            }

            break;
        }

        case IAP_STAGE_FINISHING:  //IAP 完成阶段
        {
            if (0 == _iap_do_fin())
            {
                can_task_state = IAP_STAGE_FINISHED;
                TMR_vSetTime(TMR_IAP_POLLING, TMR_TIME_MS2TICKS(50));
            }
            break;
        }

        case IAP_STAGE_FINISHED: //IAP 完成后重启
        {
            /**wait some time to let ack message send out successfully*/
            if (TMR_bIsTimeExpired(TMR_IAP_POLLING))
            {
                extern void hw_cpu_reset(void);
                hw_cpu_reset();
            }

            break;
        }

        case IAP_SERVER_STAGE_OFFLINE:
        {
            if (0 == _iap_server_set_offline())
            {
                can_task_state = IAP_SERVER_STAGE_ESTABLISHING;
                can_iap_handshake = CAN_IAP_REQUEST;
            }

            break;
        }

        case IAP_SERVER_STAGE_ESTABLISHING:
        {
            if (0 == _iap_server_do_handshake())
            {
                can_task_state = IAP_SERVER_STAGE_TRANSMITTING;
            }
            break;
        }

        case IAP_SERVER_STAGE_TRANSMITTING:
        {
            if (0 == _iap_server_do_trans())
            {
                can_task_state = IAP_SERVER_STAGE_FINISHING;
            }
            break;
        }

        case IAP_SERVER_STAGE_FINISHING:
        {
            if (0 == _iap_server_do_fin())
            {
                can_task_state = IAP_SERVER_STAGE_FINISHED;
            }

            break;
        }

        case IAP_SERVER_STAGE_FINISHED:
        {
            break;
        }

        default:break;
    }
}

void can_start_iap(uint8_t sem_value)
{
    can_iap_sem = sem_value;
}


void can_iap_task_init(void)
{
    can_iap_sem = CAN_IAP_CLENT_MODE;
    can_task_state = IAP_STAGE_ESTABLISHING;
}

uint8_t can_is_data_received(void)
{
    return (can_iap_sem == CAN_IAP_CLENT_MODE_DATA);
}

void can_set_received_data(uint32_t msg_id, uint8_t* msg_data)
{
    iap_can_msg.msg_id = msg_id;
    (void)memcpy(iap_can_msg.msg_data, msg_data, 8);
}

void can_set_target_device(uint8_t device_id)
{
    can_iap_target_device = device_id;
}

uint32_t can_get_msg_exd_id(void)
{
    return (uint32_t)CAN_MSG_ID_IAP_DATA + ((uint32_t)can_iap_target_device << 16) + ((uint32_t)next_packet_index << 8) + (uint32_t)(255 - next_packet_index);
}

uint8_t can_iap_get_task_state(void)
{
    return can_task_state;
}


void hw_cpu_reset(void)
{
#define SCB_AIRCR       (*(volatile unsigned long *)0xE000ED0C)  /* Reset control Address Register */
#define SCB_RESET_VALUE 0x05FA0004                               /* Reset value, write to SCB_AIRCR can reset cpu */

    SCB_AIRCR = SCB_RESET_VALUE;
}