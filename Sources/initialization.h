/*
 * initialization.h
 *
 *  Created on: 2018年4月7日
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
 * \brief   配置信息存储结构
 * \note    配置信息将以该结构存储在EEPROM中
 */
typedef struct {
    m_lpuart_config_t m_lpuart_config;
    m_flexcan_config_t m_flexcan_config;
    uint16_t crc;
}config_info_t;

/**
 * \brief   初始化系统
 */
void init_all(void);

/**
 * \brief   初始化CAN
 */
void init_flexcan(void);

/**
 * \brief   初始化串口
 */
void init_lpuart(void);

/**
 * \brief   获取默认配置信息
 */
void get_default_config_info(void);

void set_to_default_config(void);

#endif /* INITIALIZATION_H_ */
