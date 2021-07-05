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

#include "Bootloader.h"

void Bootloader::boot(System& system, bool enableWatchdog)
{
    /* grab the status reg */
    BootloaderStatus statusReg;
    system.readStatusReg(statusReg);

    /* Check if BootloaderStatus has ever been initialized */
    const char* src = BOOTLOADER_NAME;
    char* dst = statusReg.bootloaderName;
    for (int i = 0; i < BOOTLOADER_NAME_LENGTH; i++) {
        if (*src != *dst) {
            statusReg.status = BootloaderState::noState;
            break;
        }
    }

    /* Check if value for live app makes sense */
    if (statusReg.liveAppSelect >= BOOTLOADER_MAX_APPS) {
        statusReg.status = BootloaderState::noState;
    }

    /* Boot logic */
    switch (statusReg.status) {
        case BootloaderState::stableApp: {
            /* Good to go */
            break;
        }
        case BootloaderState::newApp: {
            /* Let's do it */
            statusReg.status = BootloaderState::attemptNewApp;
            statusReg.retryCount = 0;
            system.writeStatusReg(statusReg);
            #ifdef COPYBINARY
            system.copyFlashBlock(BOOTLOADER_APP_ADDRESS[statusReg.liveAppSelect], BOOT_ADDRESS, APP_SIZE);
            #endif
            break;
        }
        case BootloaderState::attemptNewApp: {
            statusReg.retryCount++;
            if (statusReg.retryCount >= BOOTLOADER_MAX_RETRIES) {
                statusReg.retryCount = 0;

                /* try other app */
                statusReg.liveAppSelect++;
                if (statusReg.liveAppSelect >= BOOTLOADER_MAX_APPS) {
                    statusReg.liveAppSelect = 0;
                }
                system.writeStatusReg(statusReg);
            } else {
                /* try again */
                system.writeStatusReg(statusReg); 
            }
            #ifdef COPYBINARY
            /* again copy app binary from the live app's location to boot location */
            system.copyFlashBlock(BOOTLOADER_APP_ADDRESS[statusReg.liveAppSelect], BOOT_ADDRESS, APP_SIZE);
            #endif
            break;
        }
        case BootloaderState::noState:
        default: {
            /* statusReg is uninitialized, this is likely the first boot */

            /* Store bootloader name in status flash */
            src = BOOTLOADER_NAME;
            dst = statusReg.bootloaderName;
            for (int i = 0; i < BOOTLOADER_NAME_LENGTH; i++) {
                *dst++ = *src++;
            }

            statusReg.bootloaderVersion = (BOOTLOADER_VERSION_BUILD << 16)
                + (BOOTLOADER_VERSION_MINOR << 8) + (BOOTLOADER_VERSION_MAJOR);

            /* first boot, always attempt to boot from app A */
            statusReg.status = BootloaderState::attemptNewApp;
            statusReg.liveAppSelect = 0;
            statusReg.retryCount = 0;

            #ifdef COPYBINARY
            system.copyFlashBlock(BOOTLOADER_APP_ADDRESS[statusReg.liveAppSelect], BOOT_ADDRESS, APP_SIZE);
            #endif
            system.writeStatusReg(statusReg);
            break;
        }
    }

    /* Watchdog must be enabled after copying over the app, if we had to do so */
    if (enableWatchdog) {
        system.enableWatchdog();
    }

    /* Boot the app */
    #ifdef COPYBINARY
    system.executeFromAddress(BOOT_ADDRESS);
    #else
    system.executeFromAddress(BOOTLOADER_APP_ADDRESS[statusReg.liveAppSelect]);
    #endif
}
