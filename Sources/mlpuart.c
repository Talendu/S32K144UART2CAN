/*
 * mlpuart.c
 *
 *  Created on: 2018年4月17日
 *      Author: Administrator
 */

#include "mlpuart.h"

#define __LPUART_FIFO_SIZE  1024         /**< \brief 透传模式,串口接收缓冲区大小 */
#define __LPUSART_REC_LEN   32           /**< \brief 配置模式,串口接收缓冲区大小 */


uint8_t           g_uart_rx_buff[__LPUSART_REC_LEN];        /**< \brief 接收缓冲,最大USART_REC_LEN个字节. */
lpuart_rx_frame_t __g_lpuart_rx_base[__LPUART_FIFO_SIZE];   /**< \brief 透传模式接收缓冲区内存块 */

/**
 * \brief   接收状态
 *
 * bit15    接收完成标志
 * bit14    接收到0x0d
 * bit13~0  接收到的有效字节数目
 */
uint16_t   g_uart_rx_sta=0;
fifo_t     g_lpuart_rx_fifo;
uint8_t    lpuart_rx_len = 0;

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

/**
 * \brief   透明传输串口中断回掉函数
 */
void lpuart_RX_callback_transmission()
{
    switch(g_m_lpuart0_config.rxmode) {
    case 0:
    {
        if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_DATA_REG_FULL)) {
            /* 接收到数据触发的中断 */
            while (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_DATA_REG_FULL)) {
                lpuart_rx_frame_t *frame;
                if (lpuart_rx_len == 8) {
                    lpuart_rx_len = 0;
                    if (fifo_get_rear_pointer(&g_lpuart_rx_fifo, (void**)&frame)
                            == STATUS_SUCCESS) {
                        frame->descriptor = 0x08;
                        fifo_append(&g_lpuart_rx_fifo);
                    }
                }
                if (fifo_get_rear_pointer(&g_lpuart_rx_fifo, (void**)&frame)
                        != STATUS_SUCCESS) {
                    GPIO_HAL_ClearPins(PTC, 1<<11);
                    return;
                }

                LPUART_HAL_Getchar(LPUART0, &(frame->data[lpuart_rx_len]));
                lpuart_rx_len++;
                frame->descriptor = lpuart_rx_len;
            }
        } else if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_IDLE_LINE_DETECT)) {
            /* 串口空闲触发的中断 */
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
 * \brief   处于配置参数模式时,串口中断回掉函数
 *
 * \note    接收指令时,指令必须由"\r\n结束"
 */
void lpuart_RX_callback_config()
{
    if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_DATA_REG_FULL)) {
        uint8_t Res;
        LPUART_HAL_Getchar(LPUART0, &Res);/* 读取接收到的数据 */

        if((g_uart_rx_sta&0x8000)==0)/* 接收未完成 */
        {
            if(g_uart_rx_sta&0x4000)     /* 接收到了0x0d */
            {
                if(Res != 0x0a) {           /* 接收错误,重新开始 */
                    g_uart_rx_sta=0;
                } else {                    /* 接收完成了 */
                    g_uart_rx_sta|=0x8000;
                }
            } else {                    /* 还没收到0X0D */
                if(Res==0x0d) {             /* 收到0X0D */
                    g_uart_rx_sta|=0x4000;
                } else {                    /* 接到数据 */
                    g_uart_rx_buff[g_uart_rx_sta&0X3FFF] = Res;
                    g_uart_rx_sta++;
                    if(g_uart_rx_sta>(__LPUSART_REC_LEN-1)) { /* 接收数据错误,重新开始接收 */
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
 * \brief   初始化串口
 *
 * \param   lpuartUserConfig    串口配置信息
 */
status_t LPUART0_init(lpuart_user_config_t *p_lpuartUserConfig)
{
    status_t statu;
    uint32_t lpuartSourceClock;
    fifo_init(&g_lpuart_rx_fifo, __g_lpuart_rx_base,
            __LPUART_FIFO_SIZE, sizeof(lpuart_rx_frame_t));

    (void)CLOCK_SYS_GetFreq(PCC_LPUART0_CLOCK, &lpuartSourceClock);

    LPUART_HAL_Init(LPUART0);
    statu = LPUART_HAL_SetBaudRate(LPUART0, lpuartSourceClock,
            p_lpuartUserConfig->baudRate);
    if (statu != STATUS_SUCCESS) {
        return statu;
    }

    LPUART_HAL_SetBitCountPerChar(LPUART0, p_lpuartUserConfig->bitCountPerChar);
    LPUART_HAL_SetParityMode(LPUART0, p_lpuartUserConfig->parityMode);
    LPUART_HAL_SetStopBitCount(LPUART0, p_lpuartUserConfig->stopBitCount);

#if __USE_TXRX_FIFO == 1
    /* 使用FIFO */
    LPUART_HAL_SetTransmitterCmd(LPUART0, false);
    LPUART_HAL_SetReceiverCmd(LPUART0, false);
    LPUART_HAL_SetTxFIFOCmd(LPUART0, true);
    LPUART_HAL_SetRxFIFOCmd(LPUART0, true);
#endif
    LPUART_HAL_SetTransmitterCmd(LPUART0, true);
    LPUART_HAL_SetReceiverCmd(LPUART0, true);

    /* 使能串口接收和串口空闲中断 */
    LPUART_HAL_SetIntMode(LPUART0, LPUART_INT_IDLE_LINE, true);
    LPUART_HAL_SetIntMode(LPUART0, LPUART_INT_RX_DATA_REG_FULL, true);

    /* 初始化为透明传输模式 */
    LPUART_InstallRxCallback(lpuart_RX_callback_transmission);
    INT_SYS_EnableIRQ(LPUART0_RxTx_IRQn);
    return STATUS_SUCCESS;
}

/**
 * \brief   为串口设置接收中断回掉函数
 *
 * \param   callback 回掉函数
 */
void LPUART_InstallRxCallback(lpuart0_irq_callback_t callback)
{
    __gfn_lpuart0_irq_callback = callback;
}


/**
 * \brief   串口中断服务函数
 *
 * \details 如果设置回掉函数,将调用回掉函数,如果没有设置,将清除中断标志位
 */
void LPUART0_RxTx_IRQHandler()
{
    if (__gfn_lpuart0_irq_callback != NULL) {
        __gfn_lpuart0_irq_callback();
    } else {
        if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_DATA_REG_FULL)) {
            LPUART_HAL_FlushRxFifoBuffer(LPUART0);
        } else if (LPUART_HAL_GetStatusFlag(LPUART0, LPUART_IDLE_LINE_DETECT)) {
            LPUART_HAL_ClearStatusFlag(LPUART0, LPUART_IDLE_LINE_DETECT);
        }
    }
}

/**
 * \brief   设置串口波特率
 *
 * \param   baud    要设置的波特率
 *
 * \retval  STATUS_SUCCESS  设置成功
 *          STATUS_ERROR    设置失败
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
 * \brief   获得波特率
 *
 * \retval  串口波特率
 *
 * \note    该函数将返回串口0当前实际的波特率,和标准波特率之间有误差
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
 * \brief   从串口0发送一个字符数据
 */
void LPUART0_transmit_char(char send)
{
  while(!LPUART_HAL_GetStatusFlag(LPUART0, LPUART_TX_DATA_REG_EMPTY));
  LPUART_HAL_Putchar(LPUART0, send);
}

/**
 * \brief   从串口0发送数据
 *
 * \param   buffer[in]  要发送的数据
 * \param   len[in]     要发送的数据的长度
 */
void LPUART0_trancemit_buffer(const uint8_t *buffer, uint32_t len)
{
    uint32_t idx;
    for (idx=0; idx<len; idx++) {
        while (!LPUART_HAL_GetStatusFlag(LPUART0, LPUART_TX_DATA_REG_EMPTY));
        LPUART_HAL_Putchar(LPUART0, buffer[idx]);
    }
}

/**
 * \brief   串口打印字符串
 *
 * \param   data_string[in]     要打印的字符串
 *
 * \note    字符串必须以'\0'结束
 */
void LPUART0_transmit_string(const char *data_string)
{
  uint32_t i=0;
  while(data_string[i] != '\0')  {
    LPUART0_transmit_char(data_string[i]);
    i++;
  }
}

/**
 * \brief   串口接收一个字符
 */
uint8_t LPUART0_receive_byte(void)
{
  uint8_t receive;
  while(!LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_DATA_REG_FULL));
  LPUART_HAL_Getchar(LPUART0, &receive);
  return receive;
}

/*
 * \brief   将数字以字符串的形式通过串口发送
 *
 * \param   LPUARTx     向哪一个串口发送
 * \param   number      要发送的数字
 *
 * \retval  数字长度,及数字位数
 *
 * \note    只能发送32位有符号数字范围类的数字
 */
uint8_t LPUART_transmit_number(LPUART_Type *LPUARTx, int32_t number)
{
    uint8_t bit_of_number[10] = {0};    /* 32位有符号数最多10位十进制数. */
    uint8_t len, i;
    if (number < 0) {       /* 如果是负数发送负号 */
        while((LPUARTx->STAT & LPUART_STAT_TDRE_MASK) == 0); /* 等待串口发送完成 */
        LPUARTx->DATA = '-';
        number = -number;   /* 将数字变为正数,便于后面的计算 */
    }
    for(len=0;len<10;len++) {           /* 获取十进制数的每一位 */
        bit_of_number[len] = number % 10;
        number /= 10;
        if (number == 0) {
            len++;
            break;
        }
    }
    for(i=len;i;i--) {
        while((LPUARTx->STAT & LPUART_STAT_TDRE_MASK)==0); /* 等待串口发送完成 */
        LPUARTx->DATA = '0'+bit_of_number[i-1];
    }
    return len;
}
