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
#include "configcodes.h"
#include "lpuart_hal.h"
#include "gpio_hal.h"
#include "mainloop.h"

typedef struct {
    lpuart_user_config_t lpuart0_user_config;
    uint8_t txmode;
    uint8_t rxmode;
}m_lpuart_config_t;

/* \brief   串口收到数据时,如果数据是连续的,将8个字节分为一个包
 * \member  descriptor  数据描述,  bit[7]=1 接收完成; bit[7]=0 接收中
 *                              bit[6:0] 数据长度,有效值0~8
 */
typedef struct {
    uint8_t descriptor;
    uint8_t data[8];
}lpuart_rx_frame_t;


typedef void (*lpuart0_irq_callback_t)(void);

extern uint8_t lpuart_rx_len;

extern uint16_t USART_RX_STA;       //接收状态标记
extern uint8_t USART_RX_BUF[];

extern m_lpuart_config_t m_lpuart0_config;
extern const lpuart_user_config_t g_lpuart_default_UserConfig;

extern fifo_t lpuart_rx_fifo;
void lpuart_RX_callback_transparent_transmission();
void lpuart_RX_callback_configuration_parameters();

status_t LPUART0_init(lpuart_user_config_t *lpuartUserConfig);

void LPUART0_print_option_status(device_item_index_t device_code_index,
        config_item_index_t config_code_index,
        statu_item_t status_code_index);

void LPUART0_print_config_parameter(device_item_index_t device_code_index,
                                   config_item_index_t config_code_index,
                                   uint32_t            parameter);

void config_lpuart_by_lpuart(uint8_t config_code_index, uint8_t *parameter, uint16_t parameter_len);
void LPUART0_print_lpuart_info(uint8_t config_code_index);

status_t LPUART0_set_baud(uint32_t baud);
uint32_t LPUART0_get_baud(void);

void LPUART0_transmit_char(char send);
void LPUART0_trancemit_buffer(uint8_t *buffer, uint32_t len);
void LPUART0_transmit_string(char data_string[]);
char LPUART0_receive_char(void);
uint8_t LPUART_transmit_number(LPUART_Type *LPUARTx, int32_t number);

void LPUART_InstallRxCallback(lpuart0_irq_callback_t callback);

#endif /* MLPUART_H_ */
