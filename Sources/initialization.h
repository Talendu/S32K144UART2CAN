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
#include "config.h"


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

/**
 * \brief   设置为默认配置信息
 */
void set_to_default_config(void);

#endif /* INITIALIZATION_H_ */
