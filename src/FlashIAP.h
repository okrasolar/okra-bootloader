#ifndef FlashIAP_h
#define FlashIAP_h

#include "System.h"
#include "stm32f1xx.h"
#include <cstdint>
#include <cstring>

// Minimum number of bytes to be programmed at a time
#define MIN_PROG_SIZE 2U   // half word
#define INVALID_PAGE_SIZE 0xFFFFFFFF

#if (defined(STM32F101x6) || defined(STM32F102x6) || defined(STM32F103x6) || defined(STM32F100xB) || defined(STM32F101xB) || defined(STM32F102xB) || defined(STM32F103xB))
#define FLASH_PAGE_SIZE          0x400U
#endif /* STM32F101x6 || STM32F102x6 || STM32F103x6 */
       /* STM32F100xB || STM32F101xB || STM32F102xB || STM32F103xB */

#if (defined(STM32F100xE) || defined(STM32F101xE) || defined(STM32F103xE) || defined(STM32F101xG) || defined(STM32F103xG) || defined(STM32F105xC) || defined(STM32F107xC))
#define FLASH_PAGE_SIZE          0x800U
#endif /* STM32F100xB || STM32F101xB || STM32F102xB || STM32F103xB */
       /* STM32F101xG || STM32F103xG */ 
       /* STM32F105xC || STM32F107xC */

class FlashIAP
{
  public:
    FlashIAP(System& system);
    ~FlashIAP() = default;

    void copyFlashBlock(uint32_t sourceAddress, uint32_t destinationAddress, int32_t size);

    // TODO: confirm thread safety
    bool read(uint32_t address, uint8_t* data, int32_t size, uint32_t timeout = 500);

    // TODO: confirm thread safety
    bool write(uint32_t address, const uint8_t* buffer, int32_t size, uint32_t timeout = 500);
    bool erase(uint32_t address, int32_t size, uint32_t timeout = 4000);

  private:
    bool isAligned(uint32_t number, uint32_t alignment);
    bool isAlignedToPage(uint32_t address, uint32_t size);

    System& _system;
};

#endif
