/*
 * initialization.c
 *
 *  Created on: 2018��4��7��
 *      Author: Administrator
 */
#include "initialization.h"


const uint8_t init_ok[] = "Initialization OK !\r\n";

/**
 * \brief   ��ʼ��ʱ��
 */
void init_sys_clock(void) {
    CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
            g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0, CLOCK_MANAGER_POLICY_FORCIBLE);
}

/**
 * \brief   ��ʼ��IO��
 */
void init_pins(void) {
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
}

/**
 * \brief   ��ʼ����ʱ��
 */
void init_lpit(void) {
//    LPIT_DRV_Init(INST_LPIT0, &lpit0_InitConfig);
//    LPIT_DRV_InitChannel(INST_LPIT0, 0, &lpit0_ChnConfig0);
//    LPIT_HAL_StartTimerChannels(LPIT0, 0x01U);
}

/*
 * \brief   ��ȡEEPROM�е�������Ϣ
 */
status_t get_parameters_from_EEPROM(void){
    status_t status = STATUS_SUCCESS;
    config_info_t *config_parameter;
    uint16_t crcret;

    if (flash_EEPROM_init() != STATUS_SUCCESS) {
        return STATUS_ERROR;
    }
    config_parameter = (config_info_t *)(g_flashSSDConfig.EERAMBase + CONFIG_INFO_OFFSET);

    CRC_DRV_Init(INST_CRC, &crc_InitConfig0);
    CRC_DRV_WriteData(INST_CRC, (uint8_t*)config_parameter,
            sizeof(g_m_lpuart0_config) + sizeof(g_m_flexcan_config));
    crcret = CRC_DRV_GetCrcResult(INST_CRC);

    if (crcret != config_parameter->crc) {
        status = STATUS_ERROR;
        return status;
    }

    g_m_lpuart0_config = config_parameter->m_lpuart_config;
    g_m_flexcan_config = config_parameter->m_flexcan_config;

    return status;
}

/**
 * \brief   ��ȡĬ������
 */
void get_default_config_info(void) {
    g_m_lpuart0_config.lpuart0_user_config = g_lpuart_default_UserConfig;
    g_m_lpuart0_config.rxmode = 0;
    g_m_lpuart0_config.txmode = 0;

    g_m_flexcan_config.m_flexcan_user_config = canCom0_InitConfig0;
    g_m_flexcan_config.rxmode = 0;
    g_m_flexcan_config.txmode = 0;
    g_m_flexcan_config.rx_id = 0x00;
    g_m_flexcan_config.tx_id = 0x00;
    g_m_flexcan_config.id_mask = 0x00;
}

/**
 * \brief   ��ʼ��LED
 */
void led_init(void) {
    GPIO_HAL_SetPins(PTC, 0x03<<11);
    GPIO_HAL_SetPins(PTC, 0x03<<12);
    GPIO_HAL_ClearPins(PTC, 0x03<<13);
}

/**
 * \brief   ��ȡ���ں�CAN�Ĳ���
 * \details �����ȡEEPROM�еĲ���ʧ��,��ʹ��Ĭ�ϲ�������,
 *          ��ȡ�ɹ���ʹ��EEPROM�еĲ���
 */
void get_parameters(void) {
    if (get_parameters_from_EEPROM() != STATUS_SUCCESS) {
        get_default_config_info();
    }
}

/**
 * \brief   ��ʼ��ϵͳ
 */
void init_all(void) {
    init_sys_clock();
    init_pins();
    get_parameters();
    init_flexcan();
    LPUART0_init(&g_m_lpuart0_config.lpuart0_user_config);
    key_init();
    led_init();
//    LPUART0_transmit_string((int8_t *)init_ok);
}

/**
 * \brief   ����ΪĬ��������Ϣ
 */
void set_to_default_config(void) {
    get_default_config_info();
    init_flexcan();
    LPUART0_init(&g_m_lpuart0_config.lpuart0_user_config);
    save_config_paramater_to_EEPROM();
}
