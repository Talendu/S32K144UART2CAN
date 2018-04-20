/*
 * configcodes.h
 *
 *  Created on: 2018��4��8��
 *      Author: Administrator
 */

#ifndef CONFIG_H_
#define CONFIG_H_
#include "S32K144.h"
#include "status.h"
#include "crc.h"
#include "string.h"

#include "mflexcan.h"
#include "mlpuart.h"
#include "mflash.h"

#define CONFIG_INFO_OFFSET 0
/**
 * \brief   �������������������Ϣ���������е�λ��
 */
typedef enum {
    U_index = 0,
    C_index,
    O_index,
    DEVICE_CODES_COUNT
}device_item_index_t;

typedef enum {
    AT_index = 0,
    ADDITION_index,
    EQUAL_index,
    BAUD_index,
    stopBitCount_index,
    PARI_index,
    NOTERR_index,
    START_index,
    TMODE_index,
    RMODE_index,
    TXID_index,
    RXID_index,
    IDMASK_index,
    CONFIG_CODES_COUNT
}config_item_index_t;

typedef enum {
    OK_index = 0,
    ERROR_index,
    STATU_CODES_COUNT
}statu_item_t;

typedef struct {
    lpuart_user_config_t lpuart0_user_config;
    uint8_t txmode;
    uint8_t rxmode;
}m_lpuart_config_t;

/**
 * \brief   CAN������Ϣ
 */
typedef struct {
    flexcan_user_config_t m_flexcan_user_config; /**< \brief ����������Ϣ */
    uint8_t txmode;             /**< \brief CAN����ģʽ */
    uint8_t rxmode;             /**< \brief CAN����ģʽ */
    uint32_t tx_id;             /**< \brief CAN��������ID */
    uint32_t rx_id;             /**< \brief CAN��������ID */
    uint32_t id_mask;           /**< \brief CAN��������ID���� */
}m_flexcan_config_t;

extern m_lpuart_config_t    g_m_lpuart0_config;

extern m_flexcan_config_t   g_m_flexcan_config;

/**
 * \brief   ������Ϣ�洢�ṹ
 * \note    ������Ϣ���Ըýṹ�洢��EEPROM��
 */
typedef struct {
    m_lpuart_config_t m_lpuart_config;
    m_flexcan_config_t m_flexcan_config;
    uint16_t crc;
}config_info_t;




extern const char *g_device_item[];
extern const char *g_config_item[];
extern const char *g_statu_item[];




void config_by_lpuart(void);

void save_config_paramater_to_EEPROM(void);


void LPUART0_print_option_status(device_item_index_t device_code_index,
                                 config_item_index_t config_item_index,
                                 statu_item_t        status_code_index);

void config_lpuart_by_lpuart(uint8_t     config_item_index,
                             uint8_t    *p_parameter,
                             uint16_t    parameter_len);

void LPUART0_print_config_parameter(device_item_index_t device_code_index,
                                   config_item_index_t  config_item_index,
                                   uint32_t             p_parameter);

void LPUART0_print_lpuart_info(uint8_t config_item_index);

/**
 * \brief   ͨ���������ò���
 *
 * \param   config_item_index   ��������
 * \param   p_parameter[in]     ����(�����ַ���)
 * \param   parameter_len       ��������
 */
void LPUART0_config_can(config_item_index_t   config_item_index,
                           uint8_t              *p_parameter,
                           uint16_t              parameter_len);

/**
 * \brief       �Ӵ��ڴ�ӡ����
 * \param[in]   config_item_index   �����ڲ��������е�λ��
 *                                  ��ο� config_item_index_t �� config_item
 */
void LPUART0_print_can_config(config_item_index_t config_item_index);

/**
 * \brief   �������ַ���ת��Ϊ����
 * \retval  STATUS_ERROR    ת������
 *          STATUS_SUCCESS  ת���ɹ�
 * \note    �����һ��Ԫ�ز�������,ת��ʧ��
 *          �����Ǻ��н���λ���ַ���,������ܻ�õ�����֮��Ľ��
 */
status_t string2number(const uint8_t *string, uint32_t *number);

status_t hex2number(const uint8_t *p_hex, uint32_t *p_number);
#endif /* CONFIG_H_ */
