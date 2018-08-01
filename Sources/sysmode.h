/*
 * sysmode.h
 *
 *  Created on: 2018年4月20日
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
 * \brief   系统工作模式的枚举
 */
typedef enum {
    SYSTEM_MODE_TRANSMISSION = 0,   /**< brief  传输模式 */
    SYSTEM_MODE_CONFIGURATION,      /**< brief  配置模式 */
}system_mode_t;

/*
 * \brief   改变工作模式, 透传模式或者配置模式
 */
void change_mode(void);

/**
 * \brief   获取系统工作模式
 */
system_mode_t get_system_mode(void);

#endif /* SYSMODE_H_ */
