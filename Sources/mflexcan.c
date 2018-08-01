/*
 * flexcan.c
 *
 *  Created on: 2018��4��8��
 *      Author: Administrator
 */
#include "mflexcan.h"

flexcan_msgbuff_t g_can_receive_buff;

/**
 * \brief   ��������������Ϣ
 */
flexcan_data_info_t g_rx_info = {
        .data_length = 64,
        .enable_brs = 1,
        .fd_enable = 0,
        .fd_padding = 0,
        .is_remote = 0,
        .msg_id_type = FLEXCAN_MSG_ID_STD
};

/**
 * \brief   ��������������Ϣ
 */
flexcan_data_info_t g_tx_info = {
        .data_length = 64,
        .enable_brs = 1,
        .fd_enable = 0,
        .fd_padding = 0,
        .is_remote = 0,
        .msg_id_type = FLEXCAN_MSG_ID_STD
};

/**
 * \brief   CAN���ݽ��ջ�����
 */
fifo_t g_flexcan_rx_fifo;

/**
 * \brief   CAN���ݻ������ڴ�ռ�
 */
FLEXCAN_RX_DATA_TYPE __g_flexcan_rx_base[__FLEXCAN_RX_FIFO_SIZE];

/**
 * \brief   ͸������ģʽCAN���ջص�����
 *
 * \param   instance    CANͨ��
 * \param   eventType   �ж�����
 * \param   state       CANͨ��״̬��Ϣ
 */
void can_callback_transmission(uint8_t               instance,
                               flexcan_event_type_t  eventType,
                               struct FlexCANState  *state)
{
    if (eventType == FLEXCAN_EVENT_RX_COMPLETE){        /* ����ǽ�������ж� */
        FLEXCAN_RX_DATA_TYPE *fifo_rear;
        uint8_t i = 0;
        for (i=0; i<g_can_receive_buff.dataLen; i++) {    /* �������е����ݸ��Ƶ������� */
            if (fifo_get_rear_pointer(&g_flexcan_rx_fifo, (void**)&fifo_rear)
                    == STATUS_SUCCESS) {                /* �жϻ������Ƿ�Ϊ�� */
                /* ������û����,�������� */
                *fifo_rear = g_can_receive_buff.data[i];
                fifo_append(&g_flexcan_rx_fifo);
            } else {
                /* ��������,LED1����,���������ж� */
                GPIO_HAL_ClearPins(PTC, 1<<11);
                break;
            }
        }
        if (g_m_flexcan_config.rxmode == 0) {         /* ģʽ0,ֻ���ܱ�׼֡ */
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);
        } else if (g_m_flexcan_config.rxmode == 1) {  /* ģʽ1,ֻ������չ֡ */
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
        } else if (g_m_flexcan_config.rxmode == 2) {  /* ģʽ1,��������֡ */
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);
            FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
        }
    }
}

/**
 * \brief   ��������ģʽCAN���ջص�����
 *
 * \param   instance    CANͨ��
 * \param   eventType   �ж�����
 * \param   state       CANͨ��״̬��Ϣ
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
 * \brief   ��ʼ��CAN
 */
void flexcan_init(void)
{
    fifo_init(&g_flexcan_rx_fifo,                                   /* can����fifo; */
            __g_flexcan_rx_base,                                      /* can����fifo�ڴ�ռ�; */
            sizeof(__g_flexcan_rx_base)/sizeof(FLEXCAN_RX_DATA_TYPE), /* can����fifoԪ������,���ڴ��С; */
            sizeof(FLEXCAN_RX_DATA_TYPE));                        /* can���������������ʹ�С. */

    /* ��λCAN0 */
    FLEXCAN_DRV_Deinit(INST_CANCOM0);

    /* ��ʼ��CAN0 */
    FLEXCAN_DRV_Init(INST_CANCOM0, &canCom0_State,
            &g_m_flexcan_config.m_flexcan_user_config);
    /* �������������ɻص����� */
    FLEXCAN_DRV_InstallEventCallback(INST_CANCOM0,
            can_callback_transmission, NULL);

    /* ����ȫ��ID������ */
    if (g_m_flexcan_config.rxmode == 0) {         /* ģʽ0,ֻ���ܱ�׼֡ */
        flexcan_init_STD_MB();
    } else if (g_m_flexcan_config.rxmode == 1) {  /* ģʽ1,ֻ������չ֡ */
        flexcan_init_EXT_MB();
    } else if (g_m_flexcan_config.rxmode == 2) {  /* ģʽ1,��������֡ */
        if (g_m_flexcan_config.rx_id == 0) {      /* ID����0,�������б�׼֡����չ֡ */
            flexcan_init_STD_MB();
            flexcan_init_EXT_MB();
            FLEXCAN_DRV_SetRxMbGlobalMask(INST_CANCOM0,
                        FLEXCAN_MSG_ID_EXT, 0);
        } else if ((g_m_flexcan_config.rx_id & (~0x7ff)) != 0) {
            flexcan_init_EXT_MB();
        } else {
            flexcan_init_STD_MB();
        }
    }
}

/**
 * \brief   ��ʼ����׼֡����
 */
void flexcan_init_STD_MB()
{
    FLEXCAN_DRV_SetRxMbGlobalMask(INST_CANCOM0,
                FLEXCAN_MSG_ID_STD, g_m_flexcan_config.id_mask);

    g_rx_info.fd_enable = g_m_flexcan_config.fd_enable;
    g_rx_info.msg_id_type = FLEXCAN_MSG_ID_STD;       /* ��׼֡�������� */
    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_STD_MB,
            &g_rx_info, g_m_flexcan_config.rx_id);

    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);
}

/**
 * \brief   ��ʼ��Զ��֡����
 */
void flexcan_init_EXT_MB()
{
    FLEXCAN_DRV_SetRxMbGlobalMask(INST_CANCOM0,
                FLEXCAN_MSG_ID_EXT, g_m_flexcan_config.id_mask);

    g_rx_info.fd_enable = g_m_flexcan_config.fd_enable;
    g_rx_info.msg_id_type = FLEXCAN_MSG_ID_EXT;       /* ��չ֡�������� */
    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_EXT_MB,
            &g_rx_info, g_m_flexcan_config.rx_id);

    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
}

/**
 * \brief       ��ȡcan0ʱ��Ƶ��
 *
 * \param[out]  p_flexcanSourceClock[out]   ʱ��Ƶ��
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
 * \brief   �޸�CAN������
 *
 * \param   p_parameter[in]     ������,���������ַ���
 * \retval  �޸ĳɹ�����: STATUS_SUCCESS
 *          �޸�ʧ�ܷ���: DTATUS_ERROR
 *
 * \note    ����ʹ������������16�ӽ��ķ�Ƶϵ��,
 *          �������ò������ڱ���ʱ���80%��
 */
status_t flexcan_set_baud(uint32_t baud)
{

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

    /* ���ڼ�������л�ı�ʱ��,������ʧ��CAN */
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
    if (i != 7) {               /* [8,16]�п��õ������� */
        if (j != 26) {          /* [17,26]�п��õ������� */
            if (16-i > j-16) {  /* ѡ����16��������� */
                i=j;
            }
        }
    } else if (j != 26 ) {      /* [8,16]�޿��õ������� */
        i = j;                  /* [17,26]�п��õ������� */
    }

    if (i == 7 || bittime/i>256) {  /* �޷����õ�Ŀ����� */
        FLEXCAN_HAL_SelectClock(CAN0, backup_clk_source);
        g_m_flexcan_config.m_flexcan_user_config.pe_clock = backup_clk_source;
        FLEXCAN_HAL_Enable(CAN0);
        return STATUS_ERROR;
    }

    presdiv = bittime/i-1;   /* Ԥ��Ƶ. */
    pseg2=i/5;
    if (pseg2 < 2) {         /* pseg2������2 */
        pseg2 = 2;
    }
    pseg1 = 1;               /* pseg1������1 */
    propseg = i-pseg2-pseg1-1; /* propseg��� */
    if (propseg > 8) {       /* propseg����8,������Ĳ��ֵַ�pseg1�� */
        pseg1 += propseg-8;
        propseg = 8;
    }
    if (pseg1 > 8) {         /* pseg1����8,�����ಿ�ּӵ�pseg2�� */
        pseg2 += pseg1 - 8;
        pseg1 = 8;
    }
    (pseg2)--;               /* �Ĵ����е�ֵ��ʵ��ֵ��һ */
    (pseg1)--;
    (propseg)--;

    rjw = pseg1;             /* ͬ��ƫ����ȡ3, pseg1, pseg2����С��*/
    if (rjw > pseg2) {
        rjw = pseg2;
    }
    if (rjw > 3) {
        rjw = 3;
    }

    /* ��ֵ������������Ϣ�� */
    g_m_flexcan_config.m_flexcan_user_config.bitrate.preDivider = presdiv;
    g_m_flexcan_config.m_flexcan_user_config.bitrate.propSeg    = propseg;
    g_m_flexcan_config.m_flexcan_user_config.bitrate.phaseSeg1  = pseg1;

    g_m_flexcan_config.m_flexcan_user_config.bitrate.phaseSeg2  = pseg2;
    g_m_flexcan_config.m_flexcan_user_config.bitrate.rJumpwidth = rjw;

    FLEXCAN_HAL_Enable(CAN0);
    /* ���ñ����� */
    FLEXCAN_DRV_SetBitrate(INST_CANCOM0,
            &g_m_flexcan_config.m_flexcan_user_config.bitrate);
    return STATUS_SUCCESS;
}

/**
 * \brief   �޸�CAN������
 *
 * \param   p_parameter[in]     ������,���������ַ���
 * \retval  �޸ĳɹ�����: STATUS_SUCCESS
 *          �޸�ʧ�ܷ���: DTATUS_ERROR
 *
 * \note    ����ʹ������������16�ӽ��ķ�Ƶϵ��,
 *          �������ò������ڱ���ʱ���80%��
 */
status_t flexcan_set_fdbaud(uint32_t baud)
{

    uint32_t presdiv = 0;
    uint32_t propseg = 0;
    uint32_t pseg1 = 0;

    uint32_t pseg2 = 0;
    uint32_t rjw = 0;
    uint32_t bittime = 0;

    uint32_t flexcanSourceClock = 0;

    uint8_t i = 0;
    uint8_t j = 0;

    /* ���ڼ�������л�ı�ʱ��,������ʧ��CAN */
    FLEXCAN_HAL_Disable(CAN0);
    flexcan_get_source_clock(&flexcanSourceClock);

    if (baud > (flexcanSourceClock/5))
    {
        return STATUS_ERROR;
    }

    bittime = flexcanSourceClock/baud;
    for (i=26; i>4; i--) {
        if (bittime % i == 0) {
            break;
        }
    }
    for (j=27; j<48; j++) {
        if (bittime % j == 0) {
            break;
        }
    }
    if (i != 4) {               /* [5,25]�п��õ������� */
        if (j != 49) {          /* [27,48]�п��õ������� */
            if (26-i > j-26) {  /* ѡ����26��������� */
                i=j;
            }
        }
    } else if (j != 49 ) {      /* [5,26]�޿��õ������� */
        i = j;                  /* [27,48]�п��õ������� */
    }

    LPUART0_transmit_string("\r\n");
    LPUART_transmit_number(LPUART0, i);

    if (i == 4 || bittime/i>256) {  /* �޷����õ�Ŀ����� */
        FLEXCAN_HAL_Enable(CAN0);
        return STATUS_ERROR;
    }

    presdiv = bittime/i-1;   /* Ԥ��Ƶ. */
    pseg2=i/5;
    if (pseg2 < 2) {         /* pseg2������2 */
        pseg2 = 2;
    }else if (pseg2 > 8){
        pseg2 = 8;
    }
    pseg1 = 1;               /* pseg1������1 */
    propseg = i-pseg2-pseg1-1; /* propseg��� */
    if (propseg > 31) {       /* propseg����8,������Ĳ��ֵַ�pseg1�� */
        pseg1 += propseg-31;
        propseg = 31;
    }
    if (pseg1 > 8) {         /* pseg1����8,�����ಿ�ּӵ�pseg2�� */
        pseg2 += pseg1 - 8;
        pseg1 = 8;
    }
    (pseg2)--;               /* �Ĵ����е�ֵ��ʵ��ֵ��һ */
    (pseg1)--;

    rjw = pseg1;             /* ͬ��ƫ����ȡ3, pseg1, pseg2����С��*/
    if (rjw > pseg2) {
        rjw = pseg2;
    }
    if (rjw > 3) {
        rjw = 3;
    }

    /* ��ֵ������������Ϣ�� */
    g_m_flexcan_config.m_flexcan_user_config.bitrate_cbt.preDivider = presdiv;
    g_m_flexcan_config.m_flexcan_user_config.bitrate_cbt.propSeg    = propseg;
    g_m_flexcan_config.m_flexcan_user_config.bitrate_cbt.phaseSeg1  = pseg1;

    g_m_flexcan_config.m_flexcan_user_config.bitrate_cbt.phaseSeg2  = pseg2;
    g_m_flexcan_config.m_flexcan_user_config.bitrate_cbt.rJumpwidth = rjw;


    FLEXCAN_HAL_Enable(CAN0);
    /* ���ñ����� */
    FLEXCAN_DRV_SetBitrateCbt(INST_CANCOM0,
            &g_m_flexcan_config.m_flexcan_user_config.bitrate_cbt);
    return STATUS_SUCCESS;
}

/**
 * \brief   ��ȡCAN������
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

/**
 * \brief   ��ȡCAN FD������
 */
uint32_t flexcan_get_fdbaud(void)
{
    uint32_t baud = 0;
    flexcan_time_segment_t *bitrate = &g_m_flexcan_config.m_flexcan_user_config.bitrate_cbt;
    uint32_t flexcanSourceClock;
    flexcan_get_source_clock(&flexcanSourceClock);
    baud = flexcanSourceClock/(bitrate->preDivider+1)/
            (bitrate->propSeg + bitrate->phaseSeg1 + bitrate->phaseSeg2 + 3);
    return baud;
}
