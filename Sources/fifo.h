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

/**
 * \brief   fifo�����ṹ��
 *
 * ���λ����������ṹ��,
 * �û����������ڻ������Խṹ������.
 *
 * \note    ��������ʼ��ʱҪָ�������������ڴ��;
 *          ��������Ա��С��ָ�������ݵ������ݵĴ�С,
 *          ����:���Ҫ�����������uint8_t���͵�,��ômember_size = 1
 *          �������Ĵ�С����ָ���������ڴ��С,���ǻ������ܻ���ĳ�Ա��.
 *          �ڳ�ʼ��������ʱһ��Ҫ��֤: baseָ�����ڴ���С >= member_size * size
 *
 */
typedef struct {
	uint32_t front;         /**< \brief fifoͷ */
	uint32_t rear;          /**< \brief fifoβ*/
	uint32_t size;          /**< \brief fifo��С,�����ֽ���,����fifo�ĳ��� */
	uint8_t *base;          /**< \brief �ڴ��ָ��,��Ҫ�������ط������ڴ�ռ� */
	uint8_t member_size;    /**< \brief fifo��ÿ����Ա�Ĵ�С */
	uint8_t is_empty;       /**< \brief fifo�Ƿ�Ϊ�� */
}fifo_t;

/**
 * \brief   ��ʼ��fifo
 *
 * \param   p_fifo[in,out]      Ҫ��ʼ����fifo
 * \param   p_base[in]          fifo��Ӧ���ڴ��
 * \param   max_member_number   ����������Ա����
 *                              (�������ڴ��С = һ����Ա�Ĵ�С x ��Ա�������)
 * \param   menber_size         ��������һ����Առ�õ��ڴ�ռ�
 *
 * \note    ���뱣֤: p_baseָ����ڴ��Ĵ�С >= max_member_number * menber_size
 */
void fifo_init(fifo_t   *p_fifo,
               void     *p_base,
               uint32_t  max_member_number,
               uint32_t  menber_size);

/**
 * \brief   ��ȡfifoβָ��
 *
 * \param   p_fifo[in]              Ҫ������fifo
 * \param   pp_rear_pointer[out]    fifoβָ��
 *
 * \retval  STATUS_SUCCESS  �ɹ���ȡָ��
 *          STATUS_ERROR    ��ȡfifoβָ��ʧ��,fifo��
 */
status_t fifo_get_rear_pointer(fifo_t *p_fifo, void **pp_rear_pointer);

/**
 * \brief   ��fifoβ�����������֮��,����βָ��
 *
 * \param   p_fifo[in,out]  Ҫ������fifo
 */
void fifo_append(fifo_t *p_fifo);


/**
 * \brief   ��ȡfifoͷָ��
 *
 * \param   p_fifo[in]              Ҫ������fifo
 * \param   pp_data_pointer[out]    fifoͷָ��
 *
 * \retval  STATUS_SUCCESS  �ɹ���ȡָ��
 *          STATUS_ERROR    ��ȡfifoβָ��ʧ��,fifo��
 */
status_t fifo_get_front_data(fifo_t *p_fifo, void **data_pointer);

/**
 * \brief   �ͷŻ�����ͷ��Ա
 *
 * \details ��ȡfifoͷ��Ա�����ͷ�
 *
 * \param   p_fifo[in,out]  Ҫ������fifo
 */
void fifo_release(fifo_t *p_fifo);

/**
 * \brief   ��ȡ�������е�ǰ��Ա����
 *
 * \param   p_fifo[in]  Ҫ�����Ļ�����
 *
 * \retval  �������е�ǰ��Ա����
 */
uint32_t fifo_get_element_count(fifo_t *p_fifo);

/**
 * \brief   ��ȡ�������ܻ��������Ա��
 *
 * \details �������ڴ��С = һ����Ա�Ĵ�С x ��Ա�������
 *
 * \note    �����ڴ�����
 */
uint32_t fifo_get_size(fifo_t *p_fifo);

/**
 * \brief   ��ȡ�����е�����Ա���ڴ����ô�С
 */
uint32_t fifo_get_memory_size(fifo_t *p_fifo);

#endif /* FIFO_H_ */
