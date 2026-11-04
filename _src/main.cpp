#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "include.hpp"

int imu_data;
int baro_data;

int16_t ac1, ac2, ac3;
uint16_t ac4, ac5, ac6;
int16_t b1, b2, mb, mc, md;

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
    
    // --- 1. Read Temperature ---
    uint8_t temp_cmd[2] = {0xF4, 0x2E}; // Register 0xF4, Command 0x2E
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, temp_cmd, 2, false);
    sleep_ms(5); // Wait for conversion

    uint8_t reg_addr = 0xF6; // Data register
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &reg_addr, 1, true);
    i2c_read_blocking(BARO_I2C_PORT, BARO_ADDRESS, buffer, 2, false);
    *temperature = (buffer[0] << 8) | buffer[1];

    // --- 2. Read Pressure ---
    uint8_t press_cmd[2] = {0xF4, 0x34}; // Register 0xF4, Command 0x34 (OSS = 0)
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, press_cmd, 2, false);
    sleep_ms(5); // Wait for conversion

    reg_addr = 0xF6; // Data register
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &reg_addr, 1, true);
    i2c_read_blocking(BARO_I2C_PORT, BARO_ADDRESS, buffer, 3, false);
    
    *pressure = ((buffer[0] << 16) | (buffer[1] << 8) | buffer[2]) >> 8;
}

void read_bmp180_calibration() {
    uint8_t coeff_reg = 0xAA;
    uint8_t cal_data[22];

    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &coeff_reg, 1, true);
    i2c_read_blocking(BARO_I2C_PORT, BARO_ADDRESS, cal_data, 22, false);

    ac1 = (cal_data[0] << 8) | cal_data[1];
    ac2 = (cal_data[2] << 8) | cal_data[3];
    ac3 = (cal_data[4] << 8) | cal_data[5];
    ac4 = (cal_data[6] << 8) | cal_data[7];
    ac5 = (cal_data[8] << 8) | cal_data[9];
    ac6 = (cal_data[10] << 8) | cal_data[11];
    b1  = (cal_data[12] << 8) | cal_data[13];
    b2  = (cal_data[14] << 8) | cal_data[15];
    mb  = (cal_data[16] << 8) | cal_data[17];
    mc  = (cal_data[18] << 8) | cal_data[19];
    md  = (cal_data[20] << 8) | cal_data[21];
}

void calculate_bmp180_values(int32_t ut, int32_t up, float *temperature, float *pressure, float *altitude) {
    // --- Temperature Calculation ---
    int32_t x1 = ((ut - (int32_t)ac6) * (int32_t)ac5) >> 15;
    int32_t x2 = ((int32_t)mc << 11) / (x1 + (int32_t)md);
    int32_t b5 = x1 + x2;
    *temperature = ((b5 + 8) >> 4) / 10.0f; // Temperature in deg C

    // --- Pressure Calculation ---
    int32_t b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11;
    x2 = (ac2 * b6) >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = (((((int32_t)ac1) * 4 + x3) << 0) + 2) >> 2; // (Using OSS = 0)

    x1 = (ac3 * b6) >> 13;
    x2 = (b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    uint32_t b4 = (ac4 * (uint32_t)(x3 + 32768)) >> 15;
    uint32_t b7 = ((uint32_t)up - b3) * (50000 >> 0);

    int32_t p;
    if (b7 < 0x80000000) {
        p = (b7 * 2) / b4;
    } else {
        p = (b7 / b4) * 2;
    }

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);
    
    *pressure = (float)p; // Pressure in Pascals (Pa)

    // --- Altitude Calculation ---
    // Standard sea-level pressure is 101325 Pa (1013.25 hPa)
    // You can adjust 101325 to your local barometric pressure for higher accuracy
    *altitude = 44330.0f * (1.0f - powf(*pressure / 102066.0f, 0.1903f));
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

    read_bmp180_calibration();

    int16_t accel[3], gyro[3];
    int32_t raw_temp, raw_press;

    read_bmp180_calibration();
    printf("AC1 calibration value: %d\n", ac1); // Check if this is non-zero

    while (true) {
        printf("\033[2J\033[H");
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(50);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        read_mpu6050(accel, gyro);

        float temp_c, pressure_pa, altitude_m;
        read_bmp180(&raw_temp, &raw_press);
        calculate_bmp180_values(raw_temp, raw_press, &temp_c, &pressure_pa, &altitude_m);

        printf("Accel X: %d | Y: %d | Z: %d | Temperature: %.2f | Pressure: %.2f | Altitude: %.2f\n", accel[0], accel[1], accel[2], temp_c, pressure_pa, altitude_m);
        printf("DEBUG: A1 Calibration Value: %d | ut Value: %d\n", ac1, raw_temp); // Debugging line to check calibration value
        sleep_ms(50); 
    }
}