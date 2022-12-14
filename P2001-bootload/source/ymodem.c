/*
 * COPYRIGHT (C) 2012, Real-Thread Information Technology Ltd
 * All rights reserved
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-04-14     Grissiom     initial implementation
 */

#include "ymodem.h"
#include "sw_timer.h"
#include "UART/uart.h"




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
uint16_t CRC16(unsigned char *q, int len)
{
    uint16_t crc = 0;

    while (len-- > 0)
        crc = (crc << 8) ^ ccitt_table[((crc >> 8) ^ *q++) & 0xff];
    return crc;
}


static int32_t _rym_rx_ind(rt_device_t dev, rt_size_t size)
{
    return rt_sem_release(&_rym_the_ctx->sem);
}

static enum rym_code _rym_read_code(
        struct rym_ctx *ctx,
        rt_tick_t timeout)
{
    /* Fast path */
    if (rt_device_read(ctx->dev, 0, ctx->buf, 1) == 1)
        return (enum rym_code)(*ctx->buf);

    /* Slow path */
    do {
        rt_size_t rsz;

        /* No data yet, wait for one */
        if (rt_sem_take(&ctx->sem, timeout) != RT_EOK)
            return RYM_CODE_NONE;

        /* Try to read one */
        rsz = rt_device_read(ctx->dev, 0, ctx->buf, 1);
        if (rsz == 1)
            return (enum rym_code)(*ctx->buf);
    } while (1);
}

/* the caller should at least alloc _RYM_STX_PKG_SZ buffer */
static rt_size_t _rym_read_data(
        struct rym_ctx *ctx,
        rt_size_t len)
{
    /* we should already have had the code */
    rt_uint8_t *buf = ctx->buf + 1;
    rt_size_t readlen = 0;

    do
    {
        readlen += rt_device_read(ctx->dev,
                0, buf+readlen, len-readlen);
        if (readlen >= len)
            return readlen;
    } while (rt_sem_take(&ctx->sem, RYM_WAIT_CHR_TICK) == RT_EOK);

    return readlen;
}








static int32_t _rym_do_recv(
        struct rym_ctx *ctx,
        int handshake_timeout)
{
    int32_t err;

    ctx->stage = RYM_STAGE_NONE;

    ctx->buf = rt_malloc(_RYM_STX_PKG_SZ);
    if (ctx->buf == RT_NULL)
        return -RT_ENOMEM;

    err = _rym_do_handshake(ctx, handshake_timeout);
    if (err != RT_EOK)
        return err;
    while (1)
    {
        err = _rym_do_trans(ctx);
        if (err != RT_EOK)
            return err;

        err = _rym_do_fin(ctx);
        if (err != RT_EOK)
            return err;
        if (ctx->stage == RYM_STAGE_FINISHED)
            break;
    }
    return err;
}

int32_t rym_recv_on_device(
        struct rym_ctx *ctx,
        rt_device_t dev,
        uint16_t oflag,
        rym_callback on_begin,
        rym_callback on_data,
        rym_callback on_end,
        int handshake_timeout)
{
    int32_t res;
    int32_t (*odev_rx_ind)(rt_device_t dev, rt_size_t size);
    uint16_t odev_flag;
    int int_lvl;

    RT_ASSERT(_rym_the_ctx == 0);
    _rym_the_ctx = ctx;

    ctx->on_begin = on_begin;
    ctx->on_data  = on_data;
    ctx->on_end   = on_end;
    ctx->dev      = dev;
    rt_sem_init(&ctx->sem, "rymsem", 0, RT_IPC_FLAG_FIFO);

    odev_rx_ind = dev->rx_indicate;
    /* no data should be received before the device has been fully setted up.
     */
    int_lvl = rt_hw_interrupt_disable();
    rt_device_set_rx_indicate(dev, _rym_rx_ind);

    odev_flag = dev->flag;
    /* make sure the device don't change the content. */
    dev->flag &= ~RT_DEVICE_FLAG_STREAM;
    rt_hw_interrupt_enable(int_lvl);

    res = rt_device_open(dev, oflag);
    if (res != RT_EOK)
        goto __exit;

    res = _rym_do_recv(ctx, handshake_timeout);

    rt_device_close(dev);

__exit:
    /* no rx_ind should be called before the callback has been fully detached.
     */
    int_lvl = rt_hw_interrupt_disable();
    rt_sem_detach(&ctx->sem);

    dev->flag = odev_flag;
    rt_device_set_rx_indicate(dev, odev_rx_ind);
    rt_hw_interrupt_enable(int_lvl);

    rt_free(ctx->buf);
    _rym_the_ctx = RT_NULL;

    return res;
}
