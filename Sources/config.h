/*
 * configcodes.h
 *
 *  Created on: 2018年4月8日
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
 * \brief   配置命令中外设分类信息码在数组中的位置
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
 * \brief   CAN配置信息
 */
typedef struct {
    flexcan_user_config_t m_flexcan_user_config; /**< \brief 基本配置信息 */
    uint8_t txmode;             /**< \brief CAN发送模式 */
    uint8_t rxmode;             /**< \brief CAN接收模式 */
    uint32_t tx_id;             /**< \brief CAN发送邮箱ID */
    uint32_t rx_id;             /**< \brief CAN接收邮箱ID */
    uint32_t id_mask;           /**< \brief CAN接收邮箱ID掩码 */
}m_flexcan_config_t;

extern m_lpuart_config_t    g_m_lpuart0_config;

extern m_flexcan_config_t   g_m_flexcan_config;

/**
 * \brief   配置信息存储结构
 * \note    配置信息将以该结构存储在EEPROM中
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
 * \brief   通过串口配置参数
 *
 * \param   config_item_index   配置项编号
 * \param   p_parameter[in]     参数(数字字符串)
 * \param   parameter_len       参数长度
 */
void LPUART0_config_can(config_item_index_t   config_item_index,
                           uint8_t              *p_parameter,
                           uint16_t              parameter_len);

/**
 * \brief       从串口打印参数
 * \param[in]   config_item_index   参数在参数数组中的位置
 *                                  请参考 config_item_index_t 和 config_item
 */
void LPUART0_print_can_config(config_item_index_t config_item_index);

/**
 * \brief   将数字字符串转换为数字
 * \retval  STATUS_ERROR    转换错误
 *          STATUS_SUCCESS  转换成功
 * \note    如果第一个元素不是数字,转换失败
 *          必须是含有结束位的字符串,否则可能会得到意料之外的结果
 */
status_t string2number(const uint8_t *string, uint32_t *number);

status_t hex2number(const uint8_t *p_hex, uint32_t *p_number);
#endif /* CONFIG_H_ */
