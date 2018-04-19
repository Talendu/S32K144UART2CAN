/*
 * flexcan.c
 *
 *  Created on: 2018年4月8日
 *      Author: Administrator
 */
#include "mflexcan.h"

flexcan_msgbuff_t g_can_receive_buff;
uint8_t           lpuart_receive_buff[8];

m_flexcan_config_t m_flexcan_config;

/**
 * \brief   发送邮箱配置信息
 */
flexcan_data_info_t rx_info = {
        .data_length = 8,
        .enable_brs = 1,
        .fd_enable = 0,
        .fd_padding = 0,
        .is_remote = 0,
        .msg_id_type = FLEXCAN_MSG_ID_STD
};

/**
 * \brief   接收邮箱配置信息
 */
flexcan_data_info_t tx_info = {
        .data_length = 8,
        .enable_brs = 1,
        .fd_enable = 0,
        .fd_padding = 0,
        .is_remote = 0,
        .msg_id_type = FLEXCAN_MSG_ID_STD
};

/**
 * \brief   CAN数据接收缓冲区
 */
fifo_t flexcan_rx_fifo;
/**
 * \brief   CAN数据缓冲区内存空间
 */
FLEXCAN_RX_DATA_TYPE flexcan_rx_base[__FLEXCAN_RX_FIFO_SIZE];


status_t flexcan_set_baud(uint8_t *parameter);

/**
 * \brief   透明传输模式CAN接收回掉函数
 * \param   instance    CAN通道
 * \param   eventType   中断类型
 * \param   state       CAN通道状态信息
 */
void can_callback_transparent_transmission(uint8_t instance, flexcan_event_type_t eventType,
        struct FlexCANState * state)
{
    if (eventType == FLEXCAN_EVENT_RX_COMPLETE){        /* 如果是接收完成中断 */
        FLEXCAN_RX_DATA_TYPE *fifo_rear;
        uint8_t i = 0;
        for (i=0; i<g_can_receive_buff.dataLen; i++) {    /* 将邮箱中的数据复制到缓冲区 */
            if (fifo_get_rear_pointer(&flexcan_rx_fifo, (void**)&fifo_rear)
                    == STATUS_SUCCESS) {                /* 判断缓冲区是否为满 */
                /* 缓冲区没有满,则复制数据 */
                *fifo_rear = g_can_receive_buff.data[i];
                fifo_append(&flexcan_rx_fifo);
            } else {
                /* 缓冲区满,LED1点亮,并且跳出中断 */
                GPIO_HAL_ClearPins(PTC, 1<<11);
                break;
            }
        }
        if (m_flexcan_config.rxmode == 0) {         /* 模式0,只接受标准帧 */
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);
        } else if (m_flexcan_config.rxmode == 1) {  /* 模式1,只接受扩展帧 */
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
        } else if (m_flexcan_config.rxmode == 2) {  /* 模式1,接收两种帧 */
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
        }
    }
}

/**
 * \brief   参数配置模式CAN接收回掉函数
 * \param   instance    CAN通道
 * \param   eventType   中断类型
 * \param   state       CAN通道状态信息
 */
void can_callback_configuration_parameters(uint8_t instance, flexcan_event_type_t eventType,
        struct FlexCANState * state)
{
    GPIO_HAL_TogglePins(PTC, 1<<12);

    LPUART0_trancemit_buffer(g_can_receive_buff.data, 8);
    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);
    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
}

/**
 * \brief   初始化CAN
 */
void init_flexcan(void)
{
    fifo_init(&flexcan_rx_fifo,                                   /* can接收fifo; */
            flexcan_rx_base,                                      /* can接收fifo内存空间; */
            sizeof(flexcan_rx_base)/sizeof(FLEXCAN_RX_DATA_TYPE), /* can接收fifo元素容量,非内存大小; */
            sizeof(FLEXCAN_RX_DATA_TYPE));                        /* can接收数据数据类型大小. */

    FLEXCAN_DRV_Deinit(INST_CANCOM0);
    FLEXCAN_DRV_Init(INST_CANCOM0, &canCom0_State, &m_flexcan_config.m_flexcan_user_config);

    FLEXCAN_DRV_SetRxMbGlobalMask(INST_CANCOM0, FLEXCAN_MSG_ID_EXT, m_flexcan_config.id_mask);

    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_STD_MB, &rx_info, m_flexcan_config.rx_id);
    FLEXCAN_DRV_InstallEventCallback(INST_CANCOM0, can_callback_transparent_transmission, NULL);
    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);

    rx_info.msg_id_type = FLEXCAN_MSG_ID_EXT;
    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_EXT_MB, &rx_info, m_flexcan_config.rx_id);
    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
}

/**
 * \brief   通过串口配置参数
 * \param   config_code_index
 */
void config_can_by_lpuart(config_item_index_t  config_code_index,
                          uint8_t              *parameter,
                          uint16_t              parameter_len)
{
    switch(config_code_index) {
    case BAUD_index:
    {
        if (flexcan_set_baud(parameter) == STATUS_SUCCESS) {
            LPUART0_print_option_status(C_index, BAUD_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, BAUD_index, ERROR_index);
        }
        break;
    }
    case TMODE_index:
    {
        uint32_t tmode = 0;
        if(string2number(parameter, &tmode) == STATUS_SUCCESS
                && (tmode == 0
                    || tmode == 1
                    || tmode == 2)){
            m_flexcan_config.txmode = tmode;
            LPUART0_print_option_status(C_index, PARI_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, PARI_index, ERROR_index);
        }
        break;
    }
    case RMODE_index:
    {
        uint32_t rmode = 0;
        if(string2number(parameter, &rmode) == STATUS_SUCCESS
                && (rmode == 0
                    || rmode == 1
                    || rmode == 2)){
            m_flexcan_config.rxmode = rmode;
            LPUART0_print_option_status(C_index, RMODE_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, RMODE_index, ERROR_index);
        }
        break;
    }
    case TXID_index:
    {
        uint32_t txid = 0;
        if(string2number(parameter, &txid) == STATUS_SUCCESS){
            if (m_flexcan_config.txmode == 0) {             /* 发送模式0 */
                txid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;  /* 标准帧ID */
            } else if (m_flexcan_config.txmode == 1) {      /* 发送模式1 */
                txid &= CAN_ID_EXT_MASK;                    /* 扩展帧ID */
            } else if (m_flexcan_config.txmode == 2) {      /* 发送模式2 */
                if ((txid&CAN_ID_EXT_MASK) > 0x7ff) {         /* 根据ID选择发送帧的模式, */
                    txid &= CAN_ID_EXT_MASK;                /* ID超过标准帧ID范围. */
                } else {
                    txid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;
                }
            }
            m_flexcan_config.tx_id = txid;
            LPUART0_print_option_status(C_index, TXID_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, TXID_index, ERROR_index);
        }
        break;
    }
    case RXID_index:
    {
        uint32_t rxid = 0;
        if(string2number(parameter, &rxid) == STATUS_SUCCESS){
            if (m_flexcan_config.rxmode == 0) {             /* 发送模式0 */
                rxid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;  /* 标准帧ID */
                FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_STD_MB, &rx_info, rxid);
            } else if (m_flexcan_config.rxmode == 1) {      /* 发送模式1 */
                rxid &= CAN_ID_EXT_MASK;                    /* 扩展帧ID */
                FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_EXT_MB, &rx_info, rxid);
            } else if (m_flexcan_config.rxmode == 2) {      /* 发送模式2 */
                if ((rxid&CAN_ID_EXT_MASK) > 0x7ff) {         /* 根据ID选择发送帧的模式, */
                    rxid &= CAN_ID_EXT_MASK;                /* ID超过标准帧ID范围. */
                    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_EXT_MB, &rx_info, rxid);
                } else {
                    rxid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;
                    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_STD_MB, &rx_info, rxid);
                }
            }
            m_flexcan_config.rx_id = rxid;
            LPUART0_print_option_status(C_index, RXID_index, OK_index);
        } else {
            LPUART0_print_option_status(C_index, RXID_index, ERROR_index);
        }
        break;
    }
    case IDMASK_index:
    {
        uint32_t idmask = 0;
        if(string2number(parameter, &idmask) == STATUS_SUCCESS){
            idmask &= CAN_ID_EXT_MASK;
            m_flexcan_config.id_mask = idmask;
            FLEXCAN_DRV_SetRxMbGlobalMask(INST_CANCOM0, FLEXCAN_MSG_ID_EXT, m_flexcan_config.id_mask);
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


/*!
 * \brief       从串口打印参数
 * \param[in]   参数在参数数组中的位置,请参考 config_codes_index_t 和 config_codes
 */
void print_can_config_by_lpuart(config_item_index_t config_code_index)
{
    switch(config_code_index) {
    case BAUD_index:
    {
        uint32_t baud = 0;
        flexcan_time_segment_t *bitrate = &m_flexcan_config.m_flexcan_user_config.bitrate;
        uint32_t flexcanSourceClock;
        flexcan_get_source_clock(&flexcanSourceClock);
        baud = flexcanSourceClock/(bitrate->preDivider+1)/
                (bitrate->propSeg + bitrate->phaseSeg1 + bitrate->phaseSeg2 + 4);
        LPUART0_print_config_parameter(C_index, BAUD_index, baud);
        break;
    }
    case TMODE_index:
        LPUART0_print_config_parameter(C_index, TMODE_index, m_flexcan_config.txmode);
        break;
    case RMODE_index:
        LPUART0_print_config_parameter(C_index, RMODE_index, m_flexcan_config.rxmode);
        break;
    case TXID_index:
        LPUART0_print_config_parameter(C_index, TXID_index, m_flexcan_config.tx_id);
        break;
    case RXID_index:
        LPUART0_print_config_parameter(C_index, RXID_index, m_flexcan_config.rx_id);
        break;
    case IDMASK_index:
        LPUART0_print_config_parameter(C_index, IDMASK_index, m_flexcan_config.id_mask);
        break;
    default:
        break;
    }
}

/*!
 * \brief       获取can0时钟频率
 * \param[out]  flexcanSourceClock  时钟频率
 */
void flexcan_get_source_clock(uint32_t *flexcanSourceClock)
{
    if (m_flexcan_config.m_flexcan_user_config.pe_clock == FLEXCAN_CLK_SOURCE_SYS){
        CLOCK_SYS_GetFreq(CORE_CLOCK, flexcanSourceClock);
    } else {
        uint32_t i = (SCG->SOSCDIV & SCG_SOSCDIV_SOSCDIV2_MASK) >> SCG_SOSCDIV_SOSCDIV2_SHIFT;
        CLOCK_SYS_GetFreq(SOSC_CLOCK, flexcanSourceClock);
        for (i=i-1; i>0; i--) {
            *flexcanSourceClock >>= 1;
        }
    }
}

/*!
 * \brief   修改CAN比特率
 * \param   parameter   比特率,输入数字字符串
 * \retval  修改成功返回: STATUS_SUCCESS
 *          修改失败返回: DTATUS_ERROR
 */
status_t flexcan_set_baud(uint8_t *parameter)
{
    uint32_t baud;

    uint32_t presdiv = 0;
    uint32_t propseg = 0;
    uint32_t pseg1 = 0;

    uint32_t pseg2 = 0;
    uint32_t rjw = 0;
    uint32_t bittime = 0;

    flexcan_clk_source_t backup_clk_source = 0;
    uint32_t flexcanSourceClock = 0;

    uint8_t i = 0;
    uint8_t j = 0;

    backup_clk_source = m_flexcan_config.m_flexcan_user_config.pe_clock;

    if (string2number(parameter, &baud) == STATUS_SUCCESS) {
        FLEXCAN_HAL_Disable(CAN0);
        if (baud > 60000) {
            FLEXCAN_HAL_SelectClock(CAN0, FLEXCAN_CLK_SOURCE_SYS);
            m_flexcan_config.m_flexcan_user_config.pe_clock = FLEXCAN_CLK_SOURCE_SYS;
        } else {
            FLEXCAN_HAL_SelectClock(CAN0, FLEXCAN_CLK_SOURCE_SOSCDIV2);
            m_flexcan_config.m_flexcan_user_config.pe_clock = FLEXCAN_CLK_SOURCE_SOSCDIV2;
        }
        flexcan_get_source_clock(&flexcanSourceClock);

        bittime = flexcanSourceClock/baud;
        for (i=16; i>7; i--) {
            if (bittime % i == 0) {
                break;
            }
        }
        for (j=17; j<26; j++) {
            if (bittime % j == 0) {
                break;
            }
        }
        if (i != 7) {               /* [8,16]有可用的量子数 */
            if (j != 26) {          /* [17,26]有可用的量子数 */
                if (16-i > j-16) {  /* 选择离16最近的量子 */
                    i=j;
                }
            }
        } else if (j != 26 ) {      /* [8,16]无可用的量子数 */
            i = j;                  /* [17,26]有可用的量子数 */
        }
        if (i == 7 || bittime/i>256) {
            FLEXCAN_HAL_SelectClock(CAN0, backup_clk_source);
            m_flexcan_config.m_flexcan_user_config.pe_clock = backup_clk_source;
            return STATUS_ERROR;
        } else {
            presdiv = bittime/i-1;   /* 预分频. */
            pseg2=i/5;
            if (pseg2 < 2) {         /* pseg2至少是2 */
                pseg2 = 2;
            }
            pseg1 = 1;               /* pseg1至少是1 */
            propseg = i-pseg2-pseg1-1; /* propseg最大化 */
            if (propseg > 8) {       /* propseg超过8,将多余的部分分到pseg1上 */
                pseg1 += propseg-8;
                propseg = 8;
            }
            if (pseg1 > 8) {         /* pseg1超过8,将多余部分加到pseg2上 */
                pseg2 += pseg1 - 8;
                pseg1 = 8;
            }
            (pseg2)--;
            (pseg1)--;
            (propseg)--;
            rjw = pseg1;
            if (rjw > pseg2) {
                rjw = pseg2;
            }
            if (rjw > 3) {
                rjw = 3;
            }

            m_flexcan_config.m_flexcan_user_config.bitrate.preDivider = presdiv;
            m_flexcan_config.m_flexcan_user_config.bitrate.propSeg    = propseg;
            m_flexcan_config.m_flexcan_user_config.bitrate.phaseSeg1  = pseg1;

            m_flexcan_config.m_flexcan_user_config.bitrate.phaseSeg2  = pseg2;
            m_flexcan_config.m_flexcan_user_config.bitrate.rJumpwidth = rjw;

            FLEXCAN_HAL_Enable(CAN0);
            FLEXCAN_DRV_SetBitrate(INST_CANCOM0,
                    &m_flexcan_config.m_flexcan_user_config.bitrate);
            return STATUS_SUCCESS;
        }

    }
    return STATUS_ERROR;
}

