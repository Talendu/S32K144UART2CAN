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

extern uint16_t g_uart_rx_sta;       //����״̬���
extern uint8_t  g_uart_rx_buff[];

extern const lpuart_user_config_t g_lpuart_default_UserConfig;

extern fifo_t g_lpuart_rx_fifo;


/**
 * \brief   ͸�����䴮���жϻص�����
 */
void lpuart_RX_callback_transmission(void);

/**
 * \brief   �������ò���ģʽʱ,�����жϻص�����
 *
 * \note    ����ָ��ʱ,ָ�������"\r\n����"
 */
void lpuart_RX_callback_config(void);

/**
 * \brief   ��ʼ������
 *
 * \param   lpuartUserConfig    ����������Ϣ
 */
status_t LPUART0_init(lpuart_user_config_t *p_lpuartUserConfig);

/**
 * \brief   ���ô��ڲ�����
 *
 * \param   baud    Ҫ���õĲ�����
 *
 * \retval  STATUS_SUCCESS  ���óɹ�
 *          STATUS_ERROR    ����ʧ��
 */
status_t LPUART0_set_baud(uint32_t baud);

/**
 * \brief   ��ò�����
 *
 * \retval  ���ڲ�����
 *
 * \note    �ú��������ش���0��ǰʵ�ʵĲ�����,�ͱ�׼������֮�������
 */
uint32_t LPUART0_get_baud(void);

/**
 * \brief   �Ӵ���0����һ���ַ�����
 */
void LPUART0_transmit_char(char send);

/**
 * \brief   �Ӵ���0��������
 *
 * \param   buffer[in]  Ҫ���͵�����
 * \param   len[in]     Ҫ���͵����ݵĳ���
 */
void LPUART0_trancemit_buffer(const uint8_t *buffer, uint32_t len);

/**
 * \brief   ���ڴ�ӡ�ַ���
 *
 * \param   data_string[in]     Ҫ��ӡ���ַ���
 *
 * \note    �ַ���������'\0'����
 */
void LPUART0_transmit_string(const char data_string[]);

/**
 * \brief   ���ڽ���һ���ַ�
 */
char LPUART0_receive_char(void);

/*
 * \brief   ���������ַ�������ʽͨ�����ڷ���
 *
 * \param   LPUARTx     ����һ�����ڷ���
 * \param   number      Ҫ���͵�����
 *
 * \retval  ���ֳ���,������λ��
 *
 * \note    ֻ�ܷ���32λ�з������ַ�Χ�������
 */
uint8_t LPUART_transmit_number(LPUART_Type *LPUARTx, int32_t number);

/**
 * \brief   Ϊ�������ý����жϻص�����
 *
 * \param   callback �ص�����
 */
void LPUART_InstallRxCallback(lpuart0_irq_callback_t callback);

#endif /* MLPUART_H_ */
