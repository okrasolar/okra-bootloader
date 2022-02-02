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

// Minimum number of bytes to be programmed at a time
#define MIN_PROG_SIZE 2U   // half word
#define INVALID_PAGE_SIZE 0xFFFFFFFF

#if (defined(STM32F101x6) || defined(STM32F102x6) || defined(STM32F103x6) || defined(STM32F100xB) || defined(STM32F101xB) || defined(STM32F102xB) || defined(STM32F103xB))
#define FLASH_PAGE_SIZE          0x400U
#include "stm32f1xx.h"
       /* STM32F101x6 || STM32F102x6 || STM32F103x6 */
       /* STM32F100xB || STM32F101xB || STM32F102xB || STM32F103xB */

#elif (defined(STM32F100xE) || defined(STM32F101xE) || defined(STM32F103xE) || defined(STM32F101xG) || defined(STM32F103xG) || defined(STM32F105xC) || defined(STM32F107xC))
#define FLASH_PAGE_SIZE          0x800U
#include "stm32f1xx.h"
       /* STM32F100xB || STM32F101xB || STM32F102xB || STM32F103xB */
       /* STM32F101xG || STM32F103xG */
       /* STM32F105xC || STM32F107xC */

#elif (defined(STM32F302xC) || defined(STM32F303xC) || defined(STM32F358xx) || defined(STM32F373xC) || defined(STM32F378xx) || defined(STM32F302xE) || defined(STM32F303xE) \
    || defined(STM32F398xx) || defined(STM32F301x8) || defined(STM32F302x8) || defined(STM32F318xx) || defined(STM32F303x8) || defined(STM32F334x8) || defined(STM32F328xx) \
    || defined(STM32F302xC) || defined(STM32F303xC) || defined(STM32F358xx) || defined(STM32F373xC) || defined(STM32F378xx) || defined(STM32F302xE) || defined(STM32F303xE) \
    || defined(STM32F398xx) || defined(STM32F301x8) || defined(STM32F302x8) || defined(STM32F318xx) || defined(STM32F303x8) || defined(STM32F334x8) || defined(STM32F328xx))
#define FLASH_PAGE_SIZE          0x800U
#include "stm32f3xx.h"
#endif /* STM32F302xC || STM32F303xC || STM32F358xx || */
       /* STM32F373xC || STM32F378xx */
       /* STM32F301x8 || STM32F302x8 || STM32F318xx || */
       /* STM32F303x8 || STM32F334x8 || STM32F328xx */
       /* STM32F302xC || STM32F303xC || STM32F358xx || */
       /* STM32F373xC || STM32F378xx */
       /* STM32F302xE || STM32F303xE || STM32F398xx */
       /* STM32F301x8 || STM32F302x8 || STM32F318xx || */
       /* STM32F303x8 || STM32F334x8 || STM32F328xx */

// Watchdog clock frequency is ~40kHz, divide clock by 64
static const uint32_t WATCHDOG_PRESCALER = 0b100;

// Watchdog timeout of 5 seconds
static const uint32_t WATCHDOG_COUNTER = 3125;

/* application entry point */
typedef void (*AppEntry)(void);

/* static variables */
volatile static uint32_t stackPointer = 0;
volatile static uint32_t applicationEntry = 0;
volatile static AppEntry application = 0;

void System::readStatusReg(BootloaderStatus& status)
{
    status = *((BootloaderStatus*)BOOTLOADER_STATUS_STRUCT_ADDR);
}

void System::writeStatusReg(BootloaderStatus& status)
{
    // Write the status register to the flash
    // See ST PM0075 on how to program the flash memory
    // https://www.st.com/resource/en/programming_manual/cd00283419.pdf

    // Make sure the status reg is halfword aligned
    static_assert(sizeof(status) % 2 == 0);

    unlockFlash();
    while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
        ;

    erasePage(BOOTLOADER_STATUS_STRUCT_ADDR);
    while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
        ;
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER);

    uint16_t* data = (uint16_t*)&status;
    uint32_t address = BOOTLOADER_STATUS_STRUCT_ADDR;
    uint32_t size = sizeof(status);
    programHalfWords(address, data, size);

    lockFlash();
    while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
        ;
}

void System::executeFromAddress(uint32_t bootAddress)
{
    /* cast to vector table */
    uint32_t* vectorTable = (uint32_t*)bootAddress;

    /* Grab the stack pointer and program counter from the vector table */
    stackPointer = vectorTable[0];
    applicationEntry = vectorTable[1];
    application = (AppEntry)applicationEntry;

    /* Vector table redirection */
    SCB->VTOR = bootAddress;

    /* Change MSP and PSP */
    __set_MSP(stackPointer);
    __set_PSP(stackPointer);

    /* Memory barrier */
    __DSB();
    __ISB();

    /* Jump to application */
    application();

    /* Should never reach here. */
    __NOP();
}

void System::copyFlashBlock(uint32_t sourceAddress, uint32_t destinationAddress, int32_t size)
{
    // First unlock flash
    unlockFlash();
    while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
        ;

    // Then copy over pages one at a time from source to destination
    while(size != 0) {
        int32_t bytesUntilPageEnd = FLASH_PAGE_SIZE - (sourceAddress % FLASH_PAGE_SIZE);
        int32_t bytesToProgram = size;

        if (size > bytesUntilPageEnd) {
            bytesToProgram = bytesUntilPageEnd;
        }
        uint8_t buffer[bytesToProgram];

        // Read the bytes for this page into the buffer
        readFlash(sourceAddress, buffer, bytesToProgram);

        // Erase the page at the destination
        erasePage(destinationAddress);
        while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
            ;
        CLEAR_BIT(FLASH->CR, FLASH_CR_PER);

        // Write the buffer into the destination
        programHalfWords(destinationAddress, (uint16_t*)buffer, bytesToProgram);

        size -= bytesToProgram;
        sourceAddress += bytesToProgram;
        destinationAddress += bytesToProgram;
    }

    lockFlash();
}

void System::readFlash(uint32_t address, uint8_t* data, int32_t size)
{
    uint8_t* src = (uint8_t*)address;
    for (int i = 0; i < size; i++) {
        *data++ = *src++;
    }
}

void System::erasePage(uint32_t address)
{
    // Erase page
    SET_BIT(FLASH->CR, FLASH_CR_PER);
    WRITE_REG(FLASH->AR, address);
    SET_BIT(FLASH->CR, FLASH_CR_STRT);
}

void System::programHalfWords(uint32_t address, uint16_t* data, uint32_t size)
{
    for (unsigned int i = 0; i < size / sizeof(uint16_t); i++) {
        SET_BIT(FLASH->CR, FLASH_CR_PG);
        *(__IO uint16_t*)address = *data;
        while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
            ;
        CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
        data++;
        address += 2;
    }
}

void System::unlockFlash()
{
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);
}

void System::lockFlash()
{
    SET_BIT(FLASH->CR, FLASH_CR_LOCK);
}

void System::enableWatchdog()
{
    WRITE_REG(IWDG->KR, 0x5555);               // Disable write protection of IWDG registers
    WRITE_REG(IWDG->PR, WATCHDOG_PRESCALER);   // Clock frequency is ~40kHz, divide clock by 64
    WRITE_REG(IWDG->RLR, WATCHDOG_COUNTER);    // Watchdog timeout of 5 seconds
    WRITE_REG(IWDG->KR, 0xAAAA);               // Reload IWDG
    WRITE_REG(IWDG->KR, 0xCCCC);               // Start IWDG
    WRITE_REG(IWDG->KR, 0xAAAA);               // Kick IWDG
}
