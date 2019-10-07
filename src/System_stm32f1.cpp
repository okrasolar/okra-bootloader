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
#include "stm32f1xx.h"

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

    // Unlock the flash
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);

    // Erase page
    SET_BIT(FLASH->CR, FLASH_CR_PER);
    WRITE_REG(FLASH->AR, BOOTLOADER_STATUS_STRUCT_ADDR);
    SET_BIT(FLASH->CR, FLASH_CR_STRT);

    // Wait until the page erase is finished
    while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
        ;
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER);

    // Write status to the flash
    uint16_t* data = (uint16_t*)&status;
    uint32_t address = BOOTLOADER_STATUS_STRUCT_ADDR;
    for (unsigned int i = 0; i < sizeof(status) / sizeof(uint16_t); i++) {
        SET_BIT(FLASH->CR, FLASH_CR_PG);
        *(__IO uint16_t*)address = *data;
        while (READ_BIT(FLASH->SR, FLASH_SR_BSY))
            ;
        CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
        data++;
        address += 2;
    }

    // Lock the flash again
    SET_BIT(FLASH->CR, FLASH_CR_LOCK);
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
