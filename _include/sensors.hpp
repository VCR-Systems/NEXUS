#pragma once

#pragma once
#include <stdint.h> // Required for standard integer types like uint32_t

struct TelemetryPacket {
    uint32_t timestamp_ms; // Time since boot in milliseconds
    float accel_x, accel_y, accel_z; // Accelerometer data in G's
    float gyro_x, gyro_y, gyro_z;    // Gyroscope data in degrees/second
    float pressure;                  // Barometric pressure
    float temperature;               // Temperature from barometer
    float altitude;                  // Calculated altitude in meters
};

void init_sensors();

void read_mpu6050(float &ax, float &ay, float &az, 
                  float &gx, float &gy, float &gz);

void read_barometer(float &pressure, 
                    float &temperature, 
                    float &altitude);
