/**
 * @file main.c
 * @brief MPU6050 Yaw Angle Reading Demo
 * @author HoangAnh
 * @date 2025
 */

#include "stm32f4xx.h"
#include "hardware.h"
#include "uart.h"
#include "system_timer.h"
#include "i2c.h"
#include "mpu6050.h"
#include <stdio.h>
#include <stdlib.h>

/* MPU6050 Handle */
static MPU6050_Handle mpu;

int main(void) {
    /* --- KHAI BAO BIEN --- */
    uint32_t last_update_time = 0;
    uint32_t last_print_time = 0;
    char buffer[64];
    float yaw_angle = 0.0f;
    float dt = 0.01f;  /* 10ms = 0.01s */
    MPU6050_Status status;
    MPU6050_Data sensor_data;

    /* --- KHOI TAO PHAN CUNG --- */
    Hardware_Init();
    UART_Init();
    
    UART_SendString("\r\n=== MPU6050 Yaw Reading ===\r\n");
    
    /* Khoi tao I2C1 (Fast Mode 400kHz) */
    I2C_Init(I2C_1, 1);
    delay_ms_blocking(100);
    
    /* Khoi tao MPU6050 */
    UART_SendString("Initializing MPU6050...\r\n");
    status = MPU6050_Init(&mpu);
    
    if (status != MPU6050_OK) {
        UART_SendString("ERROR: MPU6050 not found!\r\n");
        while(1) {
            LED_TOGGLE();
            delay_ms_blocking(200);
        }
    }
    
    UART_SendString("MPU6050 OK!\r\n");
    LED_ON();
    
    /* Calibrate Gyroscope - Giu yen robot trong 2 giay */
    UART_SendString("Calibrating...  Keep robot still!\r\n");
    Hardware_Beep(100);
    delay_ms_blocking(500);
    
    MPU6050_Calibrate(&mpu, 200);  /* 200 samples */
    
    UART_SendString("Calibration done!\r\n");
    Hardware_Beep(50);
    delay_ms_blocking(100);
    Hardware_Beep(50);
    
    /* Reset Yaw ve 0 */
    MPU6050_ResetYaw(&mpu);
    
    UART_SendString("Format: Yaw,GyroZ,AccelX,AccelY,AccelZ\r\n");
    delay_ms_blocking(500);

    /* === VONG LAP CHINH === */
    while (1) {
        /* --- CAP NHAT YAW (100Hz = 10ms) --- */
        if (millis() - last_update_time >= 10) {
            last_update_time = millis();
            
            /* Cap nhat goc Yaw */
            MPU6050_UpdateYaw(&mpu, dt);
            
            /* Lay gia tri Yaw hien tai */
            yaw_angle = MPU6050_GetYaw(&mpu);
        }
        
        /* --- IN RA SERIAL (20Hz = 50ms) --- */
        if (millis() - last_print_time >= 50) {
            last_print_time = millis();
            
            /* Doc du lieu day du tu MPU6050 */
            if (MPU6050_ReadProcessed(&mpu, &sensor_data) == MPU6050_OK) {
                /* In: Yaw, GyroZ, AccelX, AccelY, AccelZ */
                sprintf(buffer, "%. 1f,%.1f,%.2f,%.2f,%.2f\r\n", 
                        yaw_angle,
                        sensor_data.gyro_z,
                        sensor_data.accel_x,
                        sensor_data. accel_y,
                        sensor_data.accel_z);
                UART_SendString(buffer);
            }
        }
        
        /* --- XU LY NUT NHAN --- */
        if (Hardware_ReadButton(0) == 1) {
            /* Nhan nut KEY -> Reset Yaw ve 0 */
            MPU6050_ResetYaw(&mpu);
            UART_SendString("Yaw Reset!\r\n");
            Hardware_Beep(50);
            delay_ms_blocking(200);  /* Debounce */
        }
    }
}
