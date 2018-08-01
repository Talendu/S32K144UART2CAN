/*
 * fifo.c
 *
 *  Created on: 2018年4月7日
 *      Author: Administrator
 */
#include "fifo.h"

/**
 * \brief   初始化fifo
 *
 * \param   p_fifo[in,out]      要初始化的fifo
 * \param   p_base[in]          fifo对应的内存块
 * \param   max_member_number   缓冲区最大成员容量
 *                              (缓冲区内存大小 = 一个成员的大小 x 成员最大容量)
 * \param   menber_size         缓冲区中一个成员占用的内存空间
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
 * \brief   获取fifo尾指针
 *
 * \param   p_fifo[in]              要操作的fifo
 * \param   pp_rear_pointer[out]    fifo尾指针
 *
 * \retval  STATUS_SUCCESS  成功获取指针
 *          STATUS_ERROR    获取fifo尾指针失败,fifo满
 */
status_t fifo_get_rear_pointer(fifo_t *p_fifo, void **pp_rear_pointer) {
    if (p_fifo->rear == p_fifo->front       /* 队列头尾在一起,要么是空,要么是满. */
            && p_fifo->is_empty == 0){    /* 如果队列不是空. */
        *pp_rear_pointer = NULL;           /* 不能获取头指针; */
        return STATUS_ERROR;
    } else {                            /* 否则可以获取. */
        *pp_rear_pointer = p_fifo->base      /* fifo内存首地址 */
                + (p_fifo->member_size * p_fifo->rear);   /* +偏移量. */
        return STATUS_SUCCESS;
    }
}

/**
 * \brief   在fifo尾部添加完数据之后,更新尾指针
 *
 * \param   p_fifo[in,out]  要操作的fifo
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
 * \brief   获取fifo头指针
 *
 * \param   p_fifo[in]              要操作的fifo
 * \param   pp_data_pointer[out]    fifo头指针
 *
 * \retval  STATUS_SUCCESS  成功获取指针
 *          STATUS_ERROR    获取fifo尾指针失败,fifo满
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
 * \brief   释放缓冲区头成员
 *
 * \details 读取fifo头成员后将其释放
 *
 * \param   p_fifo[in,out]      要操作的fifo
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
 * \brief   获取缓冲区中当前成员数量
 *
 * \param   p_fifo[in]  要操作的缓冲区
 *
 * \retval  缓冲区中当前成员数量
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
 * \brief   获取缓冲区能缓存的最大成员数
 *
 * \details 缓冲区内存大小 = 一个成员的大小 x 成员最大容量
 *
 * \note    不是内存容量
 */
uint32_t fifo_get_size(fifo_t *p_fifo) {
    return p_fifo->size;
}

/**
 * \brief   获取缓冲中单个成员的内存所用大小
 */
uint32_t fifo_get_memory_size(fifo_t *p_fifo) {
    return p_fifo->size * p_fifo->member_size;
}
