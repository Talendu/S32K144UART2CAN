/*
 * mfalsh.c
 *
 *  Created on: 2018年4月12日
 *      Author: Administrator
 */
#include "mflash.h"
  /* Declare a FLASH config struct which initialized by FlashInit, and will be used by all flash operations */
flash_ssd_config_t flashSSDConfig;

status_t flash_pflash_init(void) {
    status_t ret;


#ifdef S32K144_SERIES
    MSCM->OCMDR[0u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu) | MSCM_OCMDR_OCM2(0xFu);
#endif/* S32K144_SERIES */

    /* Install interrupt for Flash Command Complete event */
    INT_SYS_InstallHandler(FTFC_IRQn, CCIF_Handler, (isr_t*) 0);
    INT_SYS_EnableIRQ(FTFC_IRQn);

    /* Enable global interrupt */
    INT_SYS_EnableIRQGlobal();

    /* Always initialize the driver before calling other functions */
    ret = FLASH_DRV_Init(&flash1_InitConfig0, &flashSSDConfig);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

    /* Set callback function before a long time consuming flash operation
     * (ex: erasing) to let the application code do other tasks while flash
     * in operation. In this case we use it to enable interrupt for
     * Flash Command Complete event */
    flashSSDConfig.CallBack = (flash_callback_t)CCIF_Callback;

    return STATUS_SUCCESS;
}

/*
 * \brief   将DFlash初始化为EEPROM
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
    MSCM->OCMDR[1u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu) | MSCM_OCMDR_OCM2(0xFu);
    MSCM->OCMDR[2u] |= MSCM_OCMDR_OCM0(0xFu) | MSCM_OCMDR_OCM1(0xFu) | MSCM_OCMDR_OCM2(0xFu);
#endif /* S32K144_SERIES */

    /* Install interrupt for Flash Command Complete event */
    INT_SYS_InstallHandler(FTFC_IRQn, CCIF_Handler, (isr_t*) 0);
    INT_SYS_EnableIRQ(FTFC_IRQn);

    /* Enable global interrupt */
    INT_SYS_EnableIRQGlobal();

    /* Always initialize the driver before calling other functions */
    ret = FLASH_DRV_Init(&flash1_InitConfig0, &flashSSDConfig);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

#if ((FEATURE_FLS_HAS_FLEX_NVM == 1u) & (FEATURE_FLS_HAS_FLEX_RAM == 1u))
    /* Config FlexRAM as EEPROM if it is currently used as traditional RAM */
    if (flashSSDConfig.EEESize == 0u)
    {
#ifndef FLASH_TARGET
        /* First, erase all Flash blocks if code is placed in RAM to ensure
         * the IFR region is blank before partitioning FLexNVM and FlexRAM */
        uint32_t address;
        uint32_t size;
        ret = FLASH_DRV_EraseAllBlock(&flashSSDConfig);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }

        /* Verify the erase operation at margin level value of 1 */
        ret = FLASH_DRV_VerifyAllBlock(&flashSSDConfig, 1u);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }

        /* Reprogram secure byte in Flash configuration field */
#if (FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD == 1u)
        address = 0x408u;
        size = FTFx_PHRASE_SIZE;
#else   /* FEATURE_FLASH_HAS_PROGRAM_LONGWORD_CMD == 1u */
        address = 0x40Cu;
        size = FTFx_LONGWORD_SIZE;
#endif /* FEATURE_FLS_HAS_PROGRAM_PHRASE_CMD */
        ret = FLASH_DRV_Program(&flashSSDConfig, address, size, unsecure_key);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }
#endif /* FLASH_TARGET */

        /* Configure FlexRAM as EEPROM and FlexNVM as EEPROM backup region,
         * DEFlashPartition will be failed if the IFR region isn't blank.
         * Refer to the device document for valid EEPROM Data Size Code
         * and FlexNVM Partition Code. For example on S32K144:
         * - EEEDataSizeCode = 0x02u: EEPROM size = 4 Kbytes
         * - DEPartitionCode = 0x08u: EEPROM backup size = 64 Kbytes */
        ret = FLASH_DRV_DEFlashPartition(&flashSSDConfig, 0x02u, 0x08u, 0x0u, false);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }
        else
        {
            /* Re-initialize the driver to update the new EEPROM configuration */
            ret = FLASH_DRV_Init(&flash1_InitConfig0, &flashSSDConfig);
            if (ret != STATUS_SUCCESS)
            {
                return ret;
            }

            /* Make FlexRAM available for EEPROM */
            ret = FLASH_DRV_SetFlexRamFunction(&flashSSDConfig, EEE_ENABLE, 0x00u, NULL);
            if (ret != STATUS_SUCCESS)
            {
                return ret;
            }
        }
    }
    else    /* FLexRAM is already configured as EEPROM */
    {
        /* Make FlexRAM available for EEPROM, make sure that FlexNVM and FlexRAM
         * are already partitioned successfully before */
        ret = FLASH_DRV_SetFlexRamFunction(&flashSSDConfig, EEE_ENABLE, 0x00u, NULL);
        if (ret != STATUS_SUCCESS)
        {
            return ret;
        }
    }
#endif /* (FEATURE_FLS_HAS_FLEX_NVM == 1u) & (FEATURE_FLS_HAS_FLEX_RAM == 1u) */

    /* Set callback function before a long time consuming flash operation
     * (ex: erasing) to let the application code do other tasks while flash
     * in operation. In this case we use it to enable interrupt for
     * Flash Command Complete event */

    flashSSDConfig.CallBack = (flash_callback_t)CCIF_Callback;
    return STATUS_SUCCESS;
}

/*
 * \brief   擦出PFlash扇区
 * \param   sector_index    扇区指标,及从第几个扇区开始擦出,
 *                          取值范围0~127(4kByte一个扇区,共512kByte)
 * \param   sector_num      擦出的扇区个数
 * \note    sector_index + sector_num <= 127
 */
status_t flash_pflash_erase_sectors(uint32_t sector_index,
        uint32_t sector_num) {
    status_t ret;
    uint32_t dest;
    uint32_t size;

    dest = sector_index << 12;
    size = sector_num << 12;
    ret = FLASH_DRV_EraseSector(&flashSSDConfig, dest, size);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

    /* Disable Callback */
    flashSSDConfig.CallBack = NULL_CALLBACK;

    /* Verify the erase operation at margin level value of 1, user read */
    ret = FLASH_DRV_VerifySection(&flashSSDConfig, dest, size / FTFx_DPHRASE_SIZE, 1u);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

    return STATUS_SUCCESS;
}

/*
 * \brief   向PFlash写入数据
 * \param   address 写入数据的地址,必须是8的倍数
 * \param   size    写入数据的字节数
 * \param   sourceBuffer    要写入的数据
 * \param   failAddr[out]   返回写入数据的首地址
 * \retval
 */
status_t flash_write_PFLASH(uint32_t address, uint32_t size, uint8_t *sourceBuffer, uint32_t *failAddr) {

    /* Erase the sixth PFlash sector */
    status_t ret;
//    address = 6u * FEATURE_FLS_PF_BLOCK_SECTOR_SIZE;
//    ret = flash_pflash_erase_and_verify_sector(&flashSSDConfig, address, size);

    /* Write some data to the erased PFlash sector */
//    size = BUFFER_SIZE;
    ret = FLASH_DRV_Program(&flashSSDConfig, address, size, sourceBuffer);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }

    /* Verify the program operation at margin level value of 1, user margin */
    ret = FLASH_DRV_ProgramCheck(&flashSSDConfig, address, size, sourceBuffer, failAddr, 1u);
    if (ret != STATUS_SUCCESS)
    {
        return ret;
    }
    return STATUS_SUCCESS;
}


status_t flash_write_EEPROM(uint32_t index, uint8_t *sourceBuffer, uint32_t len) {
    status_t ret;
    /* Try to write data to EEPROM if FlexRAM is configured as EEPROM */
     if (flashSSDConfig.EEESize != 0u)
     {
         uint32_t address;
         address = flashSSDConfig.EERAMBase + index;
         ret = FLASH_DRV_EEEWrite(&flashSSDConfig, address, len, sourceBuffer);
         if (ret != STATUS_SUCCESS)
         {
             return ret;
         }

         /* Verify the written data */
         if (*((uint32_t *)sourceBuffer) != *((uint32_t *)address))
         {
             /* Failed to write data to EEPROM */
             return STATUS_ERROR;
         }
     }
     return STATUS_SUCCESS;
}

void CCIF_Handler(void)
{
  /* Disable Flash Command Complete interrupt */
  FTFx_FCNFG &= (~FTFx_FCNFG_CCIE_MASK);

  return;
}

/*!
\brief Callback function for Flash operations
*/
START_FUNCTION_DEFINITION_RAMSECTION
void CCIF_Callback(void)
{
  /* Enable interrupt for Flash Command Complete */
  if ((FTFx_FCNFG & FTFx_FCNFG_CCIE_MASK) == 0u)
  {
      FTFx_FCNFG |= FTFx_FCNFG_CCIE_MASK;
  }
}
END_FUNCTION_DEFINITION_RAMSECTION
