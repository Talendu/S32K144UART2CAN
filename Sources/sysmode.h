/*
 * sysmode.h
 *
 *  Created on: 2018��4��20��
 *      Author: Administrator
 */

#ifndef SYSMODE_H_
#define SYSMODE_H_

#include "S32K144.h"
#include "null.h"
#include "pin_mux.h"

#include "mlpuart.h"
#include "mflexcan.h"

/**
 * \brief   ϵͳ����ģʽ��ö��
 */
typedef enum {
    SYSTEM_MODE_TRANSMISSION = 0,   /**< brief  ����ģʽ */
    SYSTEM_MODE_CONFIGURATION,      /**< brief  ����ģʽ */
}system_mode_t;

/*
 * \brief   �ı乤��ģʽ, ͸��ģʽ��������ģʽ
 */
void change_mode(void);

/**
 * \brief   ��ȡϵͳ����ģʽ
 */
system_mode_t get_system_mode(void);

#endif /* SYSMODE_H_ */
