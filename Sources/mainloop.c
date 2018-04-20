/*
 * mainloop.c
 *
 *  Created on: 2018��4��8��
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
 * 0:����͸��ģʽ
 * 1:��������ģʽ
 */
uint8_t mode = 0;

static void transparent_transmission(void);
static void lpuart_to_can(void);
static void can_to_lpuart(void);

/*
 * \brief   ��ѭ��, ��ʼ����ɺ�����ѭ��, ��������������˳�
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
 * \brief   �����ڽ��ջ�������������CANת��
 */
static void lpuart_to_can(void) {
    lpuart_rx_frame_t *frame;
    if (fifo_get_front_data(&g_lpuart_rx_fifo, (void**)&frame) != STATUS_SUCCESS) {
        /* �����������û������,�������� */
        return;
    }
    /* CAN�ķ���ģʽ��0 */
    if (g_m_flexcan_config.txmode == 0) {
        if (FLEXCAN_DRV_GetTransferStatus(INST_CANCOM0, TRANSMIT_STD_MB)
                != STATUS_BUSY) {   /* CAN���ڷ�æ״̬,��CAN�����е������Ѿ�������� */
            g_tx_info.msg_id_type = FLEXCAN_MSG_ID_STD;
            g_tx_info.data_length = (frame->descriptor & 0x7f) % 9;
            FLEXCAN_DRV_Send(INST_CANCOM0, TRANSMIT_STD_MB, &g_tx_info,
                    g_m_flexcan_config.tx_id, frame->data);
            fifo_release(&g_lpuart_rx_fifo);  /* ���ݷ������, �ͷŻ����� */
        }
    }

}

/*
 * \brief   ��CAN���ջ������������򴮿�ת��
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
 * \brief   ת������
 */
static void transparent_transmission(void) {
    lpuart_to_can();
    can_to_lpuart();
}

/*
 * \brief   �ı乤��ģʽ, ͸��ģʽ��������ģʽ
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
