/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : flash1.c
**     Project     : CAN2UART
**     Processor   : S32K144_100
**     Component   : flash
**     Version     : Component 1.0.0, Driver 01.00, CPU db: 3.00.000
**     Repository  : SDK_S32K144_04
**     Compiler    : GNU C Compiler
**     Date/Time   : 2018-04-18, 17:29, # CodeGen: 11
**
**     Copyright : 1997 - 2015 Freescale Semiconductor, Inc.
**     Copyright 2016-2017 NXP
**     All Rights Reserved.
**     
**     THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
**     IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
**     OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
**     IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
**     INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
**     SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
**     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
**     STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
**     IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
**     THE POSSIBILITY OF SUCH DAMAGE.
** ###################################################################*/
/*!
** @file flash1.c
** @version 01.00
*/
/*!
**  @addtogroup flash1_module flash1 module documentation
**  @{
*/
/* Module flash1.
 *
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External variable could be made static.
 * The external variable will be used in other source files in application code.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.1, Conversion between a pointer
 * to function and another type.
 * The cast is required to define a callback function.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, A cast shall not be performed
 * between pointer to void and an arithmetic type.
 * The cast is required to initialize a pointer with an unsigned int define,
 * representing an address.
 */

#include "flash1.h"
/*! @brief Configuration structure flashCfg_0 */
const flash_user_config_t flash1_InitConfig0 = {
    .PFlashBase  = 0x00000000U,
    .PFlashSize  = 0x00080000U,
    .DFlashBase  = 0x10000000U,
    .EERAMBase   = 0x14000000U,
    /* If using callback, any code reachable from this function must not be placed in a Flash block targeted for a program/erase operation.*/
    .CallBack    = NULL_CALLBACK
};

/* END flash1. */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the Freescale S32K series of microcontrollers.
**
** ###################################################################
*/
