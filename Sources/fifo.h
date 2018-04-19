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

/*
 * \brief   fifo描述结构体
 * \member  front   fifo头
 *          rear    fifo尾
 *          size    fifo大小,注意这里的size不是字节数,而是fifo的长度
 *          base    内存块指针,需要在其他地方分配内存空间
 *          member_size fifo中每个成员的大小
 */
typedef struct {
	uint32_t front;
	uint32_t rear;
	uint32_t size;
	uint8_t *base;
	uint8_t member_size;
	uint8_t is_empty;
}fifo_t;

void fifo_init(fifo_t *fifo, void* base,uint32_t max_member_number, uint32_t menber_size);

status_t fifo_get_rear_pointer(fifo_t *fifo, void **rear_pointer);
void fifo_append(fifo_t *fifo);

status_t fifo_get_front_data(fifo_t *fifo, void **data_pointer);
void fifo_release(fifo_t *fifo);

uint32_t fifo_get_element_count(fifo_t *fifo);
uint32_t fifo_get_size(fifo_t *fifo);
uint32_t fifo_get_memory_size(fifo_t *fifo);

#endif /* FIFO_H_ */
