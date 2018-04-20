/*
 * mlpuart.h
 *
 *  Created on: 2018年4月17日
 *      Author: Administrator
 */

#ifndef MLPUART_H_
#define MLPUART_H_
#include "S32K144.h"
#include "lpuart_driver.h"
#include "fifo.h"
#include "config.h"
#include "lpuart_hal.h"
#include "gpio_hal.h"
#include "mainloop.h"

/* \brief   串口收到数据时,如果数据是连续的,将8个字节分为一个包
 * \member  descriptor  数据描述,  bit[7]=1 接收完成; bit[7]=0 接收中
 *                              bit[6:0] 数据长度,有效值0~8
 */
typedef struct {
    uint8_t descriptor;
    uint8_t data[8];
}lpuart_rx_frame_t;


typedef void (*lpuart0_irq_callback_t)(void);

extern uint16_t g_uart_rx_sta;       //接收状态标记
extern uint8_t  g_uart_rx_buff[];

extern const lpuart_user_config_t g_lpuart_default_UserConfig;

extern fifo_t g_lpuart_rx_fifo;


/**
 * \brief   透明传输串口中断回掉函数
 */
void lpuart_RX_callback_transmission(void);

/**
 * \brief   处于配置参数模式时,串口中断回掉函数
 *
 * \note    接收指令时,指令必须由"\r\n结束"
 */
void lpuart_RX_callback_config(void);

/**
 * \brief   初始化串口
 *
 * \param   lpuartUserConfig    串口配置信息
 */
status_t LPUART0_init(lpuart_user_config_t *p_lpuartUserConfig);

/**
 * \brief   设置串口波特率
 *
 * \param   baud    要设置的波特率
 *
 * \retval  STATUS_SUCCESS  设置成功
 *          STATUS_ERROR    设置失败
 */
status_t LPUART0_set_baud(uint32_t baud);

/**
 * \brief   获得波特率
 *
 * \retval  串口波特率
 *
 * \note    该函数将返回串口0当前实际的波特率,和标准波特率之间有误差
 */
uint32_t LPUART0_get_baud(void);

/**
 * \brief   从串口0发送一个字符数据
 */
void LPUART0_transmit_char(char send);

/**
 * \brief   从串口0发送数据
 *
 * \param   buffer[in]  要发送的数据
 * \param   len[in]     要发送的数据的长度
 */
void LPUART0_trancemit_buffer(const uint8_t *buffer, uint32_t len);

/**
 * \brief   串口打印字符串
 *
 * \param   data_string[in]     要打印的字符串
 *
 * \note    字符串必须以'\0'结束
 */
void LPUART0_transmit_string(const char data_string[]);

/**
 * \brief   串口接收一个字符
 */
char LPUART0_receive_char(void);

/*
 * \brief   将数字以字符串的形式通过串口发送
 *
 * \param   LPUARTx     向哪一个串口发送
 * \param   number      要发送的数字
 *
 * \retval  数字长度,及数字位数
 *
 * \note    只能发送32位有符号数字范围类的数字
 */
uint8_t LPUART_transmit_number(LPUART_Type *LPUARTx, int32_t number);

/**
 * \brief   为串口设置接收中断回掉函数
 *
 * \param   callback 回掉函数
 */
void LPUART_InstallRxCallback(lpuart0_irq_callback_t callback);

#endif /* MLPUART_H_ */
