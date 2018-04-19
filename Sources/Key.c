/*
 * Key.c
 *
 *  Created on: 2018��3��29��
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
 * \brief   ����IO��ʼ��
 */
void key_PORT_init(void)
{
    PCC_HAL_SetClockMode(PCC, PCC_PORTE_CLOCK, true);
    PORT_HAL_SetMuxModeSel(PORTE, 7, 1);
    GPIO_HAL_SetPinDirection(PTE, 7, 0);
    PORT_HAL_SetPinIntSel(PORTE, 7, PORT_INT_EITHER_EDGE);
}

/**
 * \brief   �����жϳ�ʼ��
 */
void key_NVIC_init()
{
    INT_SYS_EnableIRQ(PORTE_IRQn);
    INT_SYS_SetPriority(PORTE_IRQn, 0x1);
}

/**
 * \brief   ������ʼ��
 */
void key_init(void)
{
    key_PORT_init();
    key_NVIC_init();
}

/**
 * \brief   PORTE�жϷ�����
 */
void PORTE_IRQHandler()
{
    static uint32_t press_ticks = 0;
    PORT_HAL_ClearPinIntFlagCmd(PORTE, 7);

    if (GPIO_HAL_ReadPins(PTE) & (0x01<<7)) {   /* �����ͷ�״̬  */
        uint32_t free_ticks = OSIF_GetMilliseconds();

        if ( ((press_ticks < free_ticks) && ((free_ticks - press_ticks) > 3000))
                || ((press_ticks > free_ticks) && press_ticks - free_ticks < 0xfffff447)) {
            /* �������º��ͷ�֮��ʱ������3s */
            set_to_default_config();
        } else if ( ((press_ticks < free_ticks) && ((free_ticks - press_ticks) > 10))
                || ((press_ticks > free_ticks) && press_ticks - free_ticks < 0xfffffff5)) {
            /* �������º��ͷ�֮��ʱ������10ms */
            change_mode();
        }
    } else {                                    /* ��������״̬  */
        /* ��������ʱ��ȡ�δ�ʱ��ʱ��,�����жϰ�������ʱ�� */
        press_ticks = OSIF_GetMilliseconds();
    }
}
/* End of functions definition */
