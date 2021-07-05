/*
 * Okra Bootloader
 * Copyright (C) 2019 Okra Solar Pty Ltd
 * https://www.okrasolar.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#pragma once

#include <cstdint>

#include "Config.h"

class System
{
  public:
    /**
     * @brief read the status from flash
     *
     * @param status struct to read the status into
     */
    void readStatusReg(BootloaderStatus& status);

    /**
     * @brief write the status into flash
     *
     * @param status new status data to write
     */
    void writeStatusReg(BootloaderStatus& status);

    /**
     * @brief execute the binary at address bootAddress
     *
     * @param bootAddress absolute memory address of the binary to execute
     */
    void executeFromAddress(uint32_t bootAddress);

    /**
     * @brief copy a block of flash from one address to another
     *
     * @param sourceAddress absolute memory address of the flash block
     * @param destinationAddress absolute memory of the location to write the flash block
     * @param size size in bytes of the flash block
     */
    void copyFlashBlock(uint32_t sourceAddress, uint32_t destinationAddress, int32_t size);

    /**
     * @brief read a block of flash into a data buffer
     *
     * @param address absolute memory address of the flash block
     * @param data buffer used for reading the data
     * @param size size of the data to read
     */
    void readFlash(uint32_t address, uint8_t* data, int32_t size);

    /**
     * @brief erase a page of flash at specified address
     *
     * @param address
     */
    void erasePage(uint32_t address);

    /**
     * @brief program up to a single page of flash
     *
     * @param address absolute memory address to program
     * @param data pointer to data that we want to program
     * @param size size in bytes of the data that we want to program
     */
    void programHalfWords(uint32_t address, uint16_t* data, uint32_t size);

    /**
     * @brief unlock the flash
     */
    void unlockFlash();

    /**
     * @brief lock the flash
     */
    void lockFlash();

    /**
     * @brief enables the MCU's watchdog.
     */
    void enableWatchdog();
};
