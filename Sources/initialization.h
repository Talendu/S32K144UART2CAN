/*
 * initialization.h
 *
 *  Created on: 2018��4��7��
 *      Author: Administrator
 */

#ifndef INITIALIZATION_H_
#define INITIALIZATION_H_

#include "S32K144.h"
#include "clockMan1.h"
#include "canCom0.h"
#include "pin_mux.h"
#include "dmaController0.h"
#include "osif.h"
#include "Key.h"
#include "mlpuart.h"
#include "mflexcan.h"
#include "mflash.h"
#include "crc.h"


#define CONFIG_INFO_OFFSET 0


/**
 * \brief   ������Ϣ�洢�ṹ
 * \note    ������Ϣ���Ըýṹ�洢��EEPROM��
 */
typedef struct {
    m_lpuart_config_t m_lpuart_config;
    m_flexcan_config_t m_flexcan_config;
    uint16_t crc;
}config_info_t;

/**
 * \brief   ��ʼ��ϵͳ
 */
void init_all(void);

/**
 * \brief   ��ʼ��CAN
 */
void init_flexcan(void);

/**
 * \brief   ��ʼ������
 */
void init_lpuart(void);

/**
 * \brief   ��ȡĬ��������Ϣ
 */
void get_default_config_info(void);

void set_to_default_config(void);

#endif /* INITIALIZATION_H_ */
