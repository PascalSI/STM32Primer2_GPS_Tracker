/********************************************************************************/
/*!
	@file			usb_msc_mass_mal.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2016.01.15
	@brief          Descriptor Header for Mal.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.03.21	V2.00	Adopted FatFs10.0a
		2016.01.15	V3.00	Changed definition compatibility.
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#include "sdio_stm32f1.h"
#include "usb_msc_mass_mal.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint64_t Mass_Memory_Size[MAX_LUN];
uint32_t Mass_Block_Size[MAX_LUN];
uint32_t Mass_Block_Count[MAX_LUN];

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief  Initializes the Media on the STM32.
*/
/**************************************************************************/
uint16_t MAL_Init(uint8_t lun)
{
  uint16_t status = MAL_OK;

  switch (lun)
  {
    case LUN_SDCARD:
      Status = SD_Init();
      break;
#ifdef USE_STM3210E_EVAL
    case 1:
      NAND_Init();
      break;
#endif
    default:
      return MAL_FAIL;
  }
  return status;
}

/**************************************************************************/
/*! 
    @brief  Write sectors.
*/
/**************************************************************************/
uint16_t MAL_Write(uint8_t lun, uint64_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{

  switch (lun)
  {
    case LUN_SDCARD:
      Status = SD_WriteBlock((uint8_t*)Writebuff, Memory_Offset, Transfer_Length);
      if ( Status != SD_OK )
      {
        return MAL_FAIL;
      } 
      break;
	
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/**************************************************************************/
/*! 
    @brief  Read sectors.
*/
/**************************************************************************/
uint16_t MAL_Read(uint8_t lun, uint64_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{

  switch (lun)
  {
    case LUN_SDCARD:
	
      Status = SD_ReadBlock((uint8_t*)Readbuff, Memory_Offset, Transfer_Length);
      if ( Status != SD_OK )
      {
        return MAL_FAIL;
      }
      break;
	
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/**************************************************************************/
/*! 
    @brief  Get status.
*/
/**************************************************************************/
uint16_t MAL_GetStatus (uint8_t lun)
{

  uint32_t DeviceSizeMul = 0, NumberOfBlocks = 0;
  uint64_t dwDevSize;  /* nemui */

  if (lun == LUN_SDCARD)
  {
    if (SD_Init() == SD_OK)
    {
      SD_GetCardInfo(&SDCardInfo);
      SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
      DeviceSizeMul = (SDCardInfo.SD_csd.DeviceSizeMul + 2);

      if(SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
      {
		/* nemui */
		dwDevSize  = (uint64_t)(SDCardInfo.SD_csd.DeviceSize + 1) * 512 * 1024;
		/* nemui  calculate highest LBA */
		Mass_Block_Count[LUN_SDCARD] = (dwDevSize - 1) / 512;
      }
      else
      {
        NumberOfBlocks  = ((1 << (SDCardInfo.SD_csd.RdBlockLen)) / 512);
        Mass_Block_Count[LUN_SDCARD] = ((SDCardInfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks/2));
      }
      Mass_Block_Size[LUN_SDCARD]  = 512;

      Status = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16)); 
      Status = SD_EnableWideBusOperation(SDIO_BusWide_4b); 
      if ( Status != SD_OK )
      {
        return MAL_FAIL;
      }

      Mass_Memory_Size[LUN_SDCARD] = (uint64_t)Mass_Block_Count[LUN_SDCARD] * Mass_Block_Size[LUN_SDCARD];

      return MAL_OK;

    }
  }
  return MAL_FAIL;
}

/* End Of File ---------------------------------------------------------------*/
