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
#include "dmaController0.h"
#include "config.h"
#include "fifo.h"

#define RECEIVE_STD_MB  0       /**< \brief CAN标准帧接收邮箱地址 */
#define RECEIVE_EXT_MB  1       /**< \brief CAN扩展帧接收邮箱地址 */

#define TRANSMIT_STD_MB 2      /**< \brief CAN标准帧发送邮箱地址 */
#define TRANSMIT_EXT_MB 3      /**< \brief CAN扩展帧发送邮箱地址 */

#define __FLEXCAN_RX_FIFO_SIZE    (8*1024)  /**< \brief CAN接收数据缓冲区内存块大小 */
#define FLEXCAN_RX_DATA_TYPE uint8_t        /**< \brief CAN接收数据存储类型 */


extern flexcan_msgbuff_t    g_can_receive_buff; /**< \brief CAN最后一次接收到的数据  */

extern flexcan_data_info_t  g_rx_info;
extern flexcan_data_info_t  g_tx_info;
extern fifo_t               g_flexcan_rx_fifo;

/**
 * \brief   透明传输模式CAN接收回掉函数
 *
 * \param   instance    CAN通道
 * \param   eventType   中断类型
 * \param   state       CAN通道状态信息
 */
void can_callback_transmission(uint8_t               instance,
                               flexcan_event_type_t  eventType,
                               flexcan_state_t      *state);

/**
 * \brief   参数配置模式CAN接收回掉函数
 *
 * \param   instance    CAN通道
 * \param   eventType   中断类型
 * \param   state       CAN通道状态信息
 */
void can_callback_config(uint8_t               instance,
                         flexcan_event_type_t  eventType,
                         flexcan_state_t      *state);

/**
 * \brief   初始化CAN
 */
void flexcan_init(void);

/**
 * \brief   初始化标准帧邮箱
 */
void flexcan_init_STD_MB(void);

/**
 * \brief   初始化远程帧邮箱
 */
void flexcan_init_EXT_MB(void);

/**
 * \brief       获取can0时钟频率
 *
 * \param[out]  p_flexcanSourceClock[out]   时钟频率
 */
void flexcan_get_source_clock(uint32_t *flexcanSourceClock);

/**
 * \brief   设置CAN波特率
 *
 * \param   p_parameter[in]     以'\0'结束的字符串(十进制或十六进制)
 */
status_t flexcan_set_baud(uint32_t baud);

/**
 * \brief   修改CAN FD比特率
 *
 * \param   p_parameter[in]     比特率,输入数字字符串
 *
 * \retval  修改成功返回: STATUS_SUCCESS
 *          修改失败返回: DTATUS_ERROR
 *
 * \note    优先使用量子数量与26接近的分频系数,
 *          优先设置采样点在比特时间的80%处
 */
status_t flexcan_set_fdbaud(uint32_t baud);

/**
 * \brief   获取CAN波特率
 */
uint32_t flexcan_get_baud(void);

/**
 * \brief   获取CAN FD波特率
 */
uint32_t flexcan_get_fdbaud(void);

#endif /* MFLEXCAN_H_ */
