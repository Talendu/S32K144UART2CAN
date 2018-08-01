/*
 * sysmode.c
 *
 *  Created on: 2018年4月20日
 *      Author: Administrator
 */

#include "sysmode.h"

/**
 * \brief   系统工作模式
 *
 * \note    内部变量,可通过get_system_mode()函数获取
 */
system_mode_t __g_sysetm_mode = SYSTEM_MODE_TRANSMISSION;

/*
 * \brief   改变工作模式, 透传模式或者配置模式
 */
void change_mode() {
    if (__g_sysetm_mode == SYSTEM_MODE_TRANSMISSION) {
        __g_sysetm_mode = SYSTEM_MODE_CONFIGURATION;
        GPIO_HAL_ClearPins(PTC, 1<<12);
        LPUART_InstallRxCallback(lpuart_RX_callback_config);
        FLEXCAN_DRV_InstallEventCallback(0, can_callback_config, NULL);
    } else if(__g_sysetm_mode == SYSTEM_MODE_CONFIGURATION){
        __g_sysetm_mode = SYSTEM_MODE_TRANSMISSION;
        GPIO_HAL_SetPins(PTC, 1<<12);
        LPUART_InstallRxCallback(lpuart_RX_callback_transmission);
        FLEXCAN_DRV_InstallEventCallback(0, can_callback_transmission, NULL);
    }
}

/**
 * \brief   获取系统工作模式
 */
system_mode_t get_system_mode(void) {
    return __g_sysetm_mode;
}
