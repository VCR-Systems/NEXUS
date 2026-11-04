#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "include.hpp"

int imu_data;
int baro_data;

void read_mpu6050(int16_t accel[3], int16_t gyro[3]) {
    uint8_t buffer[14];
    uint8_t val = 0x3B; //starting register for accelerometer data
    //tell the sensor which register we want to start reading from
    i2c_write_blocking(IMU_I2C_PORT, IMU_ADDRESS, &val, 1, true);
    //read 14 consecutive bytes of data (Accel X/Y/Z, Temp, Gyro X/Y/Z)
    i2c_read_blocking(IMU_I2C_PORT, IMU_ADDRESS, buffer, 14, false);
    //combine high and low bytes into 16-bit integers
    accel[0] = (buffer[0] << 8) | buffer[1];  // Accel X
    accel[1] = (buffer[2] << 8) | buffer[3];  // Accel Y
    accel[2] = (buffer[4] << 8) | buffer[5];  // Accel Z

    gyro[0]  = (buffer[8] << 8) | buffer[9];  // Gyro X
    gyro[1]  = (buffer[10] << 8) | buffer[11]; // Gyro Y
    gyro[2]  = (buffer[12] << 8) | buffer[13]; // Gyro Z
}

void read_bmp180(int32_t *temperature, int32_t *pressure) {
    uint8_t buffer[3];
    uint8_t val = 0xF4; //control register
    uint8_t cmd = 0x2E; //command to read temperature
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &val, 1, true);
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &cmd, 1, false);
    sleep_ms(5); //wait for conversion
    val = 0xF6; //data register
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &val, 1, true);
    i2c_read_blocking(BARO_I2C_PORT, BARO_ADDRESS, buffer, 2, false);
    *temperature = (buffer[0] << 8) | buffer[1];

    cmd = 0x34; //command to read pressure
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &val, 1, true);
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &cmd, 1, false);
    sleep_ms(5); //wait for conversion
    val = 0xF6; //data register
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &val, 1, true);
    i2c_read_blocking(BARO_I2C_PORT, BARO_ADDRESS, buffer, 3, false);
    *pressure = (buffer[0] << 16) | (buffer[1] << 8) | buffer[2];
}

int main() {
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_set_function(IMU_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(IMU_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(IMU_SDA_PIN);
    gpio_pull_up(IMU_SCL_PIN);
    i2c_init(IMU_I2C_PORT, 400 * 1000);

    gpio_set_function(BARO_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(BARO_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(BARO_SDA_PIN);
    gpio_pull_up(BARO_SCL_PIN);
    i2c_init(BARO_I2C_PORT, 400 * 1000);
    
    uint8_t wake_cmd[] = {0x6B, 0x00}; 
    i2c_write_blocking(IMU_I2C_PORT, IMU_ADDRESS, wake_cmd, 2, false);

    int16_t accel[3], gyro[3];
    int32_t temperature, pressure;

    while (true) {
        printf("\033[2J\033[H");
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(100);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        read_mpu6050(accel, gyro);
        read_bmp180(&temperature, &pressure);
        //print live values per tick
        printf("Accel X: %d | Y: %d | Z: %d | Temperature: %d | Pressure: %d\n", accel[0], accel[1], accel[2], temperature, pressure);
        sleep_ms(100); 
    }
}