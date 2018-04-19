/*
 * flexcan.h
 *
 *  Created on: 2018年4月8日
 *      Author: Administrator
 */

#ifndef MFLEXCAN_H_
#define MFLEXCAN_H_

#include "S32K144.h"
#include "clockMan1.h"
#include "canCom0.h"
#include "pin_mux.h"
#include "dmaController0.h"
#include "osif.h"
#include "mlpuart.h"
#include "configcodes.h"
#include "fifo.h"

#define RECEIVE_STD_MB  8       /**< \brief CAN标准帧接收邮箱地址 */
#define RECEIVE_EXT_MB  9       /**< \brief CAN扩展帧接收邮箱地址 */

#define TRANSMIT_STD_MB 10      /**< \brief CAN标准帧发送邮箱地址 */
#define TRANSMIT_EXT_MB 11      /**< \brief CAN扩展帧发送邮箱地址 */

#define __FLEXCAN_RX_FIFO_SIZE    (8*1024)  /**< \brief CAN接收数据缓冲区内存块大小 */
#define FLEXCAN_RX_DATA_TYPE uint8_t        /**< \brief CAN接收数据存储类型 */

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


extern flexcan_msgbuff_t g_can_receive_buff;
extern uint8_t           lpuart_receive_buff[];
extern m_flexcan_config_t m_flexcan_config;
extern flexcan_data_info_t rx_info;
extern flexcan_data_info_t tx_info;

extern fifo_t flexcan_rx_fifo;

void can_callback_transparent_transmission(uint8_t instance, flexcan_event_type_t eventType,
        struct FlexCANState * state);
void can_callback_configuration_parameters(uint8_t instance, flexcan_event_type_t eventType,
        struct FlexCANState * state);

void init_flexcan(void);

void flexcan_get_source_clock(uint32_t *flexcanSourceClock);

void config_can_by_lpuart(config_item_index_t config_code_index, uint8_t *parameter, uint16_t parameter_len);
void print_can_config_by_lpuart(config_item_index_t config_code_index);


#endif /* MFLEXCAN_H_ */
