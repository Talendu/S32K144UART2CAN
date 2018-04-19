/*
 * configcodes.h
 *
 *  Created on: 2018年4月8日
 *      Author: Administrator
 */

#ifndef CONFIGCODES_H_
#define CONFIGCODES_H_
#include "S32K144.h"
#include "status.h"

typedef enum {
    U_index,
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
    OK_index,
    ERROR_index,
    STATU_CODES_COUNT
}statu_item_t;

extern char *device_item[];
extern char *config_item[];
extern char *statu_item[];
/**
 * \brief   将数字字符串转换为数字
 * \retval  STATUS_ERROR    转换错误
 *          STATUS_SUCCESS  转换成功
 * \note    如果第一个元素不是数字,转换失败
 *          必须是含有结束位的字符串,否则可能会得到意料之外的结果
 */
status_t string2number(uint8_t *string, uint32_t *number);
#endif /* CONFIGCODES_H_ */
