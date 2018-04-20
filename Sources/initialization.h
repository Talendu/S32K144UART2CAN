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
#include "config.h"


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

/**
 * \brief   ����ΪĬ��������Ϣ
 */
void set_to_default_config(void);

#endif /* INITIALIZATION_H_ */
