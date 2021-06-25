#pragma once

#include <cstdint>


class IFlashStorage
{
  public:
    virtual ~IFlashStorage() = default;
    virtual void copyFlashBlock(uint32_t sourceAddress, uint32_t destinationAddress, int32_t size) = 0;
    virtual bool read(uint32_t registerAddress, uint8_t* data, int32_t readLen, uint32_t timeout = 500) = 0;
    // virtual bool write(uint32_t registerAddress, const uint8_t* data, int32_t writeLen, uint32_t timeout = 500) = 0;
    // virtual bool erase(uint32_t address, int32_t size, uint32_t timeout = 4000) = 0;
};
