#pragma once

// GP0/1 radio
#define PIN_SDA_BMP180 2
#define PIN_SCL_BMP180 3
#define PIN_SDA_MPU6050 4
#define PIN_SCL_MPU6050 5
#define PIN_INT1_MPU6050 6
#define PIN_INT2_MPU6050 7
#define PIN_PWM 8
#define PIN_LED 9
//GP10-13 SPI1 radio
//GP14/15 Radio IRQ/Reset
#define PIN_SD_MISO 16
#define PIN_SD_CS 17
#define PIN_SD_SCK 18
#define PIN_SD_MOSI 19
#define RECOV_CH1 20
#define RECOV_CH2 21
#define REG_MODE_SEL 23
#define VBUS_DETECT 24
#define BOARD_LED 25
#define PIN_PYRO1_CONT 26 //A0
#define PIN_PYRO2_CONT 27 //A1
#define PIN_PYRO_BAT_SNS 28 //A2

//I2C addresses
#define MPU6050_ADDR 0x68// Default I2C address for MPU6050
#define BMP280_ADDR  0x76// Default I2C address for BMP280 barometer

//other
#define SAMPLING_RATE_HZ 60 //60 hz should be enough