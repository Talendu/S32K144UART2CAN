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
#include "mainloop.h"

#include "mflexcan.h"
#include "mlpuart.h"
#include "mflash.h"


#define CONFIG_INFO_OFFSET 0    /**< \brief   ������Ϣ�洢��ַ��EEPROM�׵�ַƫ����  */
/**
 * \brief   �������������������Ϣ���������е�λ��
 */
typedef enum {
    U_index = 0,
    C_index,
    O_index,
    DEVICE_ITEM_COUNT
}device_item_index_t;
/**
 * \brief   �������������������Ϣ���������е�λ��
 */
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
    CONFIG_ITEM_COUNT
}config_item_index_t;
/**
 * \brief   ��������������������Ϣ���������е�λ��
 */
typedef enum {
    OK_index = 0,
    ERROR_index,
    STATU_ITEM_COUNT
}statu_item_t;

/**
 * \brief   lpuart������Ϣ�ṹ��
 */
typedef struct {
    lpuart_user_config_t lpuart0_user_config;   /**< \brief ����������Ϣ */
    uint8_t txmode;                             /**< \brief ����ģʽ */
    uint8_t rxmode;                             /**< \brief ����ģʽ */
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

extern m_lpuart_config_t    g_m_lpuart0_config; /**< \brief lpuart������Ϣ */
extern m_flexcan_config_t   g_m_flexcan_config; /**< \brief CAN������Ϣ */

/**
 * \brief   ������Ϣ�洢�ṹ
 * \note    ������Ϣ���Ըýṹ�洢��EEPROM��
 */
typedef struct {
    m_lpuart_config_t m_lpuart_config;
    m_flexcan_config_t m_flexcan_config;
    uint16_t crc;
}config_info_t;

/**
 * \brief   ATָ�����豸��Ϣ��
 */
extern const char *g_device_item[];
/**
 * \brief   ATָ�������������
 */
extern const char *g_config_item[];
/**
 * \brief   ATָ���н����
 */
extern const char *g_statu_item[];

/*
 * \brief   ͨ���������û����ӡ����
 *
 * \note    ָ���ʽ AT[+(item)[=(value)]]
 */
void config_by_lpuart(void);

/*
 * \brief   ���������EEPROM
 */
void save_config_paramater_to_EEPROM(void);

/**
 * \brief   ��ӡ�������ý��
 *
 * \param   device_item_index   �豸����
 * \param   config_item_index   ��������
 * \param   status_item_index   ״̬����
 */
void LPUART0_print_option_result(device_item_index_t device_item_index,
                                 config_item_index_t config_item_index,
                                 statu_item_t        status_code_index);

/**
 * \brief   ͨ���������ô��ڲ���
 *
 * \param   config_item_index   ��������
 * \param   parameter[in]       ����(�����ַ���)
 * \param   parameter_len       �����ַ�������
 */
void config_lpuart_by_lpuart(uint8_t     config_item_index,
                             uint8_t    *parameter,
                             uint16_t    parameter_len);

/**
 * \brief   ��һ����ʽ��ӡ������ӡ����
 *
 * \details �豸+����+":"+����
 *
 * \param   device_item_index   �豸�������е�λ��(��@C�Ǵ���CAN)
 * \param   config_item_index   �������������е�λ��
 * \param   parameter           ����
 */
void LPUART0_print_config_parameter(device_item_index_t device_item_index,
                                   config_item_index_t  config_item_index,
                                   uint32_t             parameter);

/**
 * \brief   ��ӡ���ڲ����ж�Ӧ�Ĳ���
 *
 * \param   config_item_index   �����������е�λ��
 */
void LPUART0_print_lpuart_info(uint8_t config_item_index);

/**
 * \brief   ͨ���������ò���
 *
 * \param   config_item_index   ��������
 * \param   p_parameter[in]     ����(�����ַ���)
 * \param   parameter_len       ��������
 */
void LPUART0_config_can(config_item_index_t   config_item_index,
                           uint8_t           *parameter,
                           uint16_t           parameter_len);

/**
 * \brief       �Ӵ��ڴ�ӡ����
 * \param[in]   config_item_index   �����ڲ��������е�λ��
 *                                  ��ο� config_item_index_t �� config_item
 */
void LPUART0_print_can_info(config_item_index_t config_item_index);

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
