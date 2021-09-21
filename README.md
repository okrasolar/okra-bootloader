# Okra Bootloader - a simple bootloader for A/B firmware switching

This bootloader is useful for firmware updating from a running application and
A/B switching. In such a configuration, two firmware binaries are stored in
the MCUs flash: App A and app B. While app A is running, it can update app B,
and vice versa.

The bootloader is designed to be small and simple, it's total binary size is
only around 800 bytes.

At first boot and after updating the other binary, the app needs to inform
the bootloader by writing a status flag to a specific address in the flash
memory. The address and structure of the status data are defined in
'Config.h'

## Procedure for the application after each boot
- Read the 'BootloaderStatus' from 'BOOTLOADER_STATUS_STRUCT_ADDR'
- If 'BootloaderStatus::status' equals 'BootloaderState::attemptNewApp',
  change it to 'BootloaderState::stableApp'
- In case the status changed (first boot of a new app), write it back to the
  flash at 'BOOTLOADER_STATUS_STRUCT_ADDR'

## Procedure for the application when performing a firmware update
- Write the new firmware to the BOOTLOADER_APP_ADDRESS of the other application.
  When compiling, don't forget to update your linker scripts and startup code.
  Make sure the binary is correct, for example by verifying a checksum.
- Read the 'BootloaderStatus' from 'BOOTLOADER_STATUS_STRUCT_ADDR'
- Change the 'BootloaderStatus::status' to 'BootloaderState::newApp'
- Change the 'BootloaderStatus::liveAppSelect' to the number of the other app
- Write the struct back to 'BOOTLOADER_STATUS_STRUCT_ADDR'
- Reset the device
- The bootloader will try to boot the new app. If the app does not confirm to be
  functioning by setting 'BootloaderState::stableApp', the bootloader will
  switch back to the other app after 'BOOTLOADER_MAX_RETRIES' boots.

## Device support
The bootloader is written for the STM32F103RCT MCU. Porting to other Cortex-M
devices should be easy by replacing the files "startup.s", "system.c" and the
linker-script "linker.ld" for the architecture accordingly.

## Building
The build system is Meson + Ninja

## Build example
- `meson build --cross-file meson.cross.build`
- `cd build`
- `ninja`

## Build options
There is a single build option "COPYBINARY". When this option is enabled, the boot address is distinct
from the locations where the apps are stored. On boot, the live app is copied over from its stored location to the boot
address, and it is then booted from there. You can enable this option from the build folder with the following command:
- `meson configure -DCOPYBINARY=enabled`

## TODO
Currently, the bootloader erases the whole page at 'BOOTLOADER_STATUS_STRUCT_ADDR',
and it's content is lost. The bootloader should restore the contents of the page.
