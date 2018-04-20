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





extern const char *g_device_item[];
extern const char *g_config_item[];
extern const char *g_statu_item[];
/**
 * \brief   �������ַ���ת��Ϊ����
 * \retval  STATUS_ERROR    ת������
 *          STATUS_SUCCESS  ת���ɹ�
 * \note    �����һ��Ԫ�ز�������,ת��ʧ��
 *          �����Ǻ��н���λ���ַ���,������ܻ�õ�����֮��Ľ��
 */
status_t string2number(uint8_t *string, uint32_t *number);
#endif /* CONFIG_H_ */
