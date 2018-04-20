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


m_lpuart_config_t  g_m_lpuart0_config;

m_flexcan_config_t g_m_flexcan_config;

/*
 * \brief   通过串口配置或则打印参数
 * \note    指令格式 AT[+(item)[=(value)]]
 */
void config_by_lpuart(void) {
    uint8_t index = 0;
    if (g_uart_rx_sta & 0x8000) { /* 串口接收完成 */
        GPIO_HAL_ClearPins(PTC, 1<<11);                   /* 关闭LED3,配置命令中 */
        g_uart_rx_buff[g_uart_rx_sta & 0x3fff] = '\0';     /* 将末尾变为0,使其成为字符串. */

        if(memcmp(g_uart_rx_buff, g_config_item[AT_index], 2)) {
            /* 指令必须是AT开头,换行(\r\n)结尾 */
            g_uart_rx_sta = 0;
            return;
        }
        index = 2;
        if (g_uart_rx_buff[index] != '+') {   /* AT后面不是'+' */
            if (g_uart_rx_buff[index] == '\0') {  /* 而是'\0': */
                LPUART0_transmit_string(g_statu_item[OK_index]); /* 返回OK; */
            } else {                            /* 否则: */
                LPUART0_transmit_string(g_statu_item[ERROR_index]); /* 返回OK. */
            }
            g_uart_rx_sta = 0;
            return;
        }

        int i = 0;
        index = 3;
        if (g_uart_rx_buff[index] == '@') {           /* +后面是@ */
            index += 2;                             /* 向后移动两位是指令描述符 */
        }                                           /* 否则不加 */
        for (i=3; i<CONFIG_CODES_COUNT; i++) {      /* 循环查找指令 */
            uint8_t config_code_len = strlen(g_config_item[i]);
            if(memcmp(g_uart_rx_buff+index, g_config_item[i], config_code_len) == 0) {
                index += config_code_len;           /* 下一个判定字符位置 */
                break;                              /* 如果指令匹配,跳出循环 */
            }
        }
        if (i == CONFIG_CODES_COUNT) { /* i=CONFIG_CODES_COUNT说明没有匹配的指令 */
            g_uart_rx_sta = 0;
            return;
        }
        if (g_uart_rx_buff[3] == '@') {                   /* +后面是@ */
            if (g_uart_rx_buff[4] == 'U') {                   /* 配置串口参数 */
                if (g_uart_rx_buff[index] == '='){
                    index++;
                    /* 配置串口参数 */
                    config_lpuart_by_lpuart(i, g_uart_rx_buff+index,
                            (g_uart_rx_sta & 0x3fff) - index);
                    /* 将参数保存到EEPROM */
                    save_config_paramater_to_EEPROM();
                } else {
                    LPUART0_print_lpuart_info(i);
                }
            } else if (g_uart_rx_buff[4] == 'C') {            /* 配置CAN参数 */
                if (g_uart_rx_buff[index] == '='){
                    index++;
                    /* 配置CAN参数 */
                    LPUART0_config_can(i, g_uart_rx_buff+index,
                            (g_uart_rx_sta & 0x3fff) - index);
                    /* 将参数保存到EEPROM */
                    save_config_paramater_to_EEPROM();
                } else {
                    LPUART0_print_can_config(i);
                }
            } else {
                g_uart_rx_sta = 0;
                return;
            }
        } else {

        }
        GPIO_HAL_SetPins(PTC, 1<<11);
        g_uart_rx_sta = 0;
    }
}

/*
 * \brief   保存参数到EEPROM
 */
void save_config_paramater_to_EEPROM() {
    if (flash_EEPROM_init() != STATUS_SUCCESS) {
        LPUART0_transmit_string("ERROR\r\n");
        return;
    } else {
        config_info_t paramater = {
                .m_lpuart_config = g_m_lpuart0_config,
                .m_flexcan_config = g_m_flexcan_config
        };
        CRC_DRV_Deinit(INST_CRC);
        CRC_DRV_Init(INST_CRC, &crc_InitConfig0);
        CRC_DRV_WriteData(INST_CRC, (uint8_t *)&paramater,
                sizeof(g_m_lpuart0_config) + sizeof(g_m_flexcan_config));
        paramater.crc = CRC_DRV_GetCrcResult(INST_CRC);
        flash_write_EEPROM(CONFIG_INFO_OFFSET, (uint8_t *)&paramater, sizeof(paramater));
    }
}


/**
 * \brief   打印参数配置结果
 * \param   device_item_index   设备项编号
 * \param   config_item_index   配置项编号
 * \param   status_item_index   状态项编号
 */
void LPUART0_print_option_status(device_item_index_t device_item_index,
                                 config_item_index_t config_item_index,
                                 statu_item_t        status_item_index)
{
    LPUART0_transmit_string("Set ");
    LPUART0_transmit_string(g_config_item[config_item_index]);
    LPUART0_transmit_char(' ');
    LPUART0_transmit_string(g_statu_item[status_item_index]);
    LPUART0_transmit_string("\r\n");
}

/**
 * \brief   通过串口配置串口参数
 * \param   config_item_index   配置项编号
 * \param   parameter[in]       参数(数字字符串)
 * \param   parameter_len       参数字符串长度
 */
void config_lpuart_by_lpuart(uint8_t    config_item_index,
                             uint8_t   *p_parameter,
                             uint16_t   parameter_len)
{
    while((LPUART0->STAT & LPUART_STAT_TDRE_MASK) == 0);    /* 等待缓冲区中的数据发送完成 */
    switch(config_item_index) {
    case BAUD_index:            /* 修改串口波特率; */
    {
        uint32_t baud;
        if(string2number(p_parameter, &baud) == STATUS_SUCCESS &&
                LPUART0_set_baud(baud) == STATUS_SUCCESS) {
        } else {
            LPUART0_print_option_status(U_index, BAUD_index, ERROR_index);
        }
        break;
    }
    case stopBitCount_index:    /* 修改串口停止位位数; */
    {
        uint32_t stopbit;
        if(string2number(p_parameter, &stopbit) == STATUS_SUCCESS
                && (stopbit == LPUART_ONE_STOP_BIT
                    || stopbit == LPUART_TWO_STOP_BIT)){

            LPUART_HAL_SetStopBitCount(LPUART0, LPUART_ONE_STOP_BIT);
            g_m_lpuart0_config.lpuart0_user_config.stopBitCount = stopbit;
            LPUART0_print_option_status(U_index, stopBitCount_index, OK_index);
        } else {
            LPUART0_print_option_status(U_index, stopBitCount_index, ERROR_index);
        }
        break;
    }
    case PARI_index:            /* 修改串口校验类型; */
    {
        uint32_t pari;
        if(string2number(p_parameter, &pari) == STATUS_SUCCESS
                && (pari == LPUART_PARITY_DISABLED
                    || pari == LPUART_PARITY_EVEN
                    || pari == LPUART_PARITY_ODD)){

            LPUART_HAL_SetParityMode(LPUART0, pari);
            g_m_lpuart0_config.lpuart0_user_config.parityMode = pari;
            LPUART0_print_option_status(U_index, PARI_index, OK_index);
        } else {
            LPUART0_print_option_status(U_index, PARI_index, ERROR_index);
        }
        break;
    }
    case NOTERR_index:
        break;
    case START_index:           /* 退出参数配置模式; */
        change_mode();
        break;
    case TMODE_index:           /* 修改串口输出模式; */
    {
        uint32_t txmode;
        if(string2number(p_parameter, &txmode) == STATUS_SUCCESS
            && (txmode == 0
                || txmode == 1)){
            g_m_lpuart0_config.txmode = txmode;
            LPUART0_print_option_status(U_index, TMODE_index, OK_index);
        } else {
            LPUART0_print_option_status(U_index, TMODE_index, ERROR_index);
        }
        break;
    }
    case RMODE_index:           /* 修改串口输入模式; */
    {
        uint32_t rxmode;
        if(string2number(p_parameter, &rxmode) == STATUS_SUCCESS
            && (rxmode == 0
                || rxmode == 1)){
            g_m_lpuart0_config.txmode = rxmode;
            LPUART0_print_option_status(U_index, RMODE_index, OK_index);
        } else {
            LPUART0_print_option_status(U_index, RMODE_index, ERROR_index);
        }
        break;
    }
    default:
        break;
    }
}

void LPUART0_print_config_parameter(device_item_index_t device_item_index,
                                    config_item_index_t config_item_index,
                                    uint32_t            parameter)
{
    LPUART0_transmit_string(g_device_item[device_item_index]);
    LPUART0_transmit_string(g_config_item[config_item_index]);
    LPUART0_transmit_char(':');
    LPUART_transmit_number(LPUART0, parameter);
    LPUART0_transmit_string("\r\n");
}

void LPUART0_print_lpuart_info(uint8_t config_item_index)
{
    switch(config_item_index) {
    case BAUD_index:            /* 打印串口波特率; */
        LPUART0_print_config_parameter(U_index, BAUD_index,
                g_m_lpuart0_config.lpuart0_user_config.baudRate);
        break;
    case stopBitCount_index:    /* 打印串口停止位位数; */
        LPUART0_print_config_parameter(U_index, stopBitCount_index,
                g_m_lpuart0_config.lpuart0_user_config.stopBitCount);
        break;
    case PARI_index:            /* 打印串口校验方式; */
        LPUART0_print_config_parameter(U_index, PARI_index,
                g_m_lpuart0_config.lpuart0_user_config.parityMode);
        break;
    case NOTERR_index:
        break;
    case START_index:
        change_mode();
        break;
    case TMODE_index:           /* 打印串口发送模式; */
        LPUART0_print_config_parameter(U_index, TMODE_index,
                g_m_lpuart0_config.txmode);
        break;
    case RMODE_index:           /* 打印串口接收模式. */
        LPUART0_print_config_parameter(U_index, RMODE_index,
                g_m_lpuart0_config.rxmode);
        break;
    default:
        break;
    }
}



/**
 * \brief   通过串口配置参数
 *
 * \param   config_item_index   配置项编号
 * \param   p_parameter[in]     参数(数字字符串)
 * \param   parameter_len       参数长度
 */
void LPUART0_config_can(config_item_index_t  config_item_index,
                           uint8_t             *p_parameter,
                           uint16_t             parameter_len)
{
    switch(config_item_index) {
    case BAUD_index:
    {
        if (flexcan_set_baud(p_parameter) == STATUS_SUCCESS) {
            LPUART0_print_option_status(C_index, BAUD_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, BAUD_index, ERROR_index);
        }
        break;
    }
    case TMODE_index:
    {
        uint32_t tmode = 0;
        if(string2number(p_parameter, &tmode) == STATUS_SUCCESS
                && (tmode == 0
                    || tmode == 1
                    || tmode == 2)){
            g_m_flexcan_config.txmode = tmode;
            LPUART0_print_option_status(C_index, PARI_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, PARI_index, ERROR_index);
        }
        break;
    }
    case RMODE_index:
    {
        uint32_t rmode = 0;
        if(string2number(p_parameter, &rmode) == STATUS_SUCCESS
                && (rmode == 0
                    || rmode == 1
                    || rmode == 2)){
            g_m_flexcan_config.rxmode = rmode;
            LPUART0_print_option_status(C_index, RMODE_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, RMODE_index, ERROR_index);
        }
        break;
    }
    case TXID_index:
    {
        uint32_t txid = 0;
        if(string2number(p_parameter, &txid) == STATUS_SUCCESS){
            if (g_m_flexcan_config.txmode == 0) {             /* 发送模式0 */
                txid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;  /* 标准帧ID */
            } else if (g_m_flexcan_config.txmode == 1) {      /* 发送模式1 */
                txid &= CAN_ID_EXT_MASK;                    /* 扩展帧ID */
            } else if (g_m_flexcan_config.txmode == 2) {      /* 发送模式2 */
                if ((txid&CAN_ID_EXT_MASK) > 0x7ff) {         /* 根据ID选择发送帧的模式, */
                    txid &= CAN_ID_EXT_MASK;                /* ID超过标准帧ID范围. */
                } else {
                    txid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;
                }
            }
            g_m_flexcan_config.tx_id = txid;
            LPUART0_print_option_status(C_index, TXID_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, TXID_index, ERROR_index);
        }
        break;
    }
    case RXID_index:
    {
        uint32_t rxid = 0;
        if(string2number(p_parameter, &rxid) == STATUS_SUCCESS){
            if (g_m_flexcan_config.rxmode == 0) {             /* 发送模式0 */
                rxid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;  /* 标准帧ID */
                FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_STD_MB,
                        &g_rx_info, rxid);
            } else if (g_m_flexcan_config.rxmode == 1) {      /* 发送模式1 */
                rxid &= CAN_ID_EXT_MASK;                    /* 扩展帧ID */
                FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_EXT_MB,
                        &g_rx_info, rxid);
            } else if (g_m_flexcan_config.rxmode == 2) {      /* 发送模式2 */
                if ((rxid&CAN_ID_EXT_MASK) > 0x7ff) {   /* 根据ID选择发送帧的模式, */
                    rxid &= CAN_ID_EXT_MASK;            /* ID超过标准帧ID范围. */
                    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_EXT_MB,
                            &g_rx_info, rxid);
                } else {
                    rxid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;
                    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_STD_MB,
                            &g_rx_info, rxid);
                }
            }
            g_m_flexcan_config.rx_id = rxid;
            LPUART0_print_option_status(C_index, RXID_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, RXID_index, ERROR_index);
        }
        break;
    }
    case IDMASK_index:
    {
        uint32_t idmask = 0;
        if(string2number(p_parameter, &idmask) == STATUS_SUCCESS){
            idmask &= CAN_ID_EXT_MASK;
            g_m_flexcan_config.id_mask = idmask;
            FLEXCAN_DRV_SetRxMbGlobalMask(INST_CANCOM0, FLEXCAN_MSG_ID_EXT,
                    g_m_flexcan_config.id_mask);
            LPUART0_print_option_status(C_index, IDMASK_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, IDMASK_index, ERROR_index);
        }
        break;
    }
    default:
        break;
    }
}

/**
 * \brief       从串口打印参数
 * \param[in]   config_item_index   参数在参数数组中的位置
 *                                  请参考 config_item_index_t 和 config_item
 */
void LPUART0_print_can_config(config_item_index_t config_item_index)
{
    switch(config_item_index) {
    case BAUD_index:
    {
        uint32_t baud = 0;
        flexcan_time_segment_t *bitrate = &g_m_flexcan_config.m_flexcan_user_config.bitrate;
        uint32_t flexcanSourceClock;
        flexcan_get_source_clock(&flexcanSourceClock);
        baud = flexcanSourceClock/(bitrate->preDivider+1)/
                (bitrate->propSeg + bitrate->phaseSeg1 + bitrate->phaseSeg2 + 4);
        LPUART0_print_config_parameter(C_index, BAUD_index, baud);
        break;
    }
    case TMODE_index:
        LPUART0_print_config_parameter(C_index, TMODE_index, g_m_flexcan_config.txmode);
        break;
    case RMODE_index:
        LPUART0_print_config_parameter(C_index, RMODE_index, g_m_flexcan_config.rxmode);
        break;
    case TXID_index:
        LPUART0_print_config_parameter(C_index, TXID_index, g_m_flexcan_config.tx_id);
        break;
    case RXID_index:
        LPUART0_print_config_parameter(C_index, RXID_index, g_m_flexcan_config.rx_id);
        break;
    case IDMASK_index:
        LPUART0_print_config_parameter(C_index, IDMASK_index, g_m_flexcan_config.id_mask);
        break;
    default:
        break;
    }
}

status_t string2number(const uint8_t *p_string, uint32_t *p_number) {
    const uint8_t *p = p_string;
    uint32_t temp = 0;
    if (*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X')) {
        return hex2number(p_string+2, p_number);
    }
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

status_t hex2number(const uint8_t *p_hex, uint32_t *p_number)
{
    const uint8_t *p = p_hex;
    register uint32_t temp = 0;
    while (*p != '\0') {
        if(p - p_hex > 8) {     /* hex长度超过8个,转换失败 */
            p_number = NULL;
            return STATUS_ERROR;
        }
        if ('0' <= *p && *p <= '9') {
            temp <<= 4;
            temp += *p - '0';
        }else if ('a'<= *p && *p <= 'f') {
            temp <<= 4;
            temp += *p - 'a' + 10;
        }else if ('A'<= *p && *p <= 'F') {
            temp <<= 4;
            temp += *p - 'A' + 10;
        } else {
            break;
        }
        p++;
    }
    if (p == p_hex) {
        p_number = NULL;
        return STATUS_ERROR;
    } else {
        *p_number = temp;
        return STATUS_SUCCESS;
    }
}

