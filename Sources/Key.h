/*
 * Key.h
 *
 *  Created on: 2018年3月29日
 *      Author: Du
 */

#ifndef KEY_H_
#define KEY_H_

#include "S32K144.h"
#include "pcc_hal.h"
#include "pin_mux.h"
#include "interrupt_manager.h"
#include "osif.h"
#include "sysmode.h"
#include "config.h"

/**
 * \brief   按键初始化
 */
void key_init(void);

#endif /* KEY_H_ */
