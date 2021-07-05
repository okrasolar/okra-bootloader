#pragma once

/* Length of the bootloader name string (including \0 termination) */
const uint8_t BOOTLOADER_NAME_LENGTH = 16;

/* Name of the bootloader. This name is copied to the
 * status flash at first boot */
const char BOOTLOADER_NAME[BOOTLOADER_NAME_LENGTH] = "Okra Bootloader";

/* Version of the bootloader */
const uint8_t BOOTLOADER_VERSION_BUILD = 1;
const uint8_t BOOTLOADER_VERSION_MINOR = 1;
const uint8_t BOOTLOADER_VERSION_MAJOR = 0;

/* Number of retries before switching to the next app */
const uint8_t BOOTLOADER_MAX_RETRIES = 2;

/* Flash address for the bootloader status structure.
 * This address must be aligned to a flash page. Each time a new application
 * is detected by the bootloader (BootloaderState::newApp),
 * the full page is erased, and the updated struct is written to the address */
const uint32_t BOOTLOADER_STATUS_STRUCT_ADDR = 0x08000800;

/* Number of apps in the flash memory. In most scenarios, this should
 * be set to 2, to allow A/B switching between apps after an update */
const uint8_t BOOTLOADER_MAX_APPS = 2;

#ifdef COPYBINARY
/* Source address of the applications */
const uint32_t BOOTLOADER_APP_ADDRESS[BOOTLOADER_MAX_APPS] = { 0x08040800, 0x08080000 };
#else
const uint32_t BOOTLOADER_APP_ADDRESS[BOOTLOADER_MAX_APPS] = { 0x08001000, 0x08040800 };
#endif

#ifdef COPYBINARY
/* Actual boot address */
const uint32_t BOOT_ADDRESS = 0x08001000;

/* Size of each app in bytes */
const int32_t APP_SIZE = 260096;
#endif

/* Bootloader state enumeration. This state needs to be set to "newApp"
 * by the application after an update, and to "stableApp" after the
 * first successful boot */
enum BootloaderState {
    noState = 0,     // State not properly initialized
    newApp,          // Set By App after download of the binary
    attemptNewApp,   // Set by Bootloader when first booting the new binary
    stableApp,       // Set by App after sucessful boot of the new app
};

/* Bootloader status struct. This struct's status field should be updated
 * in flash by the app after first boot (stableApp) or after an update of the
 * other firmware binary (newApp) */
struct BootloaderStatus {
    char bootloaderName[BOOTLOADER_NAME_LENGTH];
    uint32_t bootloaderVersion;
    uint32_t status;   // Update this field and write to flash in your app!
    uint32_t liveAppSelect;
    uint32_t retryCount;
};

/*
 * Enables the watchdog for the MCU. The actual implementation details,
 * as well as the value for the watchdog counter are platform dependent
 * and can be configured in the according System_*.cpp file.
 */
const bool ENABLE_WATCHDOG = true;
