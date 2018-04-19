/*
 * Key.c
 *
 *  Created on: 2018年3月29日
 *      Author: Du
 */

/* User include */
#include "Key.h"
#include "pcc_hal.h"
#include "port_hal.h"
#include "gpio_hal.h"
#include "interrupt_manager.h"
#include "osif.h"

#include "mainloop.h"
#include "initialization.h"

/* End of user include */

/* Functions definition */

/**
 * \brief   按键IO初始化
 */
void key_PORT_init(void)
{
    PCC_HAL_SetClockMode(PCC, PCC_PORTE_CLOCK, true);
    PORT_HAL_SetMuxModeSel(PORTE, 7, 1);
    GPIO_HAL_SetPinDirection(PTE, 7, 0);
    PORT_HAL_SetPinIntSel(PORTE, 7, PORT_INT_EITHER_EDGE);
}

/**
 * \brief   按键中断初始化
 */
void key_NVIC_init()
{
    INT_SYS_EnableIRQ(PORTE_IRQn);
    INT_SYS_SetPriority(PORTE_IRQn, 0x1);
}

/**
 * \brief   按键初始化
 */
void key_init(void)
{
    key_PORT_init();
    key_NVIC_init();
}

/**
 * \brief   PORTE中断服务函数
 */
void PORTE_IRQHandler()
{
    static uint32_t press_ticks = 0;
    PORT_HAL_ClearPinIntFlagCmd(PORTE, 7);

    if (GPIO_HAL_ReadPins(PTE) & (0x01<<7)) {   /* 按键释放状态  */
        uint32_t free_ticks = OSIF_GetMilliseconds();

        if ( ((press_ticks < free_ticks) && ((free_ticks - press_ticks) > 3000))
                || ((press_ticks > free_ticks) && press_ticks - free_ticks < 0xfffff447)) {
            /* 按键按下和释放之间时间差大于3s */
            set_to_default_config();
        } else if ( ((press_ticks < free_ticks) && ((free_ticks - press_ticks) > 10))
                || ((press_ticks > free_ticks) && press_ticks - free_ticks < 0xfffffff5)) {
            /* 按键按下和释放之间时间差大于10ms */
            change_mode();
        }
    } else {                                    /* 按键按下状态  */
        /* 按键按下时获取滴答定时器时间,用于判断按键按下时间 */
        press_ticks = OSIF_GetMilliseconds();
    }
}
/* End of functions definition */
