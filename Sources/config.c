/*
 * configcodes.c
 *
 *  Created on: 2018年4月8日
 *      Author: Administrator
 */
#include "config.h"


const char *g_device_item[DEVICE_CODES_COUNT] = {
    "@U",       /* 代表后面的内容是配置串口 */
    "@C",       /* 代表后面的内容是配置CAN */
    "@O",       /* 代表后面的内容是配置其他 */
};

const char *g_config_item[CONFIG_CODES_COUNT] = {
        "AT",
        "+",
        "=",
        "BAUD",     /* 波特率 */
        "STOP",     /* 停止位数量 0:一位; 1:2位. */
        "PARI",     /* 校验 0:不校验; 2:偶校验; 3:奇校验. */
        "NOTERR",
        "START",    /* 结束配置,开启透传. */
        "TMODE",    /* 串口输出模式  0:只输出数据;
                                  1:输出地址+数据,1字节头+4字节地址+1字节数据长度+数据; */
                    /* CAN输出模式 0:标准帧; 1:扩展帧; 2:标准帧或者扩展帧. */
        "RMODE",    /* 串口输入模式  0:只输入数据;
                                  1:输入地址+数据,1字节头+4字节地址+1字节数据长度+数据; */
                    /* CAN输出模式 0:标准帧; 1:扩展帧; 2:标准帧或者扩展帧. */
        "TXID",     /* 发送数据时用的ID */
        "RXID",     /* 接收数据用的ID */
        "IDMASK",   /* 接收ID屏蔽码 */
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
    if (p == p_string) {  /* 如果字符串第一个都不是数字,那么转换失败. */
        return STATUS_ERROR;
    }
    *p_number = temp;     /*否则转换成功*/
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

