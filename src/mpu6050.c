/**
 * @file mpu6050.c
 * @brief MPU6050 Driver - Non-blocking version
 */

#include "mpu6050.h"
#include "system_timer.h"
#include <math.h>

/* MPU6050 Registers */
#define MPU6050_WHO_AM_I    0x75
#define MPU6050_PWR_MGMT_1  0x6B
#define MPU6050_GYRO_CONFIG 0x1B
#define MPU6050_ACCEL_CONFIG 0x1C
#define MPU6050_SMPLRT_DIV  0x19
#define MPU6050_CONFIG      0x1A
#define MPU6050_GYRO_ZOUT_H 0x47
#define MPU6050_ACCEL_XOUT_H 0x3B

/* Gyro/Accel Scale Factors */
static const float GYRO_SCALE_250  = 131.0f;
static const float GYRO_SCALE_500  = 65.5f;
static const float GYRO_SCALE_1000 = 32.8f;
static const float GYRO_SCALE_2000 = 16.4f;
static const float ACCEL_SCALE_2G  = 16384.0f;

MPU6050_Status MPU6050_Init(MPU6050_Handle *handle) {
    uint8_t who_am_i;
    uint8_t sample_div;
    
    handle->i2c = MPU6050_I2C_INSTANCE;
    handle->address = MPU6050_ADDR;
    handle->yaw = 0.0f;
    handle->gyro_z_offset = 0.0f;
    handle->last_update = 0;
    
    /* Set gyro scale */
    switch(MPU6050_GYRO_RANGE) {
        case 0: handle->gyro_scale = GYRO_SCALE_250; break;
        case 1: handle->gyro_scale = GYRO_SCALE_500; break;
        case 2: handle->gyro_scale = GYRO_SCALE_1000; break;
        case 3: handle->gyro_scale = GYRO_SCALE_2000; break;
        default: handle->gyro_scale = GYRO_SCALE_250;
    }
    
    handle->accel_scale = ACCEL_SCALE_2G;
    
    /* Check device */
    if(I2C_IsDeviceReady(handle->i2c, handle->address) != I2C_OK) {
        return MPU6050_NOT_FOUND;
    }
    
    /* Read WHO_AM_I */
    I2C_ReadRegister(handle->i2c, handle->address, MPU6050_WHO_AM_I, &who_am_i);
    if(who_am_i != 0x68) {
        return MPU6050_ERROR;
    }
    
    /* Reset device */
    I2C_WriteRegister(handle->i2c, handle->address, MPU6050_PWR_MGMT_1, 0x80);
    delay_ms_blocking(100);
    
    /* Wake up */
    I2C_WriteRegister(handle->i2c, handle->address, MPU6050_PWR_MGMT_1, 0x03);
    delay_ms_blocking(100);
    
    /* Configure gyro */
    I2C_WriteRegister(handle->i2c, handle->address, MPU6050_GYRO_CONFIG, 
                      MPU6050_GYRO_RANGE << 3);
    
    /* Configure accel */
    I2C_WriteRegister(handle->i2c, handle->address, MPU6050_ACCEL_CONFIG, 0x00);
    
    /* Set sample rate */
    sample_div = (1000 / MPU6050_SAMPLE_RATE) - 1;
    I2C_WriteRegister(handle->i2c, handle->address, MPU6050_SMPLRT_DIV, sample_div);
    
    /* DLPF */
    I2C_WriteRegister(handle->i2c, handle->address, MPU6050_CONFIG, 0x04);
    
    return MPU6050_OK;
}

MPU6050_Status MPU6050_Calibrate(MPU6050_Handle *handle, uint16_t samples) {
    int16_t gz;
    int32_t gz_sum = 0;
    uint16_t i;
    uint32_t last_sample = millis();
    
    for(i = 0; i < samples; i++) {
        /* Non-blocking delay 10ms */
        while(!delay_elapsed_ms(last_sample, 10));
        last_sample = millis();
        
        gz = MPU6050_ReadGyroZ(handle);
        gz_sum += gz;
    }
    
    handle->gyro_z_offset = (float)gz_sum / samples / handle->gyro_scale;
    
    return MPU6050_OK;
}

int16_t MPU6050_ReadGyroZ(MPU6050_Handle *handle) {
    uint8_t data[2];
    int16_t gz = 0;
    
    if(I2C_ReadRegisters(handle->i2c, handle->address, 
                         MPU6050_GYRO_ZOUT_H, data, 2) == I2C_OK) {
        gz = (int16_t)((data[0] << 8) | data[1]);
    }
    
    return gz;
}

MPU6050_Status MPU6050_ReadRaw(MPU6050_Handle *handle, MPU6050_RawData *data) {
    uint8_t buffer[14];
    
    if(I2C_ReadRegisters(handle->i2c, handle->address, 
                         MPU6050_ACCEL_XOUT_H, buffer, 14) != I2C_OK) {
        return MPU6050_ERROR;
    }
    
    data->accel_x = (int16_t)((buffer[0] << 8) | buffer[1]);
    data->accel_y = (int16_t)((buffer[2] << 8) | buffer[3]);
    data->accel_z = (int16_t)((buffer[4] << 8) | buffer[5]);
    data->temp    = (int16_t)((buffer[6] << 8) | buffer[7]);
    data->gyro_x  = (int16_t)((buffer[8] << 8) | buffer[9]);
    data->gyro_y  = (int16_t)((buffer[10] << 8) | buffer[11]);
    data->gyro_z  = (int16_t)((buffer[12] << 8) | buffer[13]);
    
    return MPU6050_OK;
}

MPU6050_Status MPU6050_ReadProcessed(MPU6050_Handle *handle, MPU6050_Data *data) {
    MPU6050_RawData raw;
    
    if(MPU6050_ReadRaw(handle, &raw) != MPU6050_OK) {
        return MPU6050_ERROR;
    }
    
    data->gyro_x = raw.gyro_x / handle->gyro_scale;
    data->gyro_y = raw.gyro_y / handle->gyro_scale;
    data->gyro_z = raw.gyro_z / handle->gyro_scale;
    
    data->accel_x = raw.accel_x / handle->accel_scale;
    data->accel_y = raw.accel_y / handle->accel_scale;
    data->accel_z = raw.accel_z / handle->accel_scale;
    
    data->temp = raw.temp / 340.0f + 36.53f;
    
    return MPU6050_OK;
}

MPU6050_Status MPU6050_UpdateYaw(MPU6050_Handle *handle, float dt) {
    int16_t gz_raw = MPU6050_ReadGyroZ(handle);
    float gz_dps = (gz_raw / handle->gyro_scale) - handle->gyro_z_offset;
    
    /* Deadzone */
    if(fabs(gz_dps) < MPU6050_GYRO_DEADZONE) {
        gz_dps = 0.0f;
    }
    
    /* Integrate */
    handle->yaw += gz_dps * dt;
    
    /* Normalize */
    while(handle->yaw >= 360.0f) handle->yaw -= 360.0f;
    while(handle->yaw < 0.0f) handle->yaw += 360.0f;
    
    /* Update timestamp */
    handle->last_update = millis();
    
    return MPU6050_OK;
}

float MPU6050_GetYaw(MPU6050_Handle *handle) {
    return handle->yaw;
}

void MPU6050_ResetYaw(MPU6050_Handle *handle) {
    handle->yaw = 0.0f;
}

void MPU6050_SetYaw(MPU6050_Handle *handle, float angle) {
    handle->yaw = angle;
    while(handle->yaw >= 360.0f) handle->yaw -= 360.0f;
    while(handle->yaw < 0.0f) handle->yaw += 360.0f;
}

MPU6050_Status MPU6050_IsReady(MPU6050_Handle *handle) {
    if(I2C_IsDeviceReady(handle->i2c, handle->address) == I2C_OK) {
        return MPU6050_OK;
    }
    return MPU6050_NOT_FOUND;
}
