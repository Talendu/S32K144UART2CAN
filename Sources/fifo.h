/*
 * fifo.h
 *
 *  Created on: 2018年4月7日
 *      Author: Administrator
 */

#ifndef FIFO_H_
#define FIFO_H_
#include "S32K144.h"
#include "status.h"
#include "null.h"

/**
 * \brief   fifo描述结构体
 *
 * 环形缓冲区描述结构体,
 * 该缓冲区可用于缓存线性结构的数据.
 *
 * \note    缓冲区初始化时要指定缓冲区所用内存块;
 *          缓冲区成员大小是指缓存数据单个数据的大小,
 *          例如:如果要缓存的数据是uint8_t类型的,那么member_size = 1
 *          缓冲区的大小不是指缓冲区的内存大小,而是缓冲区能缓存的成员数.
 *          在初始化缓冲区时一定要保证: base指定的内存块大小 >= member_size * size
 *
 */
typedef struct {
	uint32_t front;         /**< \brief fifo头 */
	uint32_t rear;          /**< \brief fifo尾*/
	uint32_t size;          /**< \brief fifo大小,不是字节数,而是fifo的长度 */
	uint8_t *base;          /**< \brief 内存块指针,需要在其他地方分配内存空间 */
	uint8_t member_size;    /**< \brief fifo中每个成员的大小 */
	uint8_t is_empty;       /**< \brief fifo是否为空 */
}fifo_t;

/**
 * \brief   初始化fifo
 *
 * \param   p_fifo[in,out]      要初始化的fifo
 * \param   p_base[in]          fifo对应的内存块
 * \param   max_member_number   缓冲区最大成员容量
 *                              (缓冲区内存大小 = 一个成员的大小 x 成员最大容量)
 * \param   menber_size         缓冲区中一个成员占用的内存空间
 *
 * \note    必须保证: p_base指向的内存块的大小 >= max_member_number * menber_size
 */
void fifo_init(fifo_t   *p_fifo,
               void     *p_base,
               uint32_t  max_member_number,
               uint32_t  menber_size);

/**
 * \brief   获取fifo尾指针
 *
 * \param   p_fifo[in]              要操作的fifo
 * \param   pp_rear_pointer[out]    fifo尾指针
 *
 * \retval  STATUS_SUCCESS  成功获取指针
 *          STATUS_ERROR    获取fifo尾指针失败,fifo满
 */
status_t fifo_get_rear_pointer(fifo_t *p_fifo, void **pp_rear_pointer);

/**
 * \brief   在fifo尾部添加完数据之后,更新尾指针
 *
 * \param   p_fifo[in,out]  要操作的fifo
 */
void fifo_append(fifo_t *p_fifo);


/**
 * \brief   获取fifo头指针
 *
 * \param   p_fifo[in]              要操作的fifo
 * \param   pp_data_pointer[out]    fifo头指针
 *
 * \retval  STATUS_SUCCESS  成功获取指针
 *          STATUS_ERROR    获取fifo尾指针失败,fifo满
 */
status_t fifo_get_front_data(fifo_t *p_fifo, void **data_pointer);

/**
 * \brief   释放缓冲区头成员
 *
 * \details 读取fifo头成员后将其释放
 *
 * \param   p_fifo[in,out]  要操作的fifo
 */
void fifo_release(fifo_t *p_fifo);

/**
 * \brief   获取缓冲区中当前成员数量
 *
 * \param   p_fifo[in]  要操作的缓冲区
 *
 * \retval  缓冲区中当前成员数量
 */
uint32_t fifo_get_element_count(fifo_t *p_fifo);

/**
 * \brief   获取缓冲区能缓存的最大成员数
 *
 * \details 缓冲区内存大小 = 一个成员的大小 x 成员最大容量
 *
 * \note    不是内存容量
 */
uint32_t fifo_get_size(fifo_t *p_fifo);

/**
 * \brief   获取缓冲中单个成员的内存所用大小
 */
uint32_t fifo_get_memory_size(fifo_t *p_fifo);

#endif /* FIFO_H_ */
