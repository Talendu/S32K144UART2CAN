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
#include "configcodes.h"
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
static void config_by_lpuart(void);

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
    if (fifo_get_front_data(&lpuart_rx_fifo, (void**)&frame) != STATUS_SUCCESS) {
        /* �����������û������,�������� */
        return;
    }
    /* CAN�ķ���ģʽ��0 */
    if (m_flexcan_config.txmode == 0) {
        if (FLEXCAN_DRV_GetTransferStatus(INST_CANCOM0, TRANSMIT_STD_MB)
                != STATUS_BUSY) {   /* CAN���ڷ�æ״̬,��CAN�����е������Ѿ�������� */
            tx_info.msg_id_type = FLEXCAN_MSG_ID_STD;
            tx_info.data_length = (frame->descriptor & 0x7f) % 9;
            FLEXCAN_DRV_Send(INST_CANCOM0, TRANSMIT_STD_MB, &tx_info,
                    m_flexcan_config.tx_id, frame->data);
            fifo_release(&lpuart_rx_fifo);  /* ���ݷ������, �ͷŻ����� */
        }
    }

}

/*
 * \brief   ��CAN���ջ������������򴮿�ת��
 */
static void can_to_lpuart(void) {
    if (LPUART0->STAT & LPUART_STAT_TDRE_MASK) {
        uint8_t *flexcan_data;
        if (fifo_get_front_data(&flexcan_rx_fifo, (void**)&flexcan_data)
                == STATUS_SUCCESS) {
            LPUART0->DATA = *flexcan_data;
            fifo_release(&flexcan_rx_fifo);
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
 * \brief   ͨ���������û����ӡ����
 * \note    ָ���ʽ AT[+(item)[=(value)]]
 */
static void config_by_lpuart(void) {
    uint8_t index = 0;
    if (USART_RX_STA & 0x8000) { /* ���ڽ������ */
        GPIO_HAL_ClearPins(PTC, 1<<11);                   /* �ر�LED3,���������� */
        USART_RX_BUF[USART_RX_STA & 0x3fff] = '\0';     /* ��ĩβ��Ϊ0,ʹ���Ϊ�ַ���. */

        if(memcmp(USART_RX_BUF, config_item[AT_index], 2)) {
            /* ָ�������AT��ͷ,����(\r\n)��β */
            USART_RX_STA = 0;
            return;
        }
        index = 2;
        if (USART_RX_BUF[index] != '+') {   /* AT���治��'+' */
            if (USART_RX_BUF[index] == '\0') {  /* ����'\0': */
                LPUART0_transmit_string(statu_item[OK_index]); /* ����OK; */
            } else {                            /* ����: */
                LPUART0_transmit_string(statu_item[ERROR_index]); /* ����OK. */
            }
            USART_RX_STA = 0;
            return;
        }

        int i = 0;
        index = 3;
        if (USART_RX_BUF[index] == '@') {           /* +������@ */
            index += 2;                             /* ����ƶ���λ��ָ�������� */
        }                                           /* ���򲻼� */
        for (i=3; i<CONFIG_CODES_COUNT; i++) {      /* ѭ������ָ�� */
            uint8_t config_code_len = strlen(config_item[i]);
            if(memcmp(USART_RX_BUF+index, config_item[i], config_code_len) == 0) {
                index += config_code_len;           /* ��һ���ж��ַ�λ�� */
                break;                              /* ���ָ��ƥ��,����ѭ�� */
            }
        }
        if (i == CONFIG_CODES_COUNT) { /* i=CONFIG_CODES_COUNT˵��û��ƥ���ָ�� */
            USART_RX_STA = 0;
            return;
        }
//        LPUART0_transmit_string(USART_RX_BUF + 3);
        if (USART_RX_BUF[3] == '@') {                   /* +������@ */
            if (USART_RX_BUF[4] == 'U') {                   /* ���ô��ڲ��� */
                if (USART_RX_BUF[index] == '='){
                    index++;
                    /* ���ô��ڲ��� */
                    config_lpuart_by_lpuart(i, USART_RX_BUF+index,
                            (USART_RX_STA & 0x3fff) - index);
                    /* ���������浽EEPROM */
                    save_config_paramater_to_EEPROM();
                } else {
                    LPUART0_print_lpuart_info(i);
                }
            } else if (USART_RX_BUF[4] == 'C') {            /* ����CAN���� */
                if (USART_RX_BUF[index] == '='){
                    index++;
                    /* ����CAN���� */
                    config_can_by_lpuart(i, USART_RX_BUF+index,
                            (USART_RX_STA & 0x3fff) - index);
                    /* ���������浽EEPROM */
                    save_config_paramater_to_EEPROM();
                } else {
                    print_can_config_by_lpuart(i);
                }
            } else {
                USART_RX_STA = 0;
                return;
            }
        } else {

        }
        GPIO_HAL_SetPins(PTC, 1<<11);
        USART_RX_STA = 0;
    }
}

/*
 * \brief   ���������EEPROM
 */
void save_config_paramater_to_EEPROM() {
    if (flash_EEPROM_init() != STATUS_SUCCESS) {
        LPUART0_transmit_string("ERROR\r\n");
        return;
    } else {
        config_info_t paramater = {
                .m_lpuart_config = m_lpuart0_config,
                .m_flexcan_config = m_flexcan_config
        };
        CRC_DRV_Deinit(INST_CRC);
        CRC_DRV_Init(INST_CRC, &crc_InitConfig0);
        CRC_DRV_WriteData(INST_CRC, (uint8_t *)&paramater,
                sizeof(m_lpuart0_config) + sizeof(m_flexcan_config));
        paramater.crc = CRC_DRV_GetCrcResult(INST_CRC);
        flash_write_EEPROM(CONFIG_INFO_OFFSET, (uint8_t *)&paramater, sizeof(paramater));
    }
}

/*
 * \brief   �ı乤��ģʽ, ͸��ģʽ��������ģʽ
 */
void change_mode() {
    if (mode == 0) {
        mode = 1;
        GPIO_HAL_ClearPins(PTC, 1<<12);
        LPUART_InstallRxCallback(lpuart_RX_callback_configuration_parameters);
        FLEXCAN_DRV_InstallEventCallback(0, can_callback_configuration_parameters, NULL);
    } else {
        mode = 0;
        GPIO_HAL_SetPins(PTC, 1<<12);
        LPUART_InstallRxCallback(lpuart_RX_callback_transparent_transmission);
        FLEXCAN_DRV_InstallEventCallback(0, can_callback_transparent_transmission, NULL);
    }
}