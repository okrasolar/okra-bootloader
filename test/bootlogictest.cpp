#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "Bootloader.h"
#include "System.h"

#include <string.h>

BootloaderStatus inStatus;
BootloaderStatus outStatus;
uint32_t finalBootAddress = 0;
uint32_t copySourceAddress = 0;
uint32_t copyDestinationAddress = 0;
bool readCalled = false;
bool writeCalled = false;

void System::readStatusReg(BootloaderStatus& status)
{
    readCalled = true;
    status = inStatus;
}

void System::writeStatusReg(BootloaderStatus& status)
{
    writeCalled = true;
    outStatus = status;
}

void System::executeFromAddress(uint32_t bootAddress)
{
    finalBootAddress = bootAddress;
}

void System::enableWatchdog() {}

void System::copyFlashBlock(uint32_t sourceAddress, uint32_t destinationAddress, int32_t sizr)
{
    copySourceAddress = sourceAddress;
    copyDestinationAddress = destinationAddress;
}

TEST_GROUP(BootLogicTest){
    virtual void setup()
    {
        inStatus = { 0 };
        outStatus = { 0 };
        readCalled = false;
        writeCalled = false;
        finalBootAddress = 0x0;
    }
};

TEST(BootLogicTest, FirstBoot)
{
    System sys;
    bool enableWatchdog = false;

    inStatus = { 0 };

    Bootloader bl;
    bl.boot(sys, enableWatchdog);

    CHECK_TRUE(readCalled);
    CHECK_TRUE(writeCalled);

    STRCMP_EQUAL(BOOTLOADER_NAME, outStatus.bootloaderName);
    CHECK_EQUAL((BOOTLOADER_VERSION_BUILD << 16) + (BOOTLOADER_VERSION_MINOR << 8)
            + (BOOTLOADER_VERSION_MAJOR),
        outStatus.bootloaderVersion);
    CHECK_EQUAL(BootloaderState::attemptNewApp, outStatus.status);
    CHECK_EQUAL(0, outStatus.liveAppSelect);

    #ifdef COPYBINARY
    CHECK_EQUAL(copySourceAddress, BOOTLOADER_APP_ADDRESS[0]);
    CHECK_EQUAL(copyDestinationAddress, BOOT_ADDRESS);
    CHECK_EQUAL(BOOT_ADDRESS, finalBootAddress);
    #else
    CHECK_EQUAL(BOOTLOADER_APP_ADDRESS[0], finalBootAddress);
    #endif
}

TEST(BootLogicTest, BootCurrentAppA)
{
    System sys;
    bool enableWatchdog = false;

    strcpy(inStatus.bootloaderName, BOOTLOADER_NAME);
    inStatus.bootloaderVersion = (BOOTLOADER_VERSION_BUILD << 16) + (BOOTLOADER_VERSION_MINOR << 8)
        + (BOOTLOADER_VERSION_MAJOR);
    inStatus.status = BootloaderState::stableApp;
    inStatus.liveAppSelect = 0;

    Bootloader bl;
    bl.boot(sys, enableWatchdog);

    CHECK_TRUE(readCalled);
    CHECK_FALSE(writeCalled);

    #ifdef COPYBINARY
    CHECK_EQUAL(copySourceAddress, BOOTLOADER_APP_ADDRESS[0]);
    CHECK_EQUAL(copyDestinationAddress, BOOT_ADDRESS);
    CHECK_EQUAL(BOOT_ADDRESS, finalBootAddress);
    #else
    CHECK_EQUAL(BOOTLOADER_APP_ADDRESS[0], finalBootAddress);
    #endif
}

TEST(BootLogicTest, BootCurrentAppB)
{
    System sys;
    bool enableWatchdog = false;

    strcpy(inStatus.bootloaderName, BOOTLOADER_NAME);
    inStatus.bootloaderVersion = (BOOTLOADER_VERSION_BUILD << 16) + (BOOTLOADER_VERSION_MINOR << 8)
        + (BOOTLOADER_VERSION_MAJOR);
    inStatus.status = BootloaderState::stableApp;
    inStatus.liveAppSelect = 1;

    Bootloader bl;
    bl.boot(sys, enableWatchdog);

    CHECK_TRUE(readCalled);
    CHECK_FALSE(writeCalled);

    #ifdef COPYBINARY
    CHECK_EQUAL(copySourceAddress, BOOTLOADER_APP_ADDRESS[1]);
    CHECK_EQUAL(copyDestinationAddress, BOOT_ADDRESS);
    CHECK_EQUAL(BOOT_ADDRESS, finalBootAddress);
    #else
    CHECK_EQUAL(BOOTLOADER_APP_ADDRESS[1], finalBootAddress);
    #endif
}

TEST(BootLogicTest, BootNewAppA)
{
    System sys;
    bool enableWatchdog = false;

    strcpy(inStatus.bootloaderName, BOOTLOADER_NAME);
    inStatus.bootloaderVersion = (BOOTLOADER_VERSION_BUILD << 16) + (BOOTLOADER_VERSION_MINOR << 8)
        + (BOOTLOADER_VERSION_MAJOR);
    inStatus.status = BootloaderState::newApp;
    inStatus.liveAppSelect = 0;

    Bootloader bl;
    bl.boot(sys, enableWatchdog);

    CHECK_TRUE(readCalled);
    CHECK_TRUE(writeCalled);

    CHECK_EQUAL(BootloaderState::attemptNewApp, outStatus.status);

    #ifdef COPYBINARY
    CHECK_EQUAL(copySourceAddress, BOOTLOADER_APP_ADDRESS[0]);
    CHECK_EQUAL(copyDestinationAddress, BOOT_ADDRESS);
    CHECK_EQUAL(BOOT_ADDRESS, finalBootAddress);
    #else
    CHECK_EQUAL(BOOTLOADER_APP_ADDRESS[0], finalBootAddress);
    #endif
}

TEST(BootLogicTest, BootNewAppB)
{
    System sys;
    bool enableWatchdog = false;

    strcpy(inStatus.bootloaderName, BOOTLOADER_NAME);
    inStatus.bootloaderVersion = (BOOTLOADER_VERSION_BUILD << 16) + (BOOTLOADER_VERSION_MINOR << 8)
        + (BOOTLOADER_VERSION_MAJOR);
    inStatus.status = BootloaderState::newApp;
    inStatus.liveAppSelect = 1;

    Bootloader bl;
    bl.boot(sys, enableWatchdog);

    CHECK_TRUE(readCalled);
    CHECK_TRUE(writeCalled);

    CHECK_EQUAL(BootloaderState::attemptNewApp, outStatus.status);

    #ifdef COPYBINARY
    CHECK_EQUAL(copySourceAddress, BOOTLOADER_APP_ADDRESS[1]);
    CHECK_EQUAL(copyDestinationAddress, BOOT_ADDRESS);
    CHECK_EQUAL(BOOT_ADDRESS, finalBootAddress);
    #else
    CHECK_EQUAL(BOOTLOADER_APP_ADDRESS[1], finalBootAddress);
    #endif
}

TEST(BootLogicTest, BootBAfterAFails)
{
    System sys;
    bool enableWatchdog = false;

    strcpy(inStatus.bootloaderName, BOOTLOADER_NAME);
    inStatus.bootloaderVersion = (BOOTLOADER_VERSION_BUILD << 16) + (BOOTLOADER_VERSION_MINOR << 8)
        + (BOOTLOADER_VERSION_MAJOR);
    inStatus.status = BootloaderState::attemptNewApp;
    inStatus.liveAppSelect = 0;

    Bootloader bl;
    for (int i = 0; i < BOOTLOADER_MAX_RETRIES; i++) {
        bl.boot(sys, enableWatchdog);
        inStatus = outStatus;
    }

    CHECK_TRUE(readCalled);
    CHECK_TRUE(writeCalled);

    CHECK_EQUAL(BootloaderState::attemptNewApp, outStatus.status);

    #ifdef COPYBINARY
    CHECK_EQUAL(copySourceAddress, BOOTLOADER_APP_ADDRESS[1]);
    CHECK_EQUAL(copyDestinationAddress, BOOT_ADDRESS);
    CHECK_EQUAL(BOOT_ADDRESS, finalBootAddress);
    #else
    CHECK_EQUAL(BOOTLOADER_APP_ADDRESS[1], finalBootAddress);
    #endif
}

TEST(BootLogicTest, BootAAfterBFails)
{
    System sys;
    bool enableWatchdog = false;

    strcpy(inStatus.bootloaderName, BOOTLOADER_NAME);
    inStatus.bootloaderVersion = (BOOTLOADER_VERSION_BUILD << 16) + (BOOTLOADER_VERSION_MINOR << 8)
        + (BOOTLOADER_VERSION_MAJOR);
    inStatus.status = BootloaderState::attemptNewApp;
    inStatus.liveAppSelect = 1;

    Bootloader bl;
    for (int i = 0; i < BOOTLOADER_MAX_RETRIES; i++) {
        bl.boot(sys, enableWatchdog);
        inStatus = outStatus;
    }

    CHECK_TRUE(readCalled);
    CHECK_TRUE(writeCalled);

    CHECK_EQUAL(BootloaderState::attemptNewApp, outStatus.status);

    #ifdef COPYBINARY
    CHECK_EQUAL(copySourceAddress, BOOTLOADER_APP_ADDRESS[0]);
    CHECK_EQUAL(copyDestinationAddress, BOOT_ADDRESS);
    CHECK_EQUAL(BOOT_ADDRESS, finalBootAddress);
    #else
    CHECK_EQUAL(BOOTLOADER_APP_ADDRESS[0], finalBootAddress);
    #endif
}
