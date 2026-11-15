
# Floppy 80 (M1) Software Build

## Floppy80 firmware

You need to install the C SDK for RPI
https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html#sdk-setup

CMake and Make are essential for this to work
* On Mac OSX - `brew install cmake`

```
# ensure you are in the firmware directory
cd firmware

# first run cmake - creating a separate build folder 
cmake -B build -S .

# then from the build folder
cd build

# run the make command
make Floppy80
```

this will create Floppy80.uf2 file which you burn to the RPI PICO

### Build options (speed / behaviour)

Pass these on the first cmake command, e.g. `cmake -B build -S . -DSD_FLUSH_DELAY_MS=0`
(with the VS Code extension, add them to the CMake configure arguments).

| Option | Default | Meaning |
|--------|---------|---------|
| `SD_LIBRARY` | `NEW` | SD card library. `NEW` is `no-OS-FatFS-SD-SDIO-SPI-RPi-Pico` (SDHC/SDXC cards only). `LEGACY` is the older `no-OS-FatFS-SD-SPI-RPi-Pico`, which also works with standard capacity cards (2 GB or smaller). Use `-DSD_LIBRARY=LEGACY` if you need an old small card. |
| `SD_SPI_KHZ` | `5000` | SD card SPI *boot* clock in kHz. The card is always mounted, and `system.cfg` read, at this speed (5000 = the original firmware). To go faster, set `SDKHZ=nnnn` in `system.cfg` (see USER-GUIDE.md) instead of changing this: cards and wiring differ, and a card that cannot keep up at the higher speed still boots at the safe one. |
| `SD_FLUSH_DELAY_MS` | `200` | Idle time before buffered writes are synced (`f_sync`) to the SD card. `0` syncs after every write (original behaviour, slowest, safest if power can drop right after a write). |
| `SD_FLUSH_MAX_MS` | `2000` | Longest time unsynced writes may stay buffered during continuous writing. |
| `SD_CRC_ENABLED` | `ON` | Verify CRC on SD data blocks. `OFF` is a little faster at high SPI clocks. |
| `SD_SPI_MODE` | `0` | SPI mode (0-3) for the SD card. 0 is the standard setting; the SD library notes say mode 3 can be around 15% faster with some cards. Test thoroughly before relying on it. |
| `SD_MESSAGES` | `ON` | Print SD card driver error and info messages on the USB console. |
| `SD_DEBUG_MESSAGES` | `OFF` | Also print the driver's debug messages. Very verbose; use it when chasing SD card problems. |
| `FDC_LOGGING` | `OFF` | Log Z80 register accesses for the CLI log command. Adds work to every bus access on core 1, so leave it off unless you are debugging. |
| `SYSCLOCK` | `200` (Pico), SDK default `150` (Pico 2) | System clock in MHz, e.g. `-DSYSCLOCK=150`. Works for both boards. The SDK applies it before `main()` (SDK 2.1.1 or later). The `NopDelay()` bus delays in `memory.c` are tuned for 200 MHz (Pico) and 150 MHz (Pico 2), so test other speeds with a real Z80. The current value is shown on the `Clock` line of the status listing. |

### SD card library

Two SD card libraries are included and one is chosen at build time with `-DSD_LIBRARY=`:

* `NEW` (default): `firmware/lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico` (v3.x, FatFs R0.15). It needs Pico SDK 2.0.0 or later and officially supports the Pico and Pico 2. It only supports SDHC and SDXC cards, so standard capacity cards (up to and including 2 GB) are not supported. Only its SPI interface is used: the SD card is wired to SPI0 (see `hw_config.c`), and SDIO would need four data lines on consecutive GPIOs.
* `LEGACY`: `firmware/lib/no-OS-FatFS-SD-SPI-RPi-Pico-master`. The older SPI-only library. It also works with standard capacity cards (2 GB or smaller), so choose it if you have to use an old small card:

```
cmake -B build -S . -DSD_LIBRARY=LEGACY
```

`SD_SPI_MODE`, `SD_MESSAGES` and `SD_DEBUG_MESSAGES` only apply to the `NEW` library. Use a fresh build folder when you change `SD_LIBRARY`.

### Background Reading

These are DEV resources:
* https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html
* https://admantium.medium.com/getting-started-with-raspberry-pico-and-cmake-f536e18512e6

## FDC (for TRSDOS)

FDC TRS (utility for the TRSDOS related utility)

```
cd FDC_TRS`
zmac fdc.asm
```

use TRS80GP to import FDC/CMD onto a disk image

```
trs80gp -m1 -vs -frehd -frehd_dir zout -turbo -mem 48 -d0 dmk\ld531-0.dmk -d1 dmk\ld531-1.dmk -i "IMPORT2 FDC.CMD FDC/CMD:1\r"
```

## FDC (for CP/M)

FDC CP/M (utility for the CP/M related utility)

Compile program into a hex file.

```
cd FDC_CPM`
zmac fdc.asm
```

Run TRS80GP loading the CP/M disk images.

```
trs80gp -m1 -vs -frehd -frehd_dir zout -turbo -mem 48 -d0 CPM141-0.dmk -d1 CPM141-1.dmk -d2 CPM141-2.dmk
```

Select Load from TRS80GP File menu and select the hex file.

Run the SAVE command to save program memory contents to a .COM file.
As the program grows you will need to increase the value after SAVE.
i.e. `SAVE n ufn` Where :
* n   - number of 256-byte pages to be saved.
* ufn - unambiguous file name.

e.g.

`SAVE 20 B:FDC.COM`
