/* hw_config.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use 
this file except in compliance with the License. You may obtain a copy of the 
License at

   http://www.apache.org/licenses/LICENSE-2.0 
Unless required by applicable law or agreed to in writing, software distributed 
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR 
CONDITIONS OF ANY KIND, either express or implied. See the License for the 
specific language governing permissions and limitations under the License.
*/
/*

This file should be tailored to match the hardware design.

There should be one element of the spi[] array for each hardware SPI used.

There should be one element of the sd_cards[] array for each SD card slot.
The name is should correspond to the FatFs "logical drive" identifier.
(See http://elm-chan.org/fsw/ff/doc/filename.html#vol)
The rest of the constants will depend on the type of
socket, which SPI it is driven by, and how it is wired.

*/

#include <assert.h>
#include <string.h>
//
#include "my_debug.h"
//
#include "hw_config.h"
//
#include "ff.h" /* Obtains integer types */
//
#include "diskio.h" /* Declarations of disk functions */

// SD card SPI "boot" clock: used to initialise and mount the card and read system.cfg. CMake passes
// SD_SPI_BAUD_HZ from the SD_SPI_KHZ option. SDKHZ in system.cfg can raise it afterwards (see sd_core.c).
#ifndef SD_SPI_BAUD_HZ
#define SD_SPI_BAUD_HZ (5000 * 1000)
#endif

//=============================================================================================
// LEGACY library (no-OS-FatFS-SD-SPI-RPi-Pico), selected with cmake -DSD_LIBRARY=LEGACY.
// Also accepts standard capacity SD cards (2 GB or smaller).
//=============================================================================================
#ifdef SD_LIB_LEGACY

// void spi0_dma_isr();

/* 
This example assumes the following hardware configuration:

|       | SPI0  | GPIO  | Pin   | SPI       | MicroSD   | Description            | 
| ----- | ----  | ----- | ---   | --------  | --------- | ---------------------- |
| MISO  | RX    | 4     | 6     | DO        | DO        | Master In, Slave Out   |
| MOSI  | TX    | 3     | 5     | DI        | DI        | Master Out, Slave In   |
| SCK   | SCK   | 2     | 4     | SCLK      | CLK       | SPI clock              |
| CS0   | CSn   | 5     | 7     | SS or CS  | CS        | Slave (or Chip) Select |
| DET   |       | 28    | 34    |           | CD        | Card Detect            |
| GND   |       |       | 18,23 |           | GND       | Ground                 |
| 3v3   |       |       | 36    |           | 3v3       | 3.3 volt power         |
*/

// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.
static spi_t spis[] = {  // One for each SPI.
    {
        .hw_inst = spi0,  // SPI component
        .miso_gpio = 4, // GPIO number (not pin number)
        .mosi_gpio = 3,
        .sck_gpio  = 2,
        /* The choice of SD card matters! SanDisk runs at the highest speed. PNY
           can only mangage 5 MHz. Those are all I've tried. */
        //.baud_rate = 1000 * 1000,
        .baud_rate = SD_SPI_BAUD_HZ,        // Set with -DSD_SPI_KHZ=nnn (see CMakeLists.txt). The limitation here is SPI slew rate.
        //.baud_rate = 25 * 1000 * 1000,    // Actual frequency: 20833333. Has
        // worked for me with SanDisk.

        // // Following attributes are dynamically assigned
        // .dma_isr = spi0_dma_isr,
        // .initialized = false,  // initialized flag
    }
};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
    {
        .pcName = "0:",           // Name used to mount device
        .spi = &spis[0],          // Pointer to the SPI driving this card
        .ss_gpio = 5,             // The SPI slave select GPIO for this SD card
        .use_card_detect = false,
        .card_detect_gpio = 28,   // Card detect
        .card_detected_true = 1,  // What the GPIO read returns when a card is
                                  // present. Use -1 if there is no card detect.
        // // Following attributes are dynamically assigned
        // .m_Status = STA_NOINIT,
        // .sectors = 0,
        // .card_type = 0,
    }
};

// void spi0_dma_isr() { spi_irq_handler(&spis[0]); }

/* ********************************************************************** */
size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}
size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}

/* [] END OF FILE */

#else
//=============================================================================================
// NEW library (no-OS-FatFS-SD-SDIO-SPI-RPi-Pico): SDHC/SDXC cards only.
//=============================================================================================

// SPI mode 0 is the standard setting for SD cards. Mode 3 can be faster with some cards.
#ifndef SD_SPI_MODE
#define SD_SPI_MODE 0
#endif

/*
This design uses SPI0 for the SD card:

|       | SPI0  | GPIO  | Pin   | SPI       | MicroSD   | Description            |
| ----- | ----  | ----- | ---   | --------  | --------- | ---------------------- |
| MISO  | RX    | 4     | 6     | DO        | DO        | Master In, Slave Out   |
| MOSI  | TX    | 3     | 5     | DI        | DI        | Master Out, Slave In   |
| SCK   | SCK   | 2     | 4     | SCLK      | CLK       | SPI clock              |
| CS0   | CSn   | 5     | 7     | SS or CS  | CS        | Slave (or Chip) Select |
| DET   |       | 28    | 34    |           | CD        | Card Detect            |
| GND   |       |       | 18,23 |           | GND       | Ground                 |
| 3v3   |       |       | 36    |           | 3v3       | 3.3 volt power         |

Card Detect (GPIO 28) is read by the firmware itself (see sd_core.c), not by the library.
SDIO is not possible with this wiring: it needs four data lines on consecutive GPIOs.
*/

// Hardware Configuration of the SPI "object"
static spi_t spi = {
    .hw_inst   = spi0,  // SPI component
    .miso_gpio = 4,     // GPIO number (not pin number)
    .mosi_gpio = 3,
    .sck_gpio  = 2,
    /* The choice of SD card matters! Some cards run at the highest speed, others
       (for example some PNY cards) can only manage about 5 MHz. */
    .baud_rate = SD_SPI_BAUD_HZ,  // Set with -DSD_SPI_KHZ=nnn (see CMakeLists.txt)
    .spi_mode  = SD_SPI_MODE,     // Set with -DSD_SPI_MODE=n (see CMakeLists.txt)
};

// SPI interface of the SD card socket
static sd_spi_if_t spi_if = {
    .spi     = &spi,  // Pointer to the SPI driving this card
    .ss_gpio = 5,     // The SPI slave select GPIO for this SD card
};

// Hardware Configuration of the SD Card "object"
static sd_card_t sd_card = {
    .type            = SD_IF_SPI,
    .spi_if_p        = &spi_if,  // Pointer to the SPI interface driving this card
    .use_card_detect = false,    // the firmware reads the card detect pin itself
};

/* ********************************************************************** */
size_t sd_get_num() { return 1; }

sd_card_t *sd_get_by_num(size_t num) {
    if (0 == num) {
        return &sd_card;
    } else {
        return NULL;
    }
}

/* [] END OF FILE */

#endif  // SD_LIB_LEGACY
