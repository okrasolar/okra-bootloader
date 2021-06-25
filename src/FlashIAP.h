#ifndef FlashIAP_h
#define FlashIAP_h

#include "System.h"
#include <cstdint>
#include <cstring>

// Minimum number of bytes to be programmed at a time
#define MIN_PROG_SIZE 2U   // half word
#define INVALID_PAGE_SIZE 0xFFFFFFFF

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
