#pragma once

/* Name of the bootloader. This name is copied to the
 * status flash at first boot */
const char* const BOOTLOADER_NAME = "Okra Bootloader";

/* Length of the bootloader name string (including \0 termination) */
const uint8_t BOOTLADER_NAME_LENGTH = 16;

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

/* Actual start addresses of the applications */
const uint32_t BOOTLOADER_APP_ADDRESS[2] = { 0x08006000, 0x08023000 };

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
    char bootloaderName[BOOTLADER_NAME_LENGTH];
    uint32_t bootloaderVersion;
    uint32_t status;   // Update this field and write to flash in your app!
    uint32_t liveAppSelect;
    uint32_t retryCount;
};
