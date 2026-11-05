#include <string.h>
#include "hw_config.h"

// Define SPI1 controller configuration for pins 10-12
static spi_t spis[] = {
    {
        .hw_inst = spi1,        // Switch to spi1
        .sck_gpio = 10,         // SCK on GP10
        .mosi_gpio = 11,        // MOSI on GP11
        .miso_gpio = 12,        // MISO on GP12
        .baud_rate = 400 * 1000 // Safe initial clock speed (400 kHz)
    }
};

// Define SD card configuration pointing to SPI1 and Chip Select on GP13
static sd_card_t sd_cards[] = {
    {
        .pcName = "0:",         // FatFs logical drive name
        .spi = &spis[0],        // Pointer to the SPI1 configuration
        .ss_gpio = 13,          // Chip Select (CS) on GP13
        .use_card_detect = false
    }
};

// Required library interface functions
size_t sd_get_num() {
    return count_of(sd_cards);
}

sd_card_t *sd_get_by_num(size_t num) {
    if (num < sd_get_num()) {
        return &sd_cards[num];
    }
    return NULL;
}

size_t spi_get_num() {
    return count_of(spis);
}

spi_t *spi_get_by_num(size_t num) {
    if (num < spi_get_num()) {
        return &spis[num];
    }
    return NULL;
}