/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-30     armink       the first version
 * 2018-08-27     Murphy       update log
 */
#include "ymodem_ota.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "UART/uart.h"
#include "sw_timer.h"
#include "AlgorithmBoard/AlgorithmBoard.h"
#include "flash.h"
#include "can_iap.h"

static uint8_t ymodem_ota_sem = 0;
static uint8_t ymodem_ota_task_state = 0;

#define YMODEM_DMA_SIZE     1040
typedef struct {
    uint32_t msg_len;
    uint8_t msg_data[YMODEM_DMA_SIZE];
}_ymodem_ota_msg;

static _ymodem_ota_msg ymodem_ota_msg;
static uint8_t ymodem_next_packet_index = 0;
int32_t ymodem_code = 0;


uint32_t update_file_total_size, update_file_cur_size;

static const uint16_t ccitt_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


static uint32_t _rym_putchar(uint8_t code);
static int32_t _rym_do_handshake(void);
static int32_t _rym_trans_data(void);
static int32_t _rym_do_trans(void);

static uint8_t ymodem_on_begin(uint8_t* buf, uint32_t len);
static uint8_t ymodem_on_data(uint8_t* buf, uint32_t len);

uint16_t CRC16(unsigned char* q, int len)
{
    uint16_t crc = 0;

    while (len-- > 0)
        crc = (crc << 8) ^ ccitt_table[((crc >> 8) ^ *q++) & 0xff];
    return crc;
}


static uint32_t _rym_putchar(uint8_t code)
{
    USART_SendData(uart2_handler.USARTx, code);
    return 1;
}


/* SOH/STX + seq + payload + crc */
#define _RYM_SOH_PKG_SZ (1+2+128+2)
#define _RYM_STX_PKG_SZ (1+2+1024+2)


static int32_t _rym_do_handshake(void)  //函数介绍:Y_mode 进行握手协议
{
    uint16_t recv_crc, cal_crc;
    uint32_t data_sz;

    /* send C every second, so the sender could know we are waiting for it. */
    if (TMR_bIsTimeExpired(TMR_YMODEM_POLLING))
    {
        TMR_vSetTime(TMR_YMODEM_POLLING, TMR_TIME_MS2TICKS(40));

        (void)memset(&ymodem_ota_msg, 0, sizeof(_ymodem_ota_msg));      //清空buff
        _rym_putchar(RYM_CODE_C);                                       //串口发送 ‘C’字符(40ms周期)
    }

    /* receive all data */
    if (ymodem_ota_msg.msg_len != 0)                                //如果buff的长度不等于0
    {
        if (ymodem_ota_msg.msg_data[0] == RYM_CODE_SOH)             //如果buff[0] == YMODEM_SOH(0x01)  则表示133字节长度帧
        {
            data_sz = _RYM_SOH_PKG_SZ;                              //帧头+包号+包号反码+buff[128]+CRC_H+CRC_L
        }
        else if (ymodem_ota_msg.msg_data[0] == RYM_CODE_STX)        //如果buff[0] == YMODEM_STX(0x02)  则表示1024字节长度帧
        {
            data_sz = _RYM_STX_PKG_SZ;                              //帧头+包号+包号反码+buff[1024]+CRC_H+CRC_L
        }

        /* sanity check */
        if (ymodem_ota_msg.msg_data[1] != 0 || ymodem_ota_msg.msg_data[2] != 0xFF)  //判断包号是否为0和包号反码是否为0xFF
            return -RYM_ERR_SEQ;         

        recv_crc = (uint16_t)(ymodem_ota_msg.msg_data[data_sz - 2] << 8) | (uint16_t)ymodem_ota_msg.msg_data[data_sz - 1];//接收到的CRC
        cal_crc = CRC16(&ymodem_ota_msg.msg_data[3], data_sz - 5);//计算出来的CRC

        if (recv_crc != cal_crc)
        {
            _rym_putchar(RYM_CODE_NAK);    //!< checksum failed, re-transmission    //crc校验失败  要求重新接收当前数据包请求

            return -RYM_ERR_CRC;
        }

        /* congratulations, check passed. */
        ymodem_next_packet_index = (ymodem_next_packet_index + 1) % 256;    // 到达这一步，说明检查通过  进入下一个包

        if (ymodem_on_begin(&ymodem_ota_msg.msg_data[3], data_sz - 5) != RYM_CODE_ACK)  //判断 文件名称 和 文件大小
        {
            return -RYM_ERR_CAN;
        }
        else
        {
            return 0; 
        }
    }

    return 1;
}

static int32_t _rym_trans_data(void)
{
    uint32_t data_sz;
    uint8_t code;
    uint16_t recv_crc;

    if (ymodem_ota_msg.msg_len != 0)
    {
        switch (ymodem_ota_msg.msg_data[0])
        {
            case RYM_CODE_SOH:
            {
                data_sz = 128;
                break;
            }

            case RYM_CODE_STX:
            {
                data_sz = 1024;
                break;
            }

            case RYM_CODE_EOT:
            {
                return 0;   //!< end of transmission, quit      //当接收到EOT 文件传输命令结束 返回0
            }

            default:
            {
                code = RYM_CODE_NAK;
                goto __exit_reception;
            }
        }

        /**check rx msg len*/
        if (ymodem_ota_msg.msg_len != (1 + 2 + data_sz + 2))        //判断接收到的长度
        {
            code = RYM_CODE_NAK;
            goto __exit_reception;
        }

        /* seq + data + crc */
        if ((ymodem_ota_msg.msg_data[1] + ymodem_ota_msg.msg_data[2]) != 0xFF)  //判断包号和包号反码
        {
            code = RYM_CODE_NAK;
            goto __exit_reception;
        }

        /* sanity check */
        recv_crc = (uint16_t)(ymodem_ota_msg.msg_data[ymodem_ota_msg.msg_len - 2] << 8) | (uint16_t)ymodem_ota_msg.msg_data[ymodem_ota_msg.msg_len - 1];
        if (recv_crc != CRC16(&ymodem_ota_msg.msg_data[3], data_sz))    //判断CRC校验位
        {
            code = RYM_CODE_NAK;
            goto __exit_reception;
        }

        if (ymodem_next_packet_index == ymodem_ota_msg.msg_data[1]) //ymodem 下一次packet包的号 == 包号
        {
            ymodem_next_packet_index = (ymodem_next_packet_index + 1) % 256; //ymodem 下一次packet包的号 +1

            /* congratulations, check passed. */
            if (ymodem_on_data(&ymodem_ota_msg.msg_data[3], data_sz) != RYM_CODE_ACK) //接收ymodem 有效数据
            {
                code = RYM_CODE_CAN;
                goto __exit_reception;
            }
        }

        code = RYM_CODE_ACK;        //将ACK 赋予 code
    }
    else
    {
        return 1;   //!< not receive, continue
    }

__exit_reception:    
    ymodem_code = code;
    switch (code)
    {
        case RYM_CODE_CAN:
        {
            (void)memset(&ymodem_ota_msg, 0, sizeof(_ymodem_ota_msg));

            /* the spec require multiple CAN */
            _rym_putchar(RYM_CODE_CAN);
            _rym_putchar(RYM_CODE_CAN);
            _rym_putchar(RYM_CODE_CAN);
            _rym_putchar(RYM_CODE_CAN);
            _rym_putchar(RYM_CODE_CAN);

            return -RYM_ERR_CAN;
        }

        case RYM_CODE_ACK:
            (void)memset(&ymodem_ota_msg, 0, sizeof(_ymodem_ota_msg));//buff清除

            _rym_putchar(RYM_CODE_ACK); //发送ACK
            break;

        case RYM_CODE_NAK:
            (void)memset(&ymodem_ota_msg, 0, sizeof(_ymodem_ota_msg));

            _rym_putchar(RYM_CODE_NAK);
            break;
        default:
            // wrong code
            break;
    }

    return 1;   //!< not finished, continue
}

static int32_t _rym_do_trans(void)
{
    _rym_putchar(RYM_CODE_ACK); //回复ACK表示收到起始帧

    (void)memset(&ymodem_ota_msg, 0, sizeof(_ymodem_ota_msg));  //清除buff
    _rym_putchar(RYM_CODE_C);                                   //发送字符C

    return 0;
}

static int32_t _rym_do_fin(void)
{
    /* we already got one EOT in the caller. invoke the callback if there is
     * one. */
    //if (ctx->on_end)
    //    ctx->on_end(ctx, ctx->buf + 3, 128);
    (void)memset(&ymodem_ota_msg, 0, sizeof(_ymodem_ota_msg));//buff清零

    _rym_putchar(RYM_CODE_NAK); //发送NAK

    return 0;
}

static int32_t _rym_trans_fin_packet(void)
{
    if (ymodem_ota_msg.msg_len != 0)
    {
        if (ymodem_ota_msg.msg_data[0] != RYM_CODE_EOT)
        {
            (void)memset(&ymodem_ota_msg, 0, sizeof(_ymodem_ota_msg));

            _rym_putchar(RYM_CODE_NAK);
            return 1;
        }

        (void)memset(&ymodem_ota_msg, 0, sizeof(_ymodem_ota_msg));
        _rym_putchar(RYM_CODE_ACK); //发送ACK
        _rym_putchar(RYM_CODE_C);   //发送字符C

        return 0;
    }
    else
    {
        return 1;
    }
}

static int32_t _rym_trans_fin(void)
{
    uint32_t data_sz;
    uint16_t recv_crc;

    if (ymodem_ota_msg.msg_len != 0)
    {
        if (ymodem_ota_msg.msg_data[0] == RYM_CODE_SOH)
        {
            data_sz = _RYM_SOH_PKG_SZ;
        }
        else if (ymodem_ota_msg.msg_data[0] == RYM_CODE_STX)
        {
            data_sz = _RYM_STX_PKG_SZ;
        }
        else
        {
            return -RYM_ERR_CODE;
        }

        //if (ymodem_ota_msg.msg_len != _RYM_SOH_PKG_SZ)
        //{
        //    return -RYM_ERR_DSZ;
        //}

        /* sanity check */
         if (ymodem_ota_msg.msg_data[1] != 0 || ymodem_ota_msg.msg_data[2] != 0xFF)
             return -RYM_ERR_SEQ;

         recv_crc = ((uint16_t)ymodem_ota_msg.msg_data[_RYM_SOH_PKG_SZ - 2] << 8) | (uint16_t)ymodem_ota_msg.msg_data[_RYM_SOH_PKG_SZ - 1];
         if (recv_crc != CRC16(&ymodem_ota_msg.msg_data[3], _RYM_SOH_PKG_SZ - 5))
             return -RYM_ERR_CRC;

         /* congratulations, check passed. */
         /* put the last ACK */
         _rym_putchar(RYM_CODE_ACK);
         return 0;
    }

    return 1;
}

static uint8_t ymodem_on_begin(uint8_t *buf, uint32_t len)
{
    char *file_name, *file_size;

    /* calculate and store file size */
    file_name = (char *)&buf[0];
    file_size = (char *)&buf[strlen(file_name) + 1];
    update_file_total_size = atol(file_size);

    update_file_cur_size = 0;

    /* Get download partition information and erase download partition data */
    if (update_file_total_size > FLASH_DOWNLOAD_SIZE)
    {
        return RYM_CODE_CAN;
    }

    /* erase DL section */
    //if (fal_partition_erase(dl_part, 0, update_file_total_size) < 0)
    //{
    //    LOG_E("Firmware download failed! Partition (%s) erase error!", dl_part->name);
    //    return RYM_CODE_CAN;
    //}

    FLASH_vEraseSector(FLASH_DOWNLOAD_START_ADDR, FLASH_DOWNLOAD_SIZE);
    ymodem_ota_sem = 2;    //!< data received

    return RYM_CODE_ACK;
}

static uint8_t ymodem_on_data(uint8_t *buf, uint32_t len)
{
    /* write data of application to DL partition  */
    //if (fal_partition_write(dl_part, update_file_cur_size, buf, len) < 0)
    //{
    //    LOG_E("Firmware download failed! Partition (%s) write data error!", dl_part->name);

    //    return RYM_CODE_CAN;
    //}
    FLASH_vWriteBytes(FLASH_DOWNLOAD_START_ADDR + update_file_cur_size, buf, len);


    update_file_cur_size += len;

    return RYM_CODE_ACK;
}

void ymodem_ota(void)
{
    if (!ymodem_ota_sem) return;

    switch (ymodem_ota_task_state)
    {
        case RYM_STAGE_NONE:  // Y_modem 协议阶段节点
        {
            ALGO_vReceptionInit();          //接收串口消息
            ymodem_ota_task_state = RYM_STAGE_ESTABLISHING; //进入Y_modem 协议阶段
            ymodem_next_packet_index = 0;                   //Y_modem packet index = 0

            break;
        }

        case RYM_STAGE_ESTABLISHING: //Y_modem 协议建立阶段
        {
            //if (can_iap_get_task_state() > IAP_STAGE_ESTABLISHING)  //判断can_iap是否在升级
            //{
            //    _rym_putchar('u');
            //    _rym_putchar('p');
            //    _rym_putchar('g');
            //    _rym_putchar('r');
            //    _rym_putchar('a');
            //    _rym_putchar('d');
            //    _rym_putchar('i');
            //    _rym_putchar('n');
            //    _rym_putchar('g');
            //    ymodem_ota_task_state = RYM_STAGE_IDLE;

            //    break;
            //}

            /*
                Y_mode 协议进行握手
                下位机40ms发送字符C，上位机收到发送Ymodem起始帧
                判断起始帧的各项参数，通过返回0
            */

            if (0 == _rym_do_handshake())          
            {
                ymodem_ota_task_state = RYM_STAGE_ESTABLISHED;  //通过则进行Y_modem协议确立阶段
            }

            break;
        }

        case RYM_STAGE_ESTABLISHED: //Y_modem协议确立阶段
        {
            if (0 == _rym_do_trans())
            {
                ymodem_ota_task_state = RYM_STAGE_TRANSMITTING; 
            }

            break;
        }

        case RYM_STAGE_TRANSMITTING: //Y_modem协议传输阶段
        {
            if (0 == _rym_trans_data())
            {
                ymodem_ota_task_state = RYM_STAGE_FINISHING;
            }
            break;
        }

        case RYM_STAGE_FINISHING: //Y_modem协议正在完成阶段
        {
            if (0 == _rym_do_fin())
            {
                ymodem_ota_task_state = RYM_STAGE_FINISH_PACKET;
            }
            break;
        }

        case RYM_STAGE_FINISH_PACKET:   //Y_modem协议完成PACKET阶段
        {
            if (0 == _rym_trans_fin_packet())
            {
                ymodem_ota_task_state = RYM_STAGE_FINISHED;
            }

            break;
        }

        case RYM_STAGE_FINISHED: //Y_modem协议完成阶段
        {
            ymodem_code = _rym_trans_fin();

            if (0 == ymodem_code)
            {
                ymodem_ota_task_state = RYM_STAGE_RESET;
                TMR_vSetTime(TMR_YMODEM_POLLING, TMR_TIME_MS2TICKS(40));
            }

            break;
        }

        case RYM_STAGE_RESET: //RYM服务阶段
        {
            if (TMR_bIsTimeExpired(TMR_YMODEM_POLLING))
            {
                hw_cpu_reset();
            }

            break;
        }

        case RYM_STAGE_IDLE: //RYM空闲阶段
        {

            break;
        }

        default:break;
    }

}

void ymodem_ota_set_received_data(uint32_t msg_len, uint8_t* msg_data)
{
    ymodem_ota_msg.msg_len = (msg_len <= YMODEM_DMA_SIZE) ? msg_len : YMODEM_DMA_SIZE;

    (void)memcpy(ymodem_ota_msg.msg_data, msg_data, ymodem_ota_msg.msg_len);
}

void ymodem_start_ota(void)
{
    ymodem_ota_sem = 1;
}

void ymodem_ota_task_init(void)
{
    ymodem_ota_sem = 1;
    ymodem_ota_task_state = RYM_STAGE_ESTABLISHING;
}

uint8_t ymodem_is_ota_data_received(void)
{
    return (ymodem_ota_task_state > RYM_STAGE_ESTABLISHING);
}