#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "include.hpp"
#include "config.hpp"
#include "ff.h"

int imu_data;
int baro_data;

FATFS fs;
FIL fil;

int16_t ac1, ac2, ac3;
uint16_t ac4, ac5, ac6;
int16_t b1, b2, mb, mc, md;

void read_mpu6050(int16_t accel[3], int16_t gyro[3]) {
    uint8_t buffer[14];
    uint8_t val = 0x3B; // starting register for accelerometer data
    i2c_write_blocking(IMU_I2C_PORT, IMU_ADDRESS, &val, 1, true);
    i2c_read_blocking(IMU_I2C_PORT, IMU_ADDRESS, buffer, 14, false);
    
    accel[0] = (buffer[0] << 8) | buffer[1];  // x
    accel[1] = (buffer[2] << 8) | buffer[3];  // y
    accel[2] = (buffer[4] << 8) | buffer[5];  // z

    gyro[0]  = (buffer[8] << 8) | buffer[9];  // x
    gyro[1]  = (buffer[10] << 8) | buffer[11]; // y
    gyro[2]  = (buffer[12] << 8) | buffer[13]; // z
}

void read_bmp180(int32_t *temperature, int32_t *pressure) {
    uint8_t buffer[3];
    
    // read temp
    uint8_t temp_cmd[2] = {0xF4, 0x2E};
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, temp_cmd, 2, false);
    sleep_ms(15);

    uint8_t reg_addr = 0xF6;
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, &reg_addr, 1, true);
    i2c_read_blocking(BARO_I2C_PORT, BARO_ADDRESS, buffer, 2, false);
    *temperature = (buffer[0] << 8) | buffer[1];

    // read pressure
    uint8_t press_cmd[2] = {0xF4, 0x34};
    i2c_write_blocking(BARO_I2C_PORT, BARO_ADDRESS, press_cmd, 2, false);
    sleep_ms(15);

    reg_addr = 0xF6;
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
    int32_t x1 = ((ut - (int32_t)ac6) * (int32_t)ac5) >> 15;
    int32_t x2 = ((int32_t)mc << 11) / (x1 + (int32_t)md);
    int32_t b5 = x1 + x2;
    *temperature = ((b5 + 8) >> 4) / 10.0f;

    int32_t b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11;
    x2 = (ac2 * b6) >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = (((((int32_t)ac1) * 4 + x3) << 0) + 2) >> 2;

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
    
    *pressure = (float)p;
    *altitude = 44330.0f * (1.0f - powf(*pressure / QNH, 0.1903f));
}

void init_sd_card() {
    FRESULT fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("Error mounting SD card %d\n", fr);
    } else {
        printf("SD Card mounted successfully\n");
    }
}

void write_sensor_log(float time_s, int16_t accel[3], int16_t gyro[3], float angles[3], float temp, float pressure, float altitude) {
    FIL fil;
    FRESULT fr = f_open(&fil, "0:sensor_log.csv", FA_OPEN_APPEND | FA_WRITE);
    
    if (fr == FR_OK) {
        UINT bytes_written;
        char buffer[250];
        
        float ax_ms2 = ((float)accel[0] / 16384.0f) * 9.80665f;
        float ay_ms2 = ((float)accel[1] / 16384.0f) * 9.80665f;
        float az_ms2 = ((float)accel[2] / 16384.0f) * 9.80665f;
        
        float gx_dps = (float)gyro[0] / 131.0f;
        float gy_dps = (float)gyro[1] / 131.0f;
        float gz_dps = (float)gyro[2] / 131.0f;
        
        int length = snprintf(buffer, sizeof(buffer), 
                              "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n", 
                              time_s, 
                              ax_ms2, ay_ms2, az_ms2, 
                              gx_dps, gy_dps, gz_dps, 
                              angles[0], angles[1], angles[2], 
                              temp, pressure, altitude);
                              
        f_write(&fil, buffer, length, &bytes_written);
        f_close(&fil);
    } else {
        printf("DEBUG: Failed to open log file, error code: %d\n", fr);
    }
}

void init_csv_header() {
    FIL fil;
    FRESULT fr = f_open(&fil, "0:sensor_log.csv", FA_OPEN_ALWAYS | FA_WRITE);
    if (fr == FR_OK) {
        if (f_size(&fil) == 0) {
            UINT bytes_written;
            const char *header = "Time_s,Accel_X_ms2,Accel_Y_ms2,Accel_Z_ms2,Gyro_X_dps,Gyro_Y_dps,Gyro_Z_dps,Angle_X_deg,Angle_Y_deg,Angle_Z_deg,Temperature,Pressure,Altitude\n";
            f_write(&fil, header, strlen(header), &bytes_written);
            printf("CSV header written successfully.\n");
        }
        f_close(&fil);
    } else {
        printf("Failed to initialize CSV header, error: %d\n", fr);
    }
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

    gpio_set_function(SD_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_MISO_PIN, GPIO_FUNC_SPI);
    spi_init(SD_SPI_PORT, 1000 * 1000);
    gpio_init(SD_CS_PIN);
    gpio_set_dir(SD_CS_PIN, GPIO_OUT);
    gpio_put(SD_CS_PIN, 1);

    uint8_t wake_cmd[] = {0x6B, 0x00}; 
    i2c_write_blocking(IMU_I2C_PORT, IMU_ADDRESS, wake_cmd, 2, false);

    init_sd_card();
    init_csv_header();
    read_bmp180_calibration();

    int16_t accel[3], gyro[3];
    int32_t raw_temp, raw_press;
    float angles[3] = {0.0f, 0.0f, 0.0f};
    uint32_t last_time_ms = to_ms_since_boot(get_absolute_time());

    sleep_ms(500);

    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(50);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);

        // read sensor
        read_mpu6050(accel, gyro);

        float temp_c, pressure_pa, altitude_m;
        read_bmp180(&raw_temp, &raw_press);
        calculate_bmp180_values(raw_temp, raw_press, &temp_c, &pressure_pa, &altitude_m);

        // calc time/dt
        uint32_t current_time_ms = to_ms_since_boot(get_absolute_time());
        float time_s = (float)current_time_ms / 1000.0f;
        
        float dt = (float)(current_time_ms - last_time_ms) / 1000.0f;
        last_time_ms = current_time_ms;

        // gyro rate integration to get angles
        angles[0] += ((float)gyro[0] / 131.0f) * dt;
        angles[1] += ((float)gyro[1] / 131.0f) * dt;
        angles[2] += ((float)gyro[2] / 131.0f) * dt;

        // log onto sd
        write_sensor_log(time_s, accel, gyro, angles, temp_c, pressure_pa, altitude_m);
        printf("Accel X: %d | Y: %d | Z: %d | Temperature: %.2f | Pressure: %.2f | Altitude: %.2f\n", accel[0], accel[1], accel[2], temp_c, pressure_pa, altitude_m);   
        sleep_ms(50); 
    }
}
