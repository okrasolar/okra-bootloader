#include "FlashIAP.h"

FlashIAP::FlashIAP(System& system) :
    _system(system)
{}

void FlashIAP::copyFlashBlock(uint32_t sourceAddress, uint32_t destinationAddress, int32_t size)
{
    // First erase space for the app at the destination address
    erase(destinationAddress, size);

    // Then copy over pages one at a time from source to destination
    while(size != 0) {
        uint8_t* buffer = 0;
        int32_t bytesUntilPageEnd = FLASH_PAGE_SIZE - (sourceAddress % FLASH_PAGE_SIZE);
        int32_t bytesToProgram = size;

        if (size > bytesUntilPageEnd) {
            bytesToProgram = bytesUntilPageEnd;
        }

        // Read the bytes for this page into the buffer
        read(sourceAddress, buffer, bytesToProgram);
        // Write those same bytes into the destination
        write(destinationAddress, buffer, bytesToProgram);

        size -= bytesToProgram;
        sourceAddress += bytesToProgram;
        destinationAddress += bytesToProgram;
    }
}

// size = size of the buffer
// buffer = array to store the read out half words
bool FlashIAP::read(uint32_t address, uint8_t* buffer, int32_t size, uint32_t timeout)
{
    memcpy(buffer, (const void*)address, size);
    return true;
}

bool FlashIAP::write(uint32_t address, const uint8_t* buffer, int32_t size, uint32_t timeout)
{
    // Address should be aligned to min programming size
    // totalBytes must be aligned to min programming size
    if (!isAligned(address, MIN_PROG_SIZE) || !isAligned(size, MIN_PROG_SIZE)) {
        return false;
    }

    bool success = true;

    while (size != 0) {
        int32_t bytesUntilPageEnd = FLASH_PAGE_SIZE - (address % FLASH_PAGE_SIZE);
        int32_t bytesToProgram = size;

        if (size > bytesUntilPageEnd) {
            bytesToProgram = bytesUntilPageEnd;
        }
        _system.programHalfWords(address, (uint16_t*)buffer, bytesToProgram);

        size -= bytesToProgram;
        address += bytesToProgram;
        buffer += bytesToProgram;
    }

    return success;
}

// will only erase full pages at a time
// size = number of bytes to erase
bool FlashIAP::erase(uint32_t address, int32_t size, uint32_t timeout)
{
    bool success = true;

    while (size != 0) {
        if (!isAlignedToPage(address, size)) {
            success = false;
            break;
        }
        _system.erasePage(address);
        size -= FLASH_PAGE_SIZE;
        address += FLASH_PAGE_SIZE;
    }

    return success;
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
