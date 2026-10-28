#include "config.hpp"
#include "flight_state.hpp"
#include "logger.hpp"
#include "sensors.hpp"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

void init_sensors() {
    // Initialize I2C
    i2c_init(i2c0, 400 * 1000); // 400 kHz

    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    uint8_t wakeup_data[2] = {0x6B, 0x00};
    i2c_write_blocking(PICO_DEFAULT_I2C, MPU6050_ADDR, wakeup_data, 2, false);

    // Initialize MPU6050, BMP180
}
