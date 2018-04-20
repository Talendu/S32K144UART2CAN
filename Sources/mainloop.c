/*
 * mainloop.c
 *
 *  Created on: 2018年4月8日
 *      Author: Administrator
 */
#include "mainloop.h"
#include "clockMan1.h"
#include "canCom0.h"
#include "pin_mux.h"
#include "dmaController0.h"
#include "osif.h"
#include "Key.h"
#include "config.h"
#include "mlpuart.h"
#include "mflexcan.h"
#include "string.h"
#include "crc.h"
#include "mflash.h"
#include "initialization.h"

/**
 * 0:串口透传模式
 * 1:参数配置模式
 */
uint8_t mode = 0;

static void transparent_transmission(void);
static void lpuart_to_can(void);
static void can_to_lpuart(void);
static void config_by_lpuart(void);

/*
 * \brief   主循环, 初始化完成后进入该循环, 非意外情况不会退出
 */
void mainloop(void) {
    change_mode();
    while(1) {
        if (mode == 0) {
            transparent_transmission();
        } else if (mode ==1) {
            config_by_lpuart();
        }
    }
}

/*
 * \brief   将串口接收缓冲区的数据向CAN转发
 */
static void lpuart_to_can(void) {
    lpuart_rx_frame_t *frame;
    if (fifo_get_front_data(&lpuart_rx_fifo, (void**)&frame) != STATUS_SUCCESS) {
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
            fifo_release(&lpuart_rx_fifo);  /* 数据发送完成, 释放缓冲区 */
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

/*
 * \brief   通过串口配置或则打印参数
 * \note    指令格式 AT[+(item)[=(value)]]
 */
static void config_by_lpuart(void) {
    uint8_t index = 0;
    if (g_uart_rx_sta & 0x8000) { /* 串口接收完成 */
        GPIO_HAL_ClearPins(PTC, 1<<11);                   /* 关闭LED3,配置命令中 */
        g_uart_rx_buff[g_uart_rx_sta & 0x3fff] = '\0';     /* 将末尾变为0,使其成为字符串. */

        if(memcmp(g_uart_rx_buff, g_config_item[AT_index], 2)) {
            /* 指令必须是AT开头,换行(\r\n)结尾 */
            g_uart_rx_sta = 0;
            return;
        }
        index = 2;
        if (g_uart_rx_buff[index] != '+') {   /* AT后面不是'+' */
            if (g_uart_rx_buff[index] == '\0') {  /* 而是'\0': */
                LPUART0_transmit_string(g_statu_item[OK_index]); /* 返回OK; */
            } else {                            /* 否则: */
                LPUART0_transmit_string(g_statu_item[ERROR_index]); /* 返回OK. */
            }
            g_uart_rx_sta = 0;
            return;
        }

        int i = 0;
        index = 3;
        if (g_uart_rx_buff[index] == '@') {           /* +后面是@ */
            index += 2;                             /* 向后移动两位是指令描述符 */
        }                                           /* 否则不加 */
        for (i=3; i<CONFIG_CODES_COUNT; i++) {      /* 循环查找指令 */
            uint8_t config_code_len = strlen(g_config_item[i]);
            if(memcmp(g_uart_rx_buff+index, g_config_item[i], config_code_len) == 0) {
                index += config_code_len;           /* 下一个判定字符位置 */
                break;                              /* 如果指令匹配,跳出循环 */
            }
        }
        if (i == CONFIG_CODES_COUNT) { /* i=CONFIG_CODES_COUNT说明没有匹配的指令 */
            g_uart_rx_sta = 0;
            return;
        }
        if (g_uart_rx_buff[3] == '@') {                   /* +后面是@ */
            if (g_uart_rx_buff[4] == 'U') {                   /* 配置串口参数 */
                if (g_uart_rx_buff[index] == '='){
                    index++;
                    /* 配置串口参数 */
                    config_lpuart_by_lpuart(i, g_uart_rx_buff+index,
                            (g_uart_rx_sta & 0x3fff) - index);
                    /* 将参数保存到EEPROM */
                    save_config_paramater_to_EEPROM();
                } else {
                    LPUART0_print_lpuart_info(i);
                }
            } else if (g_uart_rx_buff[4] == 'C') {            /* 配置CAN参数 */
                if (g_uart_rx_buff[index] == '='){
                    index++;
                    /* 配置CAN参数 */
                    LPUART0_config_can_by(i, g_uart_rx_buff+index,
                            (g_uart_rx_sta & 0x3fff) - index);
                    /* 将参数保存到EEPROM */
                    save_config_paramater_to_EEPROM();
                } else {
                    LPUART0_print_can_config_by(i);
                }
            } else {
                g_uart_rx_sta = 0;
                return;
            }
        } else {

        }
        GPIO_HAL_SetPins(PTC, 1<<11);
        g_uart_rx_sta = 0;
    }
}

/*
 * \brief   保存参数到EEPROM
 */
void save_config_paramater_to_EEPROM() {
    if (flash_EEPROM_init() != STATUS_SUCCESS) {
        LPUART0_transmit_string("ERROR\r\n");
        return;
    } else {
        config_info_t paramater = {
                .m_lpuart_config = m_lpuart0_config,
                .m_flexcan_config = g_m_flexcan_config
        };
        CRC_DRV_Deinit(INST_CRC);
        CRC_DRV_Init(INST_CRC, &crc_InitConfig0);
        CRC_DRV_WriteData(INST_CRC, (uint8_t *)&paramater,
                sizeof(m_lpuart0_config) + sizeof(g_m_flexcan_config));
        paramater.crc = CRC_DRV_GetCrcResult(INST_CRC);
        flash_write_EEPROM(CONFIG_INFO_OFFSET, (uint8_t *)&paramater, sizeof(paramater));
    }
}

/*
 * \brief   改变工作模式, 透传模式或者配置模式
 */
void change_mode() {
    if (mode == 0) {
        mode = 1;
        GPIO_HAL_ClearPins(PTC, 1<<12);
        LPUART_InstallRxCallback(lpuart_RX_callback_configuration_parameters);
        FLEXCAN_DRV_InstallEventCallback(0, can_callback_config, NULL);
    } else {
        mode = 0;
        GPIO_HAL_SetPins(PTC, 1<<12);
        LPUART_InstallRxCallback(lpuart_RX_callback_transparent_transmission);
        FLEXCAN_DRV_InstallEventCallback(0, can_callback_transmission, NULL);
    }
}
