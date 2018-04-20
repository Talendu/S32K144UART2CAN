/*
 * flexcan.c
 *
 *  Created on: 2018年4月8日
 *      Author: Administrator
 */
#include "mflexcan.h"

flexcan_msgbuff_t g_can_receive_buff;

/**
 * \brief   发送邮箱配置信息
 */
flexcan_data_info_t g_rx_info = {
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
flexcan_data_info_t g_tx_info = {
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
fifo_t g_flexcan_rx_fifo;

/**
 * \brief   CAN数据缓冲区内存空间
 */
FLEXCAN_RX_DATA_TYPE __g_flexcan_rx_base[__FLEXCAN_RX_FIFO_SIZE];

/**
 * \brief   透明传输模式CAN接收回掉函数
 * \param   instance    CAN通道
 * \param   eventType   中断类型
 * \param   state       CAN通道状态信息
 */
void can_callback_transmission(uint8_t               instance,
                               flexcan_event_type_t  eventType,
                               struct FlexCANState  *state)
{
    if (eventType == FLEXCAN_EVENT_RX_COMPLETE){        /* 如果是接收完成中断 */
        FLEXCAN_RX_DATA_TYPE *fifo_rear;
        uint8_t i = 0;
        for (i=0; i<g_can_receive_buff.dataLen; i++) {    /* 将邮箱中的数据复制到缓冲区 */
            if (fifo_get_rear_pointer(&g_flexcan_rx_fifo, (void**)&fifo_rear)
                    == STATUS_SUCCESS) {                /* 判断缓冲区是否为满 */
                /* 缓冲区没有满,则复制数据 */
                *fifo_rear = g_can_receive_buff.data[i];
                fifo_append(&g_flexcan_rx_fifo);
            } else {
                /* 缓冲区满,LED1点亮,并且跳出中断 */
                GPIO_HAL_ClearPins(PTC, 1<<11);
                break;
            }
        }
        if (g_m_flexcan_config.rxmode == 0) {         /* 模式0,只接受标准帧 */
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);
        } else if (g_m_flexcan_config.rxmode == 1) {  /* 模式1,只接受扩展帧 */
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
        } else if (g_m_flexcan_config.rxmode == 2) {  /* 模式1,接收两种帧 */
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
void can_callback_config(uint8_t               instance,
                         flexcan_event_type_t  eventType,
                          struct FlexCANState  *state)
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
    fifo_init(&g_flexcan_rx_fifo,                                   /* can接收fifo; */
            __g_flexcan_rx_base,                                      /* can接收fifo内存空间; */
            sizeof(__g_flexcan_rx_base)/sizeof(FLEXCAN_RX_DATA_TYPE), /* can接收fifo元素容量,非内存大小; */
            sizeof(FLEXCAN_RX_DATA_TYPE));                        /* can接收数据数据类型大小. */
    /* 复位CAN0 */
    FLEXCAN_DRV_Deinit(INST_CANCOM0);
    /* 初始化CAN0 */
    FLEXCAN_DRV_Init(INST_CANCOM0, &canCom0_State,
            &g_m_flexcan_config.m_flexcan_user_config);
    /* 设置全局ID屏蔽码 */
    FLEXCAN_DRV_SetRxMbGlobalMask(INST_CANCOM0,
            FLEXCAN_MSG_ID_EXT, g_m_flexcan_config.id_mask);

    /* 配置接收邮箱信息 */
    FLEXCAN_DRV_InstallEventCallback(INST_CANCOM0,
            can_callback_transmission, NULL); /* 设置邮箱接收完成回掉函数 */
    g_rx_info.msg_id_type = FLEXCAN_MSG_ID_STD;       /* 标准帧接收邮箱 */
    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_STD_MB,
            &g_rx_info, g_m_flexcan_config.rx_id);
    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);

    g_rx_info.msg_id_type = FLEXCAN_MSG_ID_EXT;       /* 扩展帧接收邮箱 */
    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_EXT_MB,
            &g_rx_info, g_m_flexcan_config.rx_id);
    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
}

/**
 * \brief       获取can0时钟频率
 *
 * \param[out]  p_flexcanSourceClock[out]   时钟频率
 */
void flexcan_get_source_clock(uint32_t *p_flexcanSourceClock)
{
    if (g_m_flexcan_config.m_flexcan_user_config.pe_clock == FLEXCAN_CLK_SOURCE_SYS){
        CLOCK_SYS_GetFreq(CORE_CLOCK, p_flexcanSourceClock);
    } else {
        uint32_t i = (SCG->SOSCDIV & SCG_SOSCDIV_SOSCDIV2_MASK) >> SCG_SOSCDIV_SOSCDIV2_SHIFT;
        CLOCK_SYS_GetFreq(SOSC_CLOCK, p_flexcanSourceClock);
        for (i=i-1; i>0; i--) {
            *p_flexcanSourceClock >>= 1;
        }
    }
}

/**
 * \brief   修改CAN比特率
 *
 * \param   p_parameter[in]     比特率,输入数字字符串
 * \retval  修改成功返回: STATUS_SUCCESS
 *          修改失败返回: DTATUS_ERROR
 *
 * \note    优先使用量子数量与16接近的分频系数,
 *          优先设置采样点在比特时间的80%处
 */
status_t flexcan_set_baud(const uint8_t *p_parameter)
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

    backup_clk_source = g_m_flexcan_config.m_flexcan_user_config.pe_clock;

    if (string2number(p_parameter, &baud) != STATUS_SUCCESS) {
        return STATUS_ERROR;
    }
    /* 由于计算过程中会改变时钟,所以先失能CAN */
    FLEXCAN_HAL_Disable(CAN0);
    if (baud > 60000) {
        FLEXCAN_HAL_SelectClock(CAN0, FLEXCAN_CLK_SOURCE_SYS);
        g_m_flexcan_config.m_flexcan_user_config.pe_clock = FLEXCAN_CLK_SOURCE_SYS;
    } else {
        FLEXCAN_HAL_SelectClock(CAN0, FLEXCAN_CLK_SOURCE_SOSCDIV2);
        g_m_flexcan_config.m_flexcan_user_config.pe_clock = FLEXCAN_CLK_SOURCE_SOSCDIV2;
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

    if (i == 7 || bittime/i>256) {  /* 无法配置到目标参数 */
        FLEXCAN_HAL_SelectClock(CAN0, backup_clk_source);
        g_m_flexcan_config.m_flexcan_user_config.pe_clock = backup_clk_source;
        FLEXCAN_HAL_Enable(CAN0);
        return STATUS_ERROR;
    }

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
    (pseg2)--;               /* 寄存器中的值比实际值少一 */
    (pseg1)--;
    (propseg)--;

    rjw = pseg1;             /* 同步偏移量取3, pseg1, pseg2中最小的*/
    if (rjw > pseg2) {
        rjw = pseg2;
    }
    if (rjw > 3) {
        rjw = 3;
    }

    /* 将值拷贝到配置信息中 */
    g_m_flexcan_config.m_flexcan_user_config.bitrate.preDivider = presdiv;
    g_m_flexcan_config.m_flexcan_user_config.bitrate.propSeg    = propseg;
    g_m_flexcan_config.m_flexcan_user_config.bitrate.phaseSeg1  = pseg1;

    g_m_flexcan_config.m_flexcan_user_config.bitrate.phaseSeg2  = pseg2;
    g_m_flexcan_config.m_flexcan_user_config.bitrate.rJumpwidth = rjw;

    FLEXCAN_HAL_Enable(CAN0);
    /* 设置比特率 */
    FLEXCAN_DRV_SetBitrate(INST_CANCOM0,
            &g_m_flexcan_config.m_flexcan_user_config.bitrate);
    return STATUS_SUCCESS;
}

/**
 * \brief   获取CAN波特率
 */
uint32_t flexcan_get_baud(void)
{
    uint32_t baud = 0;
    flexcan_time_segment_t *bitrate = &g_m_flexcan_config.m_flexcan_user_config.bitrate;
    uint32_t flexcanSourceClock;
    flexcan_get_source_clock(&flexcanSourceClock);
    baud = flexcanSourceClock/(bitrate->preDivider+1)/
            (bitrate->propSeg + bitrate->phaseSeg1 + bitrate->phaseSeg2 + 4);
    return baud;
}
