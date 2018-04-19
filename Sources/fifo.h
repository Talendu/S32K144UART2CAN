/*
 * fifo.h
 *
 *  Created on: 2018��4��7��
 *      Author: Administrator
 */

#ifndef FIFO_H_
#define FIFO_H_
#include "S32K144.h"
#include "status.h"
#include "null.h"

/*
 * \brief   fifo�����ṹ��
 * \member  front   fifoͷ
 *          rear    fifoβ
 *          size    fifo��С,ע�������size�����ֽ���,����fifo�ĳ���
 *          base    �ڴ��ָ��,��Ҫ�������ط������ڴ�ռ�
 *          member_size fifo��ÿ����Ա�Ĵ�С
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
