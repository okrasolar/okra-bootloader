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

#include "System.h"

void System::readStatusReg(BootloaderStatus& status)
{
    status = { 0 };
}

void System::writeStatusReg(BootloaderStatus& status) {}

void System::executeFromAddress(uint32_t bootAddress) {}

void System::copyFlashBlock(uint32_t sourceAddress, uint32_t destinationAddress, int32_t size) {}

void System::readFlash(uint32_t address, uint8_t* data, int32_t size) {}

void System::erasePage(uint32_t address) {}

void System::programHalfWords(uint32_t address, uint16_t* data, uint32_t size) {}

void System::unlockFlash() {}

void System::lockFlash() {}

void System::enableWatchdog() {}
