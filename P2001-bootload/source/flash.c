#include "flash.h"
#include "hc32f46x_efm.h"
#include "gpio.h"
#include "hc32f46x_gpio.h"




void FLASH_vWriteBytes(uint32_t ulTargetAddr, uint8_t* pucData, uint32_t ulDataLen)
{
    uint32_t ulWordNum = 0;
    uint32_t ulWord = 0;

    ulWordNum = ulDataLen / 4u;     //!< word

    /* Unlock EFM. */
    EFM_Unlock();

    /* Enable flash. */
    EFM_FlashCmd(Enable);
    /* Wait flash ready. */
    while (Set != EFM_GetFlagStatus(EFM_FLAG_RDY))
    {
        ;
    }

    while (ulWordNum--)
    {
        ulWord = *(pucData++);
        ulWord |= *(pucData++) << 8;
        ulWord |= *(pucData++) << 16;
        ulWord |= *(pucData++) << 24;

        if (EFM_SingleProgram(ulTargetAddr, ulWord) != Ok)
        {
            PORT_Toggle(LED1_PORT, LED1_PIN); //!< red
        }

        ulTargetAddr += 4u;
    }

    /* Lock EFM. */
    EFM_Lock();
}

void FLASH_vEraseSector(uint32_t ulSectorAddr, uint32_t ulSize)
{
    uint32_t ulSectorNum = 0;

    if ((ulSize % FLASH_SECTOR_SIZE) == 0)
    {
        ulSectorNum = ulSize / FLASH_SECTOR_SIZE;
    }
    else
    {
        ulSectorNum = ulSize / FLASH_SECTOR_SIZE + 1;
    }

    /* Unlock EFM. */
    EFM_Unlock();

    /* Enable flash. */
    EFM_FlashCmd(Enable);
    /* Wait flash ready. */
    while (Set != EFM_GetFlagStatus(EFM_FLAG_RDY))
    {
        ;
    }

    while (ulSectorNum--)
    {
        /* Erase sector  */
        EFM_SectorErase(ulSectorAddr);

        ulSectorAddr += FLASH_SECTOR_SIZE;
    }

    /* Lock EFM. */
    EFM_Lock();
}