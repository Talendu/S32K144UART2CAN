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
#include "mainloop.h"

#include "mflexcan.h"
#include "mlpuart.h"
#include "mflash.h"


#define CONFIG_INFO_OFFSET 0    /**< \brief   配置信息存储地址与EEPROM首地址偏移量  */
/**
 * \brief   配置命令中外设分类信息码在数组中的位置
 */
typedef enum {
    U_index = 0,
    C_index,
    O_index,
    DEVICE_ITEM_COUNT
}device_item_index_t;
/**
 * \brief   配置命令中命令分类信息码在数组中的位置
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
 * \brief   配置命令操作结果分类信息码在数组中的位置
 */
typedef enum {
    OK_index = 0,
    ERROR_index,
    STATU_ITEM_COUNT
}statu_item_t;

/**
 * \brief   lpuart配置信息结构体
 */
typedef struct {
    lpuart_user_config_t lpuart0_user_config;   /**< \brief 基本配置信息 */
    uint8_t txmode;                             /**< \brief 发送模式 */
    uint8_t rxmode;                             /**< \brief 接收模式 */
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

extern m_lpuart_config_t    g_m_lpuart0_config; /**< \brief lpuart配置信息 */
extern m_flexcan_config_t   g_m_flexcan_config; /**< \brief CAN配置信息 */

/**
 * \brief   配置信息存储结构
 * \note    配置信息将以该结构存储在EEPROM中
 */
typedef struct {
    m_lpuart_config_t m_lpuart_config;
    m_flexcan_config_t m_flexcan_config;
    uint16_t crc;
}config_info_t;

/**
 * \brief   AT指令中设备信息码
 */
extern const char *g_device_item[];
/**
 * \brief   AT指令中命令分类码
 */
extern const char *g_config_item[];
/**
 * \brief   AT指令中结果码
 */
extern const char *g_statu_item[];

/*
 * \brief   通过串口配置或则打印参数
 *
 * \note    指令格式 AT[+(item)[=(value)]]
 */
void config_by_lpuart(void);

/*
 * \brief   保存参数到EEPROM
 */
void save_config_paramater_to_EEPROM(void);

/**
 * \brief   打印参数配置结果
 *
 * \param   device_item_index   设备项编号
 * \param   config_item_index   配置项编号
 * \param   status_item_index   状态项编号
 */
void LPUART0_print_option_result(device_item_index_t device_item_index,
                                 config_item_index_t config_item_index,
                                 statu_item_t        status_code_index);

/**
 * \brief   通过串口配置串口参数
 *
 * \param   config_item_index   配置项编号
 * \param   parameter[in]       参数(数字字符串)
 * \param   parameter_len       参数字符串长度
 */
void config_lpuart_by_lpuart(uint8_t     config_item_index,
                             uint8_t    *parameter,
                             uint16_t    parameter_len);

/**
 * \brief   按一定格式打印参数打印参数
 *
 * \details 设备+命令+":"+参数
 *
 * \param   device_item_index   设备在数组中的位置(如@C是代表CAN)
 * \param   config_item_index   配置项在数组中的位置
 * \param   parameter           参数
 */
void LPUART0_print_config_parameter(device_item_index_t device_item_index,
                                   config_item_index_t  config_item_index,
                                   uint32_t             parameter);

/**
 * \brief   打印串口参数中对应的参数
 *
 * \param   config_item_index   命令在数组中的位置
 */
void LPUART0_print_lpuart_info(uint8_t config_item_index);

/**
 * \brief   通过串口配置参数
 *
 * \param   config_item_index   配置项编号
 * \param   p_parameter[in]     参数(数字字符串)
 * \param   parameter_len       参数长度
 */
void LPUART0_config_can(config_item_index_t   config_item_index,
                           uint8_t           *parameter,
                           uint16_t           parameter_len);

/**
 * \brief       从串口打印参数
 * \param[in]   config_item_index   参数在参数数组中的位置
 *                                  请参考 config_item_index_t 和 config_item
 */
void LPUART0_print_can_info(config_item_index_t config_item_index);

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
