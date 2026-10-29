#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define MPU6050_ADDR 0x68

const uint PIN_SDA_MPU6050 = 4; // GPIO pin for SDA
const uint PIN_SCL_MPU6050 = 5; // GPIO pin for SCL

void init_i2c() {
    // Initialize I2C at 400kHz
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(PIN_SDA_MPU6050, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL_MPU6050, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA_MPU6050);
    gpio_pull_up(PIN_SCL_MPU6050);
}

// read bytes from a register
int read_register(i2c_inst_t *i2c, uint8_t addr, uint8_t reg, uint8_t *buf, size_t len) {
    // write the register address we want to read from (with nostop = true)
    int count = i2c_write_blocking(i2c, addr, &reg, 1, true);
    if (count < 0) {
        return 0; // Error: address not acknowledged or bus error
    }
    
    // Read the data into the buffer
    return i2c_read_blocking(i2c, addr, buf, len, false);
}

void check_mpu6050() {
    uint8_t reg = 0x75;
    uint8_t value = 0;
    
    // Wake up the MPU6050 first (write 0x00 to power management register 0x6B)
    uint8_t wakeup_data[2] = { 0x6B, 0x00 };
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, wakeup_data, 2, false);

    // Read WHO_AM_I
    read_register(I2C_PORT, MPU6050_ADDR, reg, &value, 1);
    printf("MPU6050 WHO_AM_I: 0x%X\n", value);
}