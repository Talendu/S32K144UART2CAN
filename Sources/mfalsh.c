/*
 * mfalsh.c
 *
 *  Created on: 2018��4��12��
 *      Author: Administrator
 */
#include "mflash.h"
flash_ssd_config_t g_flashSSDConfig;

/**
 * \brief   д��pflashǰ�ĳ�ʼ������
 */
status_t flash_pflash_init(void)
{
    status_t ret;

#ifdef S32K144_SERIES
    /* ���û��� */
    MSCM->OCMDR[0u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu) | MSCM_OCMDR_OCM2(0xFu);
#endif/* S32K144_SERIES */

    /* �����жϷ����� */
    INT_SYS_InstallHandler(FTFC_IRQn, CCIF_Handler, (isr_t*) 0);
    INT_SYS_EnableIRQ(FTFC_IRQn);

    /* ʹ��ȫ���ж� */
    INT_SYS_EnableIRQGlobal();

    /* ��ʼ��FLASH */
    ret = FLASH_DRV_Init(&flash1_InitConfig0, &g_flashSSDConfig);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

    /* ����flashд����ɻص����� */
    g_flashSSDConfig.CallBack = (flash_callback_t)CCIF_Callback;

    return STATUS_SUCCESS;
}

/**
 * \brief   ��DFlash��ʼ��ΪEEPROM
 */
status_t flash_EEPROM_init(void) {
    /* Disable cache to ensure that all flash operations will take effect instantly,
     * this is device dependent */
    status_t ret;

#ifndef FLASH_TARGET
#if (FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD == 1u)
    uint8_t unsecure_key[FTFx_PHRASE_SIZE] = {0xFFu, 0xFFu, 0xFFu, 0xFFu, 0xFEu, 0xFFu, 0xFFu, 0xFFu};
#else   /* FEATURE_FLASH_HAS_PROGRAM_LONGWORD_CMD */
    uint8_t unsecure_key[FTFx_LONGWORD_SIZE] = {0xFEu, 0xFFu, 0xFFu, 0xFFu};
#endif  /* FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD */
#endif /* FLASH_TARGET */

#ifdef S32K144_SERIES
    /* ���û��� */
    MSCM->OCMDR[1u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu) | MSCM_OCMDR_OCM2(0xFu);
    MSCM->OCMDR[2u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu) | MSCM_OCMDR_OCM2(0xFu);
#endif /* S32K144_SERIES */

    /* �����жϷ����� */
    INT_SYS_InstallHandler(FTFC_IRQn, CCIF_Handler, (isr_t*) 0);
    INT_SYS_EnableIRQ(FTFC_IRQn);

    /* ʹ��ȫ���ж� */
    INT_SYS_EnableIRQGlobal();

    /* ��ʼ��FLASH */
    ret = FLASH_DRV_Init(&flash1_InitConfig0, &g_flashSSDConfig);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

#if ((FEATURE_FLS_HAS_FLEX_NVM == 1u) & (FEATURE_FLS_HAS_FLEX_RAM == 1u))
    /* ���flexRamû����ΪEEPROMʹ��,��������ΪEEPROM */
    if (g_flashSSDConfig.EEESize == 0u)
    {
#ifndef FLASH_TARGET
        /* �������� */
        uint32_t address;
        uint32_t size;
        ret = FLASH_DRV_EraseAllBlock(&g_flashSSDConfig);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }

        /* У���Ƿ�����ɹ� */
        ret = FLASH_DRV_VerifyAllBlock(&g_flashSSDConfig, 1u);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }

        /* ��ȫ�����ֶ�Flash��� */
#if (FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD == 1u)
        address = 0x408u;
        size = FTFx_PHRASE_SIZE;
#else   /* FEATURE_FLASH_HAS_PROGRAM_LONGWORD_CMD == 1u */
        address = 0x40Cu;
        size = FTFx_LONGWORD_SIZE;
#endif /* FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD */
        ret = FLASH_DRV_Program(&g_flashSSDConfig, address, size, unsecure_key);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }
#endif /* FLASH_TARGET */

        /* ��FlexRAM����ΪEEPROM,����FlexNVM��ΪEEPROM�Ĵ洢��,
         * ���IFR�����ǿհ׽����ܳɹ�DEFlashPartition.
         * ������Ч��EEPROM���ݴ�С�Ĵ���ʹ���flexnvm�����豸�ļ�
         * S32K144:
         * - EEEDataSizeCode = 0x02u: EEPROM size = 4 Kbytes
         * - DEPartitionCode = 0x08u: EEPROM backup size = 64 Kbytes
         */
        ret = FLASH_DRV_DEFlashPartition(&g_flashSSDConfig, 0x02u, 0x08u, 0x0u, false);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }
        else
        {
            /* ���³�ʼ��FLASH */
            ret = FLASH_DRV_Init(&flash1_InitConfig0, &g_flashSSDConfig);
            if (ret != STATUS_SUCCESS)
            {
                return ret;
            }

            /* ʹ��FlaxRAMΪEEPROM */
            ret = FLASH_DRV_SetFlexRamFunction(&g_flashSSDConfig, EEE_ENABLE, 0x00u, NULL);
            if (ret != STATUS_SUCCESS)
            {
                return ret;
            }
        }
    }
    else    /* FLexRAM�Ѿ�����ʼ��ΪEEPROM */
    {
        /* ʹ��FlaxRAMΪEEPROM */
        ret = FLASH_DRV_SetFlexRamFunction(&g_flashSSDConfig, EEE_ENABLE, 0x00u, NULL);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }
    }
#endif /* (FEATURE_FLS_HAS_FLEX_NVM == 1u) & (FEATURE_FLS_HAS_FLEX_RAM == 1u) */

    /* ����flashд����ɻص����� */
    g_flashSSDConfig.CallBack = (flash_callback_t)CCIF_Callback;
    return STATUS_SUCCESS;
}

/*
 * \brief   ����PFlash����
 * \param   sector_index    ����ָ��,���ӵڼ���������ʼ����,
 *                          ȡֵ��Χ0~127(4kByteһ������,��512kByte)
 * \param   sector_num      ��������������
 * \note    sector_index + sector_num <= 127
 */
status_t flash_pflash_erase_sectors(uint32_t sector_index,
        uint32_t sector_num) {
    status_t ret;
    uint32_t dest;
    uint32_t size;

    dest = sector_index << 12;
    size = sector_num << 12;
    ret = FLASH_DRV_EraseSector(&g_flashSSDConfig, dest, size);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

    /* Disable Callback */
    g_flashSSDConfig.CallBack = NULL_CALLBACK;

    /* Verify the erase operation at margin level value of 1, user read */
    ret = FLASH_DRV_VerifySection(&g_flashSSDConfig, dest, size / FTFx_DPHRASE_SIZE, 1u);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

    return STATUS_SUCCESS;
}

/*
 * \brief   ��PFlashд������
 * \param   address             д�����ݵĵ�ַ,������8�ı���
 * \param   size                д�����ݵ��ֽ���
 * \param   sourceBuffer[in]    Ҫд�������
 * \param   failAddr[out]       ����д������ʧ�ܵĵ�ַ
 * \retval
 */
status_t flash_write_PFLASH(uint32_t    address,
                            uint32_t    size,
                            uint8_t    *p_sourceBuffer,
                            uint32_t   *p_failAddr)
{
    status_t ret;
    /* ��FLASH��д������ */
    ret = FLASH_DRV_Program(&g_flashSSDConfig, address, size, p_sourceBuffer);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

    /* У��д������� */
    ret = FLASH_DRV_ProgramCheck(&g_flashSSDConfig, address, size,
            p_sourceBuffer, p_failAddr, 1u);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }
    return STATUS_SUCCESS;
}

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
status_t flash_write_EEPROM(uint32_t    offset,
                            uint8_t    *sourceBuffer,
                            uint32_t    len)
{
    status_t ret;
    /* ��EEPROM��д������ */
     if (g_flashSSDConfig.EEESize != 0u)
     {
         uint32_t address;
         address = g_flashSSDConfig.EERAMBase + offset;
         ret = FLASH_DRV_EEEWrite(&g_flashSSDConfig, address, len, sourceBuffer);
         if (ret != STATUS_SUCCESS)
         {
             return ret;
         }

         /* У��д������� */
         if (*((uint32_t *)sourceBuffer) != *((uint32_t *)address))
         {
             return STATUS_ERROR;
         }
     }
     return STATUS_SUCCESS;
}

/**
 * \brief   FLASH�����ɹ��жϷ�����
 */
void CCIF_Handler(void)
{
  /* �ر�flashд������ж� */
  FTFx_FCNFG &= (~FTFx_FCNFG_CCIE_MASK);

  return;
}

/**
 * \brief   д��flashǰ�Ļص�����
 * \details ����FALSHд������ǰ,���ȵ��øú���
 */
START_FUNCTION_DEFINITION_RAMSECTION
void CCIF_Callback(void)
{
  /* ʹ��FLASHд������ж� */
  if ((FTFx_FCNFG & FTFx_FCNFG_CCIE_MASK) == 0u)
  {
      FTFx_FCNFG |= FTFx_FCNFG_CCIE_MASK;
  }
}
END_FUNCTION_DEFINITION_RAMSECTION
