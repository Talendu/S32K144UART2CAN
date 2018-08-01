/*
 * fifo.c
 *
 *  Created on: 2018��4��7��
 *      Author: Administrator
 */
#include "fifo.h"

/**
 * \brief   ��ʼ��fifo
 *
 * \param   p_fifo[in,out]      Ҫ��ʼ����fifo
 * \param   p_base[in]          fifo��Ӧ���ڴ��
 * \param   max_member_number   ����������Ա����
 *                              (�������ڴ��С = һ����Ա�Ĵ�С x ��Ա�������)
 * \param   menber_size         ��������һ����Առ�õ��ڴ�ռ�
 */
void fifo_init(fifo_t   *p_fifo,
               void     *p_base,
               uint32_t  max_member_number,
               uint32_t  menber_size) {
    p_fifo->base = (uint8_t*) p_base;
    p_fifo->size = max_member_number;
    p_fifo->member_size = menber_size;
    p_fifo->front = 0;
    p_fifo->rear = 0;
    p_fifo->is_empty = 1;
}

/**
 * \brief   ��ȡfifoβָ��
 *
 * \param   p_fifo[in]              Ҫ������fifo
 * \param   pp_rear_pointer[out]    fifoβָ��
 *
 * \retval  STATUS_SUCCESS  �ɹ���ȡָ��
 *          STATUS_ERROR    ��ȡfifoβָ��ʧ��,fifo��
 */
status_t fifo_get_rear_pointer(fifo_t *p_fifo, void **pp_rear_pointer) {
    if (p_fifo->rear == p_fifo->front       /* ����ͷβ��һ��,Ҫô�ǿ�,Ҫô����. */
            && p_fifo->is_empty == 0){    /* ������в��ǿ�. */
        *pp_rear_pointer = NULL;           /* ���ܻ�ȡͷָ��; */
        return STATUS_ERROR;
    } else {                            /* ������Ի�ȡ. */
        *pp_rear_pointer = p_fifo->base      /* fifo�ڴ��׵�ַ */
                + (p_fifo->member_size * p_fifo->rear);   /* +ƫ����. */
        return STATUS_SUCCESS;
    }
}

/**
 * \brief   ��fifoβ�����������֮��,����βָ��
 *
 * \param   p_fifo[in,out]  Ҫ������fifo
 */
void fifo_append(fifo_t *p_fifo) {
    uint32_t rear_temp = p_fifo->rear;
    if ((++rear_temp) == p_fifo->size) {
        rear_temp = 0;
    }
    p_fifo->is_empty = 0;
    p_fifo->rear = rear_temp;
}

/**
 * \brief   ��ȡfifoͷָ��
 *
 * \param   p_fifo[in]              Ҫ������fifo
 * \param   pp_data_pointer[out]    fifoͷָ��
 *
 * \retval  STATUS_SUCCESS  �ɹ���ȡָ��
 *          STATUS_ERROR    ��ȡfifoβָ��ʧ��,fifo��
 */
status_t fifo_get_front_data(fifo_t *p_fifo, void **pp_data_pointer) {
    if (p_fifo->is_empty == 1) {
        return STATUS_ERROR;
    } else {
        *pp_data_pointer = p_fifo->base
                + (p_fifo->member_size * p_fifo->front);
        return STATUS_SUCCESS;
    }
}

/**
 * \brief   �ͷŻ�����ͷ��Ա
 *
 * \details ��ȡfifoͷ��Ա�����ͷ�
 *
 * \param   p_fifo[in,out]      Ҫ������fifo
 */
void fifo_release(fifo_t *p_fifo) {
    uint32_t front_temp = p_fifo->front;
    if ((++front_temp) == p_fifo->size) {
        front_temp = 0;
    }
    if (p_fifo->rear == front_temp) {
        p_fifo->is_empty = 1;
    }
    p_fifo->front = front_temp;
}

/**
 * \brief   ��ȡ�������е�ǰ��Ա����
 *
 * \param   p_fifo[in]  Ҫ�����Ļ�����
 *
 * \retval  �������е�ǰ��Ա����
 */
uint32_t fifo_get_element_count(fifo_t *p_fifo) {
//    uint32_t count = (fifo->size + fifo->rear - fifo->front) % fifo->size;
//    if (count == 0) {
//        if (fifo->is_empty == 1) {
//            return 0;
//        } else {
//            return fifo->size;
//        }
//    } else {
//        return count;
//    }
    if (p_fifo->front < p_fifo->rear) {
        return p_fifo->rear - p_fifo->front;
    } else if (p_fifo->front == p_fifo->rear) {
        if (p_fifo->is_empty == 1) {
            return 0;
        } else {
            return p_fifo->size;
        }
    } else {
        return p_fifo->size + p_fifo->rear - p_fifo->front;
    }
}

/**
 * \brief   ��ȡ�������ܻ��������Ա��
 *
 * \details �������ڴ��С = һ����Ա�Ĵ�С x ��Ա�������
 *
 * \note    �����ڴ�����
 */
uint32_t fifo_get_size(fifo_t *p_fifo) {
    return p_fifo->size;
}

/**
 * \brief   ��ȡ�����е�����Ա���ڴ����ô�С
 */
uint32_t fifo_get_memory_size(fifo_t *p_fifo) {
    return p_fifo->size * p_fifo->member_size;
}
