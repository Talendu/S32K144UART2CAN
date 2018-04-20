/*
 * mlpuart.c
 *
 *  Created on: 2018��4��17��
 *      Author: Administrator
 */

#include "mlpuart.h"

#define USART_REC_LEN 32
uint8_t g_uart_rx_buff[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.

/*
 * ����״̬
 * bit15��    ������ɱ�־
 * bit14��    ���յ�0x0d
 * bit13~0��  ���յ�����Ч�ֽ���Ŀ
 */
uint16_t g_uart_rx_sta=0;


fifo_t              g_lpuart_rx_fifo;
lpuart_rx_frame_t   __g_lpuart_rx_base[1024];
uint8_t             lpuart_rx_len = 0;

lpuart0_irq_callback_t __gfn_lpuart0_irq_callback = NULL;

const lpuart_user_config_t g_lpuart_default_UserConfig = {
        .baudRate = 115200,
        .bitCountPerChar = LPUART_8_BITS_PER_CHAR,
        .parityMode = LPUART_PARITY_DISABLED,
        .rxDMAChannel = 0,
        .stopBitCount  = LPUART_ONE_STOP_BIT,
        .transferType = LPUART_USING_INTERRUPTS,
        .txDMAChannel = 0
};



void lpuart_RX_callback_transparent_transmission()
{
    switch(g_m_lpuart0_config.rxmode) {
    case 0:
    {
        if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_DATA_REG_FULL)) {
            while ((LPUART0->FIFO & LPUART_FIFO_RXEMPT_MASK) == 0) {
                lpuart_rx_frame_t *frame;
                if (fifo_get_rear_pointer(&g_lpuart_rx_fifo, (void**)&frame) != STATUS_SUCCESS) {
                    GPIO_HAL_ClearPins(PTC, 1<<11);
                    return;
                }

                frame->data[lpuart_rx_len] = (uint8_t)LPUART0->DATA;
                lpuart_rx_len++;
                if (lpuart_rx_len == 8) {
                    lpuart_rx_len = 0;
                    frame->descriptor = 0x08;
                    fifo_append(&g_lpuart_rx_fifo);
                }
            }
        } else if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_IDLE_LINE_DETECT)) {
            lpuart_rx_frame_t *frame;
            if (fifo_get_rear_pointer(&g_lpuart_rx_fifo, (void**)&frame) != STATUS_SUCCESS) {
                GPIO_HAL_ClearPins(PTC, 1<<11);
                return;
            }
            frame->descriptor = lpuart_rx_len|0x80;
            lpuart_rx_len = 0;
            fifo_append(&g_lpuart_rx_fifo);
            GPIO_HAL_SetPins(PTC, 1<<12);
            LPUART_HAL_ClearStatusFlag(LPUART0, LPUART_IDLE_LINE_DETECT);
        }
        break;
    }
    case 1:
        break;
    default:
        break;
    }
}

/**
 * \brief   �������ò���ģʽʱ,�����жϻص�����
 * \note    ����ָ��ʱ,ָ�������"\r\n����"
 */
void lpuart_RX_callback_configuration_parameters()
{
    if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_DATA_REG_FULL)) {
        uint8_t Res;
        Res = LPUART0->DATA; /* ��ȡ���յ������� */

        if((g_uart_rx_sta&0x8000)==0)/* ����δ��� */
        {
            if(g_uart_rx_sta&0x4000)     /* ���յ���0x0d */
            {
                if(Res != 0x0a) {           /* ���մ���,���¿�ʼ */
                    g_uart_rx_sta=0;
                } else {                    /* ��������� */
                    g_uart_rx_sta|=0x8000;
                }
            } else {                    /* ��û�յ�0X0D */
                if(Res==0x0d) {             /* �յ�0X0D */
                    g_uart_rx_sta|=0x4000;
                } else {                    /* �ӵ����� */
                    g_uart_rx_buff[g_uart_rx_sta&0X3FFF] = Res;
                    g_uart_rx_sta++;
                    if(g_uart_rx_sta>(USART_REC_LEN-1)) { /* �������ݴ���,���¿�ʼ���� */
                        g_uart_rx_sta = 0;
                    }
                }
            }
        }
    } else if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_IDLE_LINE_DETECT)) {
        LPUART_HAL_ClearStatusFlag(LPUART0, LPUART_IDLE_LINE_DETECT);
    }
}

/**
 * \brief   ��ʼ������
 * \param   lpuartUserConfig    ����������Ϣ
 */
status_t LPUART0_init(lpuart_user_config_t *lpuartUserConfig)
{
    status_t statu;
    uint32_t lpuartSourceClock;
    fifo_init(&g_lpuart_rx_fifo, __g_lpuart_rx_base, 1024, sizeof(lpuart_rx_frame_t));

    (void)CLOCK_SYS_GetFreq(PCC_LPUART0_CLOCK, &lpuartSourceClock);

    LPUART_HAL_Init(LPUART0);
    statu = LPUART_HAL_SetBaudRate(LPUART0, lpuartSourceClock,
            lpuartUserConfig->baudRate);
    if (statu != STATUS_SUCCESS) {
        return statu;
    }

    LPUART_HAL_SetBitCountPerChar(LPUART0, lpuartUserConfig->bitCountPerChar);
    LPUART_HAL_SetParityMode(LPUART0, lpuartUserConfig->parityMode);
    LPUART_HAL_SetStopBitCount(LPUART0, lpuartUserConfig->stopBitCount);

    /* ʹ��FIFO */
    LPUART_HAL_SetTransmitterCmd(LPUART0, false);
    LPUART_HAL_SetReceiverCmd(LPUART0, false);
    LPUART0->FIFO |= LPUART_FIFO_RXFE(1);
    LPUART0->FIFO |= LPUART_FIFO_TXFE(1);
    LPUART_HAL_SetTransmitterCmd(LPUART0, true);
    LPUART_HAL_SetReceiverCmd(LPUART0, true);

    /* ʹ�ܴ��ڽ��պʹ��ڿ����ж� */
    LPUART_HAL_SetIntMode(LPUART0, LPUART_INT_IDLE_LINE, true);
    LPUART_HAL_SetIntMode(LPUART0, LPUART_INT_RX_DATA_REG_FULL, true);

    /* ��ʼ��Ϊ͸������ģʽ */
    LPUART_InstallRxCallback(lpuart_RX_callback_transparent_transmission);
    INT_SYS_EnableIRQ(LPUART0_RxTx_IRQn);
    return STATUS_SUCCESS;
}

/**
 * \brief   Ϊ�������ý����жϻص�����
 * \param   callback �ٵ�����
 */
void LPUART_InstallRxCallback(lpuart0_irq_callback_t callback)
{
    __gfn_lpuart0_irq_callback = callback;
}


/**
 * \brief   �����жϷ�����
 * \details ������ûص�����,�����ûص�����,���û������,������жϱ�־λ
 */
void LPUART0_RxTx_IRQHandler()
{
    if (__gfn_lpuart0_irq_callback != NULL) {
        __gfn_lpuart0_irq_callback();
    } else {
        if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_DATA_REG_FULL)) {
            LPUART0->FIFO |= LPUART_FIFO_RXFLUSH_MASK;
        } else if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_IDLE_LINE_DETECT)) {
            LPUART_HAL_ClearStatusFlag(LPUART0, LPUART_IDLE_LINE_DETECT);
        }
    }
}


/**
 * \brief   ���ô��ڲ�����
 *
 */
status_t LPUART0_set_baud(uint32_t baud)
{
    uint32_t lpuartSourceClock;
    status_t status = STATUS_SUCCESS;
    CLOCK_SYS_GetFreq(PCC_LPUART0_CLOCK, &lpuartSourceClock);
    status = LPUART_HAL_SetBaudRate(LPUART0, lpuartSourceClock, baud);
    if(status == STATUS_SUCCESS) {
        g_m_lpuart0_config.lpuart0_user_config.baudRate = baud;
    }
    return status;
}

/**
 * \brief   ��ò�����
 * \note    �ú��������ش���0��ǰʵ�ʵĲ�����,�ͱ�׼������֮�������
 */
uint32_t LPUART0_get_baud()
{
    uint32_t lpuartSourceClock;
    uint16_t osr = (LPUART0->BAUD & LPUART_BAUD_OSR_MASK)>>LPUART_BAUD_OSR_SHIFT;
    uint16_t sbr = (LPUART0->BAUD & LPUART_BAUD_SBR_MASK)>>LPUART_BAUD_SBR_SHIFT;
    CLOCK_SYS_GetFreq(PCC_LPUART0_CLOCK, &lpuartSourceClock);

    return lpuartSourceClock/(sbr*(osr+1));
}

/**
 * \brief   �Ӵ���0����һ���ַ�����
 */
void LPUART0_transmit_char(char send)
{
  while((LPUART0->STAT & LPUART_STAT_TDRE_MASK) == 0);
  LPUART0->DATA = send;
}

/**
 * \brief   �Ӵ���0��������
 * \param   buffer[in]  Ҫ���͵�����
 * \param   len[in]     Ҫ���͵����ݵĳ���
 */
void LPUART0_trancemit_buffer(const uint8_t *buffer, uint32_t len)
{
    uint32_t idx;
    for (idx=0; idx<len; idx++) {
        while ((LPUART0->STAT & LPUART_STAT_TDRE_MASK) == 0);
        LPUART0->DATA = buffer[idx];
    }
}

void LPUART0_transmit_string(const char *data_string)
{
  uint32_t i=0;
  while(data_string[i] != '\0')  {
    LPUART0_transmit_char(data_string[i]);
    i++;
  }
}

char LPUART0_receive_char(void)
{
  char recieve;
  while((LPUART0->STAT & LPUART_STAT_RDRF_MASK) == 0){

  }
  recieve= LPUART0->DATA;
  return recieve;
}

/*
 * \brief   ���������ַ�������ʽͨ�����ڷ���
 *
 * \param   LPUARTx     ����һ�����ڷ���
 * \param   number      Ҫ���͵�����
 *
 * \retval  ���ֳ���,������λ��
 *
 * \note    ֻ�ܷ���32λ�з������ַ�Χ�������
 */
uint8_t LPUART_transmit_number(LPUART_Type *LPUARTx, int32_t number)
{
    uint8_t bit_of_number[10] = {0};    /* 32λ�з��������10λʮ������. */
    uint8_t len, i;
    if (number < 0) {       /* ����Ǹ������͸��� */
        while((LPUARTx->STAT & LPUART_STAT_TDRE_MASK) == 0); /* �ȴ����ڷ������ */
        LPUARTx->DATA = '-';
        number = -number;   /* �����ֱ�Ϊ����,���ں���ļ��� */
    }
    for(len=0;len<10;len++) {           /* ��ȡʮ��������ÿһλ */
        bit_of_number[len] = number % 10;
        number /= 10;
        if (number == 0) {
            len++;
            break;
        }
    }
    for(i=len;i;i--) {
        while((LPUARTx->STAT & LPUART_STAT_TDRE_MASK)==0); /* �ȴ����ڷ������ */
        LPUARTx->DATA = '0'+bit_of_number[i-1];
    }
    return len;
}
