/*
 * mainloop.c
 *
 *  Created on: 2018年4月8日
 *      Author: Administrator
 */
#include "mainloop.h"

static void transparent_transmission(void);
static void lpuart_to_can(void);
static void can_to_lpuart(void);

/*
 * \brief   主循环, 初始化完成后进入该循环, 非意外情况不会退出
 */
void mainloop(void) {
    change_mode();
    while(1) {
        if (get_system_mode() == 0) {
            transparent_transmission();
        } else if (get_system_mode() ==1) {
            config_by_lpuart();
        }
    }
}

/*
 * \brief   将串口接收缓冲区的数据向CAN转发
 */
static void lpuart_to_can(void) {
    lpuart_rx_frame_t *frame;
    if (fifo_get_front_data(&g_lpuart_rx_fifo, (void**)&frame) != STATUS_SUCCESS) {
        /* 如果缓冲区中没有数据,函数返回 */
        return;
    }
    /* CAN的发送模式是0 */
    if (g_m_flexcan_config.txmode == 0) {
        if (FLEXCAN_DRV_GetTransferStatus(INST_CANCOM0, TRANSMIT_STD_MB)
                != STATUS_BUSY) {   /* CAN处于非忙状态,及CAN邮箱中的数据已经发送完成 */
            g_tx_info.msg_id_type = FLEXCAN_MSG_ID_STD;
            g_tx_info.data_length = (frame->descriptor & 0x7f) % 9;
            FLEXCAN_DRV_Send(INST_CANCOM0, TRANSMIT_STD_MB, &g_tx_info,
                    g_m_flexcan_config.tx_id, frame->data);
            fifo_release(&g_lpuart_rx_fifo);  /* 数据发送完成, 释放缓冲区 */
        }
    }

}

/*
 * \brief   将CAN接收缓冲区的数据向串口转发
 */
static void can_to_lpuart(void) {
    if (LPUART0->STAT & LPUART_STAT_TDRE_MASK) {
        uint8_t *flexcan_data;
        if (fifo_get_front_data(&g_flexcan_rx_fifo, (void**)&flexcan_data)
                == STATUS_SUCCESS) {
            LPUART0->DATA = *flexcan_data;
            fifo_release(&g_flexcan_rx_fifo);
        }
    }
}

/*
 * \brief   转发数据
 */
static void transparent_transmission(void) {
    lpuart_to_can();
    can_to_lpuart();
}

