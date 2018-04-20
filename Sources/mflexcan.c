/*
 * flexcan.c
 *
 *  Created on: 2018��4��8��
 *      Author: Administrator
 */
#include "mflexcan.h"

flexcan_msgbuff_t g_can_receive_buff;
uint8_t           g_lpuart_receive_buff[8];

m_flexcan_config_t g_m_flexcan_config;

/**
 * \brief   ��������������Ϣ
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
 * \brief   ��������������Ϣ
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
 * \brief   CAN���ݽ��ջ�����
 */
fifo_t g_flexcan_rx_fifo;
/**
 * \brief   CAN���ݻ������ڴ�ռ�
 */
FLEXCAN_RX_DATA_TYPE __g_flexcan_rx_base[__FLEXCAN_RX_FIFO_SIZE];


status_t flexcan_set_baud(uint8_t *p_parameter);

/**
 * \brief   ͸������ģʽCAN���ջص�����
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
void init_flexcan(void)
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
    /* ����ȫ��ID������ */
    FLEXCAN_DRV_SetRxMbGlobalMask(INST_CANCOM0,
            FLEXCAN_MSG_ID_EXT, g_m_flexcan_config.id_mask);

    /* ���ý���������Ϣ */
    FLEXCAN_DRV_InstallEventCallback(INST_CANCOM0,
            can_callback_transmission, NULL); /* �������������ɻص����� */
    g_rx_info.msg_id_type = FLEXCAN_MSG_ID_STD;       /* ��׼֡�������� */
    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_STD_MB,
            &g_rx_info, g_m_flexcan_config.rx_id);
    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_STD_MB, &g_can_receive_buff);

    g_rx_info.msg_id_type = FLEXCAN_MSG_ID_EXT;       /* ��չ֡�������� */
    FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_EXT_MB,
            &g_rx_info, g_m_flexcan_config.rx_id);
    FLEXCAN_DRV_Receive(INST_CANCOM0, RECEIVE_EXT_MB, &g_can_receive_buff);
}

/**
 * \brief   ͨ���������ò���
 *
 * \param   config_item_index   ��������
 * \param   p_parameter[in]     ����(�����ַ���)
 * \param   parameter_len       ��������
 */
void LPUART0_config_can_by(config_item_index_t  config_item_index,
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
            if (g_m_flexcan_config.txmode == 0) {             /* ����ģʽ0 */
                txid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;  /* ��׼֡ID */
            } else if (g_m_flexcan_config.txmode == 1) {      /* ����ģʽ1 */
                txid &= CAN_ID_EXT_MASK;                    /* ��չ֡ID */
            } else if (g_m_flexcan_config.txmode == 2) {      /* ����ģʽ2 */
                if ((txid&CAN_ID_EXT_MASK) > 0x7ff) {         /* ����IDѡ����֡��ģʽ, */
                    txid &= CAN_ID_EXT_MASK;                /* ID������׼֡ID��Χ. */
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
            if (g_m_flexcan_config.rxmode == 0) {             /* ����ģʽ0 */
                rxid &= CAN_ID_STD_MASK>>CAN_ID_STD_SHIFT;  /* ��׼֡ID */
                FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_STD_MB,
                        &g_rx_info, rxid);
            } else if (g_m_flexcan_config.rxmode == 1) {      /* ����ģʽ1 */
                rxid &= CAN_ID_EXT_MASK;                    /* ��չ֡ID */
                FLEXCAN_DRV_ConfigRxMb(INST_CANCOM0, RECEIVE_EXT_MB,
                        &g_rx_info, rxid);
            } else if (g_m_flexcan_config.rxmode == 2) {      /* ����ģʽ2 */
                if ((rxid&CAN_ID_EXT_MASK) > 0x7ff) {   /* ����IDѡ����֡��ģʽ, */
                    rxid &= CAN_ID_EXT_MASK;            /* ID������׼֡ID��Χ. */
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
 * \brief       �Ӵ��ڴ�ӡ����
 * \param[in]   config_item_index   �����ڲ��������е�λ��
 *                                  ��ο� config_item_index_t �� config_item
 */
void LPUART0_print_can_config_by(config_item_index_t config_item_index)
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

/**
 * \brief       ��ȡcan0ʱ��Ƶ��
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
 * \param   p_parameter[in]     ������,���������ַ���
 * \retval  �޸ĳɹ�����: STATUS_SUCCESS
 *          �޸�ʧ�ܷ���: DTATUS_ERROR
 */
status_t flexcan_set_baud(uint8_t *p_parameter)
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

    if (string2number(p_parameter, &baud) == STATUS_SUCCESS) {
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
        if (i == 7 || bittime/i>256) {
            FLEXCAN_HAL_SelectClock(CAN0, backup_clk_source);
            g_m_flexcan_config.m_flexcan_user_config.pe_clock = backup_clk_source;
            return STATUS_ERROR;
        } else {
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

            g_m_flexcan_config.m_flexcan_user_config.bitrate.preDivider = presdiv;
            g_m_flexcan_config.m_flexcan_user_config.bitrate.propSeg    = propseg;
            g_m_flexcan_config.m_flexcan_user_config.bitrate.phaseSeg1  = pseg1;

            g_m_flexcan_config.m_flexcan_user_config.bitrate.phaseSeg2  = pseg2;
            g_m_flexcan_config.m_flexcan_user_config.bitrate.rJumpwidth = rjw;

            FLEXCAN_HAL_Enable(CAN0);
            FLEXCAN_DRV_SetBitrate(INST_CANCOM0,
                    &g_m_flexcan_config.m_flexcan_user_config.bitrate);
            return STATUS_SUCCESS;
        }

    }
    return STATUS_ERROR;
}

