# Floppy80 firmware: CMake build options

Set an option on the first `cmake` command with `-DNAME=value`, for example:

```
cd firmware
cmake -B build -S . -DPICO_TARGET=PICO -DSYSCLOCK=150
cd build
make
```

CMake remembers option values in the build folder. Use a fresh build folder when you change
`PICO_TARGET` or `SD_LIBRARY`, or delete the cache (`CMakeCache.txt`) before reconfiguring.

## Target and clock

| Option | Default | Values | Description |
|--------|---------|--------|-------------|
| `PICO_TARGET` | `PICO2` | `PICO`, `PICO2` | Selects the board. Any other value stops with an error. |
| `SYSCLOCK` | `200` (Pico), SDK default `150` (Pico 2) | MHz, e.g. `125`, `133`, `150`, `200` | System clock, applied by the SDK before `main()`. Needs Pico SDK 2.1.1 or later. The bus delays in `memory.c` are tuned for 200 MHz (Pico) and 150 MHz (Pico 2), so test other speeds with a real Z80. The current clock is shown on the `Clock` line of the status listing. |

## SD card library

| Option | Default | Values | Description |
|--------|---------|--------|-------------|
| `SD_LIBRARY` | `NEW` | `NEW`, `LEGACY` | `NEW` is `no-OS-FatFS-SD-SDIO-SPI-RPi-Pico` (SDHC/SDXC cards only, needs SDK 2.0.0 or later). `LEGACY` is the older `no-OS-FatFS-SD-SPI-RPi-Pico`, which also works with standard capacity cards (2 GB or smaller). |
| `SD_SPI_KHZ` | `5000` | kHz | SD card SPI *boot* clock, used to mount the card and read `system.cfg`. `SDKHZ` in `system.cfg` can raise it afterwards. The actual rate is shown on the `SPI` line of the status listing. |
| `SD_FLUSH_DELAY_MS` | `200` | ms, `0` = off | Idle time before buffered writes are synced to the card. `0` syncs after every write (original behaviour, slowest, safest if power can drop right after a write). |
| `SD_FLUSH_MAX_MS` | `2000` | ms | Longest time unsynced writes may stay buffered during continuous writing. |
| `SD_CRC_ENABLED` | `ON` | `ON`, `OFF` | Checks CRC on SD data blocks. `OFF` is a little faster at high SPI clocks but less robust. |

### Only used with `SD_LIBRARY=NEW`

These are ignored when `SD_LIBRARY=LEGACY`.

| Option | Default | Values | Description |
|--------|---------|--------|-------------|
| `SD_SPI_MODE` | `0` | `0`-`3` | SPI mode for the card. `0` is the standard setting. The library notes say mode 3 can be around 15% faster with some cards, so test thoroughly before relying on it. |
| `SD_MESSAGES` | `ON` | `ON`, `OFF` | Prints SD driver error and info messages on the USB console. |
| `SD_DEBUG_MESSAGES` | `OFF` | `ON`, `OFF` | Also prints the driver's debug messages. Very verbose; use it when chasing SD card problems. |

## Debugging

| Option | Default | Values | Description |
|--------|---------|--------|-------------|
| `FDC_LOGGING` | `OFF` | `ON`, `OFF` | Records Z80 register accesses for the CLI `log` command. Adds work to every bus access on core 1, so leave it off unless you are debugging. |

## Standard CMake and Pico SDK settings

| Option | Description |
|--------|-------------|
| `-B <folder>`, `-S .` | Build folder and source folder. |
| `-DCMAKE_BUILD_TYPE=Debug` or `Release` | Build type. The Pico SDK defaults to Release. |
| `-DPICO_SDK_PATH=<path>` | Location of the Pico SDK, if the `PICO_SDK_PATH` environment variable is not set. |

## Examples

Pico 2 with the defaults:

```
cmake -B build -S .
```

Pico at 150 MHz:

```
cmake -B build -S . -DPICO_TARGET=PICO -DSYSCLOCK=150
```

Support for old standard capacity cards (2 GB or smaller):

```
cmake -B build -S . -DSD_LIBRARY=LEGACY
```

Original write behaviour (sync after every write) and driver debug output:

```
cmake -B build -S . -DSD_FLUSH_DELAY_MS=0 -DSD_DEBUG_MESSAGES=ON
```

## Settings in system.cfg (not CMake options)

These are set in `system.cfg` on the SD card, not on the command line. See `USER-GUIDE.md` for details.

| Setting | Description |
|---------|-------------|
| `SDKHZ` | SD card SPI clock in kHz, applied after the card is mounted at the `SD_SPI_KHZ` boot speed. Range 1000 to 25000. |
| `L3RAM` | `1` enables 1 KB of RAM at 3000h-33FFh. |
| `L3RAMF` | File loaded into the L3RAM area at power on. Only has an effect when `L3RAM=1`. |
