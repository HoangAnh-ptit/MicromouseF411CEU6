/**
 * @file mpu6050.h
 * @brief MPU6050 IMU Driver for Micromouse Navigation
 * @author HELLO12312E
 * @date 2025-01-28
 * 
 * Features:
 * - Yaw angle calculation for maze navigation
 * - Auto calibration on startup
 * - Drift compensation
 * - Non-blocking read option
 */

#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>
#include "i2c.h"

/* ============ CONFIGURATION ============ */

/** MPU6050 I2C Address */
#define MPU6050_ADDR            0x68

/** I2C Instance (I2C_1, I2C_2, I2C_3) */
#define MPU6050_I2C_INSTANCE    I2C_1

/** Gyroscope Range (0-3 for ±250, ±500, ±1000, ±2000 deg/s) */
#define MPU6050_GYRO_RANGE      0  // ±250 deg/s

/** Sample Rate (Hz) */
#define MPU6050_SAMPLE_RATE     200

/** Digital Low Pass Filter Bandwidth (Hz) */
#define MPU6050_DLPF_BW         20

/** Gyro Deadzone (deg/s) - for drift reduction */
#define MPU6050_GYRO_DEADZONE   0.5f

/* ============ DATA STRUCTURES ============ */

/**
 * @brief MPU6050 Status
 */
typedef enum {
    MPU6050_OK = 0,
    MPU6050_ERROR,
    MPU6050_TIMEOUT,
    MPU6050_NOT_FOUND
} MPU6050_Status;

/**
 * @brief MPU6050 Raw Data
 */
typedef struct {
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
} MPU6050_RawData;

/**
 * @brief MPU6050 Processed Data
 */
typedef struct {
    float gyro_x;    // deg/s
    float gyro_y;    // deg/s
    float gyro_z;    // deg/s
    float accel_x;   // g
    float accel_y;   // g
    float accel_z;   // g
    float temp;      // °C
} MPU6050_Data;

/**
 * @brief MPU6050 Handle Structure
 */
typedef struct {
    I2C_Instance i2c;
    uint8_t address;
    float gyro_scale;      // LSB per deg/s
    float accel_scale;     // LSB per g
    float gyro_z_offset;   // Calibration offset
    float yaw;             // Current yaw angle (0-360°)
    uint32_t last_update;  // Timestamp (ms)
} MPU6050_Handle;

/* ============ FUNCTION PROTOTYPES ============ */

/**
 * @brief Initialize MPU6050
 * @param handle: Pointer to MPU6050 handle
 * @return MPU6050_Status
 */
MPU6050_Status MPU6050_Init(MPU6050_Handle *handle);

/**
 * @brief Calibrate gyroscope (must keep sensor still)
 * @param handle: Pointer to MPU6050 handle
 * @param samples: Number of calibration samples (default 200)
 * @return MPU6050_Status
 */
MPU6050_Status MPU6050_Calibrate(MPU6050_Handle *handle, uint16_t samples);

/**
 * @brief Read raw gyroscope Z-axis
 * @param handle: Pointer to MPU6050 handle
 * @return Raw gyro Z value
 */
int16_t MPU6050_ReadGyroZ(MPU6050_Handle *handle);

/**
 * @brief Read all raw sensor data
 * @param handle: Pointer to MPU6050 handle
 * @param data: Pointer to raw data structure
 * @return MPU6050_Status
 */
MPU6050_Status MPU6050_ReadRaw(MPU6050_Handle *handle, MPU6050_RawData *data);

/**
 * @brief Read processed sensor data
 * @param handle: Pointer to MPU6050 handle
 * @param data: Pointer to processed data structure
 * @return MPU6050_Status
 */
MPU6050_Status MPU6050_ReadProcessed(MPU6050_Handle *handle, MPU6050_Data *data);

/**
 * @brief Update yaw angle (call periodically)
 * @param handle: Pointer to MPU6050 handle
 * @param dt: Time step in seconds (e.g., 0.005 for 200Hz)
 * @return MPU6050_Status
 */
MPU6050_Status MPU6050_UpdateYaw(MPU6050_Handle *handle, float dt);

/**
 * @brief Get current yaw angle
 * @param handle: Pointer to MPU6050 handle
 * @return Yaw angle (0-360°)
 */
float MPU6050_GetYaw(MPU6050_Handle *handle);

/**
 * @brief Reset yaw angle to zero
 * @param handle: Pointer to MPU6050 handle
 */
void MPU6050_ResetYaw(MPU6050_Handle *handle);

/**
 * @brief Set yaw angle to specific value
 * @param handle: Pointer to MPU6050 handle
 * @param angle: Target angle (0-360°)
 */
void MPU6050_SetYaw(MPU6050_Handle *handle, float angle);

/**
 * @brief Check if device is ready
 * @param handle: Pointer to MPU6050 handle
 * @return MPU6050_Status
 */
MPU6050_Status MPU6050_IsReady(MPU6050_Handle *handle);

#endif /* MPU6050_H */
