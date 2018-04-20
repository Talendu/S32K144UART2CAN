/*
 * configcodes.c
 *
 *  Created on: 2018��4��8��
 *      Author: Administrator
 */
#include "config.h"


const char *g_device_item[DEVICE_CODES_COUNT] = {
    "@U",       /* �����������������ô��� */
    "@C",       /* ������������������CAN */
    "@O",       /* ���������������������� */
};

const char *g_config_item[CONFIG_CODES_COUNT] = {
        "AT",
        "+",
        "=",
        "BAUD",     /* ������ */
        "STOP",     /* ֹͣλ���� 0:һλ; 1:2λ. */
        "PARI",     /* У�� 0:��У��; 2:żУ��; 3:��У��. */
        "NOTERR",
        "START",    /* ��������,����͸��. */
        "TMODE",    /* �������ģʽ  0:ֻ�������;
                                  1:�����ַ+����,1�ֽ�ͷ+4�ֽڵ�ַ+1�ֽ����ݳ���+����; */
                    /* CAN���ģʽ 0:��׼֡; 1:��չ֡; 2:��׼֡������չ֡. */
        "RMODE",    /* ��������ģʽ  0:ֻ��������;
                                  1:�����ַ+����,1�ֽ�ͷ+4�ֽڵ�ַ+1�ֽ����ݳ���+����; */
                    /* CAN���ģʽ 0:��׼֡; 1:��չ֡; 2:��׼֡������չ֡. */
        "TXID",     /* ��������ʱ�õ�ID */
        "RXID",     /* ���������õ�ID */
        "IDMASK",   /* ����ID������ */
};

const char *g_statu_item[STATU_CODES_COUNT] = {
        "OK",
        "ERROR"
};




status_t string2number(uint8_t *p_string, uint32_t *p_number) {
    uint8_t *p = p_string;
    uint32_t temp = 0;
    while ( *p >= '0' && *p <= '9') {
        temp = temp*10 + *p-'0';
        p++;
    }
    if (p == p_string) {  /* ����ַ�����һ������������,��ôת��ʧ��. */
        return STATUS_ERROR;
    }
    *p_number = temp;     /*����ת���ɹ�*/
    return STATUS_SUCCESS;
}

status_t hex2number(uint8_t *p_hex, uint32_t *p_number)
{
    uint8_t *p = p_hex;
    register uint32_t temp = 0;
    while (*p != '\0') {
        if ('0' <= *p && *p <= '9') {
            temp += *p - '0';
            temp <<= 4;
        }
    }
    return STATUS_SUCCESS;
}

