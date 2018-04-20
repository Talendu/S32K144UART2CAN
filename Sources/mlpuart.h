/*
 * mlpuart.h
 *
 *  Created on: 2018��4��17��
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

/* \brief   �����յ�����ʱ,���������������,��8���ֽڷ�Ϊһ����
 * \member  descriptor  ��������,  bit[7]=1 �������; bit[7]=0 ������
 *                              bit[6:0] ���ݳ���,��Чֵ0~8
 */
typedef struct {
    uint8_t descriptor;
    uint8_t data[8];
}lpuart_rx_frame_t;


typedef void (*lpuart0_irq_callback_t)(void);

//extern uint8_t lpuart_rx_len;

extern uint16_t g_uart_rx_sta;       //����״̬���
extern uint8_t  g_uart_rx_buff[];

extern const lpuart_user_config_t g_lpuart_default_UserConfig;

extern fifo_t g_lpuart_rx_fifo;
void lpuart_RX_callback_transparent_transmission();
void lpuart_RX_callback_configuration_parameters();

status_t LPUART0_init(lpuart_user_config_t *p_lpuartUserConfig);

status_t LPUART0_set_baud(uint32_t baud);
uint32_t LPUART0_get_baud(void);

void LPUART0_transmit_char(char send);
void LPUART0_trancemit_buffer(const uint8_t *buffer, uint32_t len);
void LPUART0_transmit_string(const char data_string[]);
char LPUART0_receive_char(void);
uint8_t LPUART_transmit_number(LPUART_Type *LPUARTx, int32_t number);

void LPUART_InstallRxCallback(lpuart0_irq_callback_t callback);

#endif /* MLPUART_H_ */
