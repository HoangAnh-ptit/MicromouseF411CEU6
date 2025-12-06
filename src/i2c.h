// i2c.h
#ifndef I2C_H
#define I2C_H

#include <stdint.h>

// I2C Status codes
typedef enum {
    I2C_OK = 0,
    I2C_ERROR,
    I2C_BUSY,
    I2C_TIMEOUT,
    I2C_NACK
} I2C_Status;

// I2C Instance
typedef enum {
    I2C_1 = 0,
    I2C_2,
    I2C_3
} I2C_Instance;

// Kh?i t?o I2C v?i t?c d? chu?n (100kHz) ho?c fast mode (400kHz)
void I2C_Init(I2C_Instance instance, uint8_t fast_mode);

// Ghi d? li?u d?n thi?t b? slave
I2C_Status I2C_Write(I2C_Instance instance, uint8_t slave_addr, uint8_t *data, uint16_t len);

// Ð?c d? li?u t? thi?t b? slave
I2C_Status I2C_Read(I2C_Instance instance, uint8_t slave_addr, uint8_t *data, uint16_t len);

// Ghi thanh ghi c?a thi?t b? slave
I2C_Status I2C_WriteRegister(I2C_Instance instance, uint8_t slave_addr, uint8_t reg, uint8_t value);

// Ð?c thanh ghi c?a thi?t b? slave
I2C_Status I2C_ReadRegister(I2C_Instance instance, uint8_t slave_addr, uint8_t reg, uint8_t *value);

// Ð?c nhi?u byte t? thanh ghi
I2C_Status I2C_ReadRegisters(I2C_Instance instance, uint8_t slave_addr, uint8_t reg, uint8_t *data, uint16_t len);

// Ki?m tra thi?t b? có s?n trên bus không
I2C_Status I2C_IsDeviceReady(I2C_Instance instance, uint8_t slave_addr);

#endif // I2C_H
