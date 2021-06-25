#include "FlashIAP.h"
#include "stm32f1xx.h"

#if (defined(STM32F101x6) || defined(STM32F102x6) || defined(STM32F103x6) || defined(STM32F100xB) || defined(STM32F101xB) || defined(STM32F102xB) || defined(STM32F103xB))
#define FLASH_PAGE_SIZE          0x400U
#endif /* STM32F101x6 || STM32F102x6 || STM32F103x6 */
       /* STM32F100xB || STM32F101xB || STM32F102xB || STM32F103xB */

#if (defined(STM32F100xE) || defined(STM32F101xE) || defined(STM32F103xE) || defined(STM32F101xG) || defined(STM32F103xG) || defined(STM32F105xC) || defined(STM32F107xC))
#define FLASH_PAGE_SIZE          0x800U
#endif /* STM32F100xB || STM32F101xB || STM32F102xB || STM32F103xB */
       /* STM32F101xG || STM32F103xG */ 
       /* STM32F105xC || STM32F107xC */

FlashIAP::FlashIAP(System& system) :
    _system(system)
{}

void FlashIAP::copyFlashBlock(uint32_t sourceAddress, uint32_t destinationAddress, int32_t size)
{
    // First erase space for the app at the destination address
    // erase(destinationAddress, size);

    _system.unlockFlash();
    while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
        ;

    // Then copy over pages one at a time from source to destination
    while(size != 0) {
        // uint8_t* buffer = 0;
        int32_t bytesUntilPageEnd = FLASH_PAGE_SIZE - (sourceAddress % FLASH_PAGE_SIZE);
        int32_t bytesToProgram = size;

        if (size > bytesUntilPageEnd) {
            bytesToProgram = bytesUntilPageEnd;
        }
        uint8_t buffer[bytesToProgram];

        // Read the bytes for this page into the buffer
        read(sourceAddress, buffer, bytesToProgram);

        // Erase the page at the destination
        _system.erasePage(destinationAddress);
        while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
            ;
        CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
        
        // Write the buffer into the destination
        // write(destinationAddress, buffer, bytesToProgram);
        _system.programHalfWords(destinationAddress, (uint16_t*)buffer, bytesToProgram);

        size -= bytesToProgram;
        sourceAddress += bytesToProgram;
        destinationAddress += bytesToProgram;
    }

    _system.lockFlash();
}

// size = size of the buffer
// buffer = array to store the read out half words
bool FlashIAP::read(uint32_t address, uint8_t* buffer, int32_t size, uint32_t timeout)
{
    memcpy(buffer, (const void*)address, size);
    return true;
}

bool FlashIAP::isAligned(uint32_t number, uint32_t alignment)
{
    return (number % alignment) == 0;
}

bool FlashIAP::isAlignedToPage(uint32_t address, uint32_t size)
{
    // - size is a multiple of page size
    // - address starts at the beginning of a page
    return isAligned(size, FLASH_PAGE_SIZE) && isAligned(address, FLASH_PAGE_SIZE);
}
