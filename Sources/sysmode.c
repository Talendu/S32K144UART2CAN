/*
 * sysmode.c
 *
 *  Created on: 2018��4��20��
 *      Author: Administrator
 */

#include "sysmode.h"

/**
 * \brief   ϵͳ����ģʽ
 *
 * \note    �ڲ�����,��ͨ��get_system_mode()������ȡ
 */
system_mode_t __g_sysetm_mode = SYSTEM_MODE_TRANSMISSION;

/*
 * \brief   �ı乤��ģʽ, ͸��ģʽ��������ģʽ
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
 * \brief   ��ȡϵͳ����ģʽ
 */
system_mode_t get_system_mode(void) {
    return __g_sysetm_mode;
}
