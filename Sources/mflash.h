/*
 * mflash.h
 *
 *  Created on: 2018��4��12��
 *      Author: Administrator
 */

#ifndef MFLASH_H_
#define MFLASH_H_
#include "S32K144.h"
#include "flash1.h"
#include "clockMan1.h"

#define FLASH_TARGET

extern flash_ssd_config_t g_flashSSDConfig;

/**
 * \brief   д��pflashǰ�ĳ�ʼ������
 */
status_t flash_pflash_init(void);

/**
 * \brief   ��DFlash��ʼ��ΪEEPROM
 */
status_t flash_EEPROM_init(void);

/*
 * \brief   ����PFlash����
 * \param   sector_index    ����ָ��,���ӵڼ���������ʼ����,
 *                          ȡֵ��Χ0~127(4kByteһ������,��512kByte)
 * \param   sector_num      ��������������
 * \note    sector_index + sector_num <= 127
 */
status_t flash_pflash_erase_sectors(uint32_t sector_index, uint32_t sector_num);

/*
 * \brief   ��PFlashд������
 * \param   address             д�����ݵĵ�ַ,������8�ı���
 * \param   size                д�����ݵ��ֽ���
 * \param   sourceBuffer[in]    Ҫд�������
 * \param   failAddr[out]       ����д������ʧ�ܵĵ�ַ
 * \retval
 */
status_t flash_write_PFLASH(uint32_t address, uint32_t size, uint8_t *sourceBuffer, uint32_t *failAddr);

/**
 * \brief   ��EEPROM��д������
 *
 * \param   offset          д�����ݵ�ַ��EEPROM�׵�ַ��ƫ����
 * \param   sourceBuffer    Ҫд�������
 * \param   len             д�����ݵĳ���
 *
 * \retval  STATUS_SUCCESS  д��ɹ�
 *          STATUS_ERROR    д��ʧ��
 */
status_t flash_write_EEPROM(uint32_t index, uint8_t *sourceBuffer, uint32_t len);

/**
 * \brief   FLASH�����ɹ��жϷ�����
 */
void CCIF_Handler(void);

START_FUNCTION_DECLARATION_RAMSECTION
/**
 * \brief   д��flashǰ�Ļص�����
 * \details ����FALSHд������ǰ,���ȵ��øú���
 */
void CCIF_Callback(void)
END_FUNCTION_DECLARATION_RAMSECTION

#endif /* MFLASH_H_ */
