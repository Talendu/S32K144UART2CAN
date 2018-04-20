/*
 * flexcan.h
 *
 *  Created on: 2018��4��8��
 *      Author: Administrator
 */

#ifndef MFLEXCAN_H_
#define MFLEXCAN_H_

#include "S32K144.h"
#include "clockMan1.h"
#include "canCom0.h"
#include "pin_mux.h"
#include "dmaController0.h"
#include "osif.h"
#include "mlpuart.h"
#include "config.h"
#include "fifo.h"

#define RECEIVE_STD_MB  8       /**< \brief CAN��׼֡���������ַ */
#define RECEIVE_EXT_MB  9       /**< \brief CAN��չ֡���������ַ */

#define TRANSMIT_STD_MB 10      /**< \brief CAN��׼֡���������ַ */
#define TRANSMIT_EXT_MB 11      /**< \brief CAN��չ֡���������ַ */

#define __FLEXCAN_RX_FIFO_SIZE    (8*1024)  /**< \brief CAN�������ݻ������ڴ���С */
#define FLEXCAN_RX_DATA_TYPE uint8_t        /**< \brief CAN�������ݴ洢���� */


extern flexcan_msgbuff_t    g_can_receive_buff;
extern uint8_t              g_lpuart_receive_buff[];

extern flexcan_data_info_t  g_rx_info;
extern flexcan_data_info_t  g_tx_info;
extern fifo_t               g_flexcan_rx_fifo;

/**
 * \brief   ͸������ģʽCAN���ջص�����
 * \param   instance    CANͨ��
 * \param   eventType   �ж�����
 * \param   state       CANͨ��״̬��Ϣ
 */
void can_callback_transmission(uint8_t               instance,
                               flexcan_event_type_t  eventType,
                               flexcan_state_t      *state);

/**
 * \brief   ��������ģʽCAN���ջص�����
 * \param   instance    CANͨ��
 * \param   eventType   �ж�����
 * \param   state       CANͨ��״̬��Ϣ
 */
void can_callback_config(uint8_t               instance,
                         flexcan_event_type_t  eventType,
                         flexcan_state_t      *state);

/**
 * \brief   ��ʼ��CAN
 */
void init_flexcan(void);

/**
 * \brief       ��ȡcan0ʱ��Ƶ��
 * \param[out]  p_flexcanSourceClock[out]   ʱ��Ƶ��
 */
void flexcan_get_source_clock(uint32_t *flexcanSourceClock);

/**
 * \brief   ����CAN������
 * \param   p_parameter[in]     ��'\0'�������ַ���(ʮ���ƻ�ʮ������)
 */
status_t flexcan_set_baud(const uint8_t *p_parameter);


#endif /* MFLEXCAN_H_ */
