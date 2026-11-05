#include <string.h>
#include "hw_config.h"
#include "ff.h"

// Define the SPI hardware and pins you chose
static sd_spi_if_t spi_if = {
    .spi = spi0,          // Which SPI peripheral (spi0 or spi1)
    .miso_gpio = 16,      // MISO pin
    .mosi_gpio = 19,      // MOSI pin
    .sck_gpio = 18,       // SCK pin
    .baud_rate = 12500000 // SPI clock speed (12.5 MHz after initialization)
};

// Define the SD Card slot configuration
static sd_card_t sd_cards[] = {
    {
        .pcName = "",                 // Default drive name ("")
        .type = SD_IF_SPI,
        .spi_if_p = &spi_if,
        .ss_gpio = 17,                // Chip Select (CS) pin
        .use_card_detect = false,     // Set true if your module has a card detect pin
    }
};

size_t sd_get_num() {
    return count_of(sd_cards);
}

sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    }
    break;
}
