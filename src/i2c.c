/**
 * @file i2c.c
 * @brief I2C Driver - FIXED PINOUT + Non-blocking
 */

#include "i2c.h"
#include "system_timer.h"
#include <stdint.h>  /* ? FIX: Thêm dòng này d? có uint8_t */

/* Base addresses */
#define I2C1_BASE   0x40005400UL
#define I2C2_BASE   0x40005800UL
#define I2C3_BASE   0x40005C00UL

/* ? FIX: KHÔNG d?nh nghia l?i RCC_BASE n?u dã có */
#ifndef RCC_BASE
#define RCC_BASE    0x40023800UL
#endif

#define GPIOB_BASE  0x40020400UL

/* RCC registers */
#define RCC_APB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_AHB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x30))

/* GPIO registers */
#define GPIOB_MODER   (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_OTYPER  (*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_OSPEEDR (*(volatile uint32_t *)(GPIOB_BASE + 0x08))
#define GPIOB_PUPDR   (*(volatile uint32_t *)(GPIOB_BASE + 0x0C))
#define GPIOB_AFRL    (*(volatile uint32_t *)(GPIOB_BASE + 0x20))
#define GPIOB_AFRH    (*(volatile uint32_t *)(GPIOB_BASE + 0x24))

/* I2C register offsets */
#define I2C_CR1     0x00
#define I2C_CR2     0x04
#define I2C_OAR1    0x08
#define I2C_OAR2    0x0C
#define I2C_DR      0x10
#define I2C_SR1     0x14
#define I2C_SR2     0x18
#define I2C_CCR     0x1C
#define I2C_TRISE   0x20

/* I2C CR1 bits */
#define I2C_CR1_PE      (1 << 0)
#define I2C_CR1_START   (1 << 8)
#define I2C_CR1_STOP    (1 << 9)
#define I2C_CR1_ACK     (1 << 10)
#define I2C_CR1_SWRST   (1 << 15)

/* I2C SR1 bits */
#define I2C_SR1_SB      (1 << 0)
#define I2C_SR1_ADDR    (1 << 1)
#define I2C_SR1_BTF     (1 << 2)
#define I2C_SR1_TXE     (1 << 7)
#define I2C_SR1_RXNE    (1 << 6)
#define I2C_SR1_AF      (1 << 10)

#define I2C_TIMEOUT_MS  100

static uint32_t I2C_GetBase(I2C_Instance instance) {
    switch(instance) {
        case I2C_1: return I2C1_BASE;
        case I2C_2: return I2C2_BASE;
        case I2C_3: return I2C3_BASE;
        default: return I2C1_BASE;
    }
}

void I2C_Init(I2C_Instance instance, uint8_t fast_mode) {
    uint32_t i2c_base = I2C_GetBase(instance);
    
    /* Enable clocks */
    RCC_AHB1ENR |= (1 << 1);  // GPIOB clock
    
    if(instance == I2C_1) {
        RCC_APB1ENR |= (1 << 21);  // I2C1 clock
        
        /* ? FIX: PB8 (SCL), PB9 (SDA) cho I2C1 */
        /* Alternate function mode (AF4) */
        GPIOB_MODER &= ~((3UL << 16) | (3UL << 18));  // Clear PB8, PB9
        GPIOB_MODER |= (2UL << 16) | (2UL << 18);     // AF mode
        
        /* Open-drain output */
        GPIOB_OTYPER |= (1 << 8) | (1 << 9);
        
        /* High speed */
        GPIOB_OSPEEDR |= (3UL << 16) | (3UL << 18);
        
        /* Pull-up resistors */
        GPIOB_PUPDR &= ~((3UL << 16) | (3UL << 18));
        GPIOB_PUPDR |= (1UL << 16) | (1UL << 18);
        
        /* AF4 for I2C1 - PB8, PB9 in AFRH */
        GPIOB_AFRH &= ~((0xFUL << 0) | (0xFUL << 4));   // Clear
        GPIOB_AFRH |= (4UL << 0) | (4UL << 4);          // AF4
    }
    else {
        /* I2C2, I2C3 không kh? d?ng */
        return;
    }
    
    /* Reset I2C peripheral */
    (*(volatile uint32_t *)(i2c_base + I2C_CR1)) |= I2C_CR1_SWRST;
    delay_us_blocking(10);
    (*(volatile uint32_t *)(i2c_base + I2C_CR1)) &= ~I2C_CR1_SWRST;
    
    /* Configure I2C timing (APB1 = 50MHz) */
    (*(volatile uint32_t *)(i2c_base + I2C_CR2)) = 50;
    
    if(fast_mode) {
        /* Fast mode 400kHz */
        (*(volatile uint32_t *)(i2c_base + I2C_CCR)) = (1 << 15) | 42;
        (*(volatile uint32_t *)(i2c_base + I2C_TRISE)) = 16;
    } else {
        /* Standard mode 100kHz */
        (*(volatile uint32_t *)(i2c_base + I2C_CCR)) = 250;
        (*(volatile uint32_t *)(i2c_base + I2C_TRISE)) = 51;
    }
    
    /* Enable I2C */
    (*(volatile uint32_t *)(i2c_base + I2C_CR1)) |= I2C_CR1_PE;
}

static I2C_Status I2C_Start(uint32_t i2c_base) {
    uint32_t start_time = millis();
    
    /* Generate START */
    (*(volatile uint32_t *)(i2c_base + I2C_CR1)) |= I2C_CR1_START;
    
    /* Wait for SB flag */
    while(!(((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_SB))) {
        if(delay_elapsed_ms(start_time, I2C_TIMEOUT_MS)) {
            return I2C_TIMEOUT;
        }
    }
    
    return I2C_OK;
}

static void I2C_Stop(uint32_t i2c_base) {
    (*(volatile uint32_t *)(i2c_base + I2C_CR1)) |= I2C_CR1_STOP;
}

static I2C_Status I2C_SendAddress(uint32_t i2c_base, uint8_t addr) {
    uint32_t start_time = millis();
    
    /* Send address */
    (*(volatile uint32_t *)(i2c_base + I2C_DR)) = addr;
    
    /* Wait for ADDR flag */
    while(!(((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_ADDR))) {
        /* Check NACK */
        if((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_AF) {
            (*(volatile uint32_t *)(i2c_base + I2C_SR1)) &= ~I2C_SR1_AF;
            return I2C_NACK;
        }
        if(delay_elapsed_ms(start_time, I2C_TIMEOUT_MS)) {
            return I2C_TIMEOUT;
        }
    }
    
    /* Clear ADDR */
    (void)(*(volatile uint32_t *)(i2c_base + I2C_SR1));
    (void)(*(volatile uint32_t *)(i2c_base + I2C_SR2));
    
    return I2C_OK;
}

I2C_Status I2C_Write(I2C_Instance instance, uint8_t slave_addr, uint8_t *data, uint16_t len) {
    uint32_t i2c_base = I2C_GetBase(instance);
    uint32_t start_time;
    uint16_t i;
    
    if(I2C_Start(i2c_base) != I2C_OK) return I2C_TIMEOUT;
    
    if(I2C_SendAddress(i2c_base, slave_addr << 1) != I2C_OK) {
        I2C_Stop(i2c_base);
        return I2C_NACK;
    }
    
    for(i = 0; i < len; i++) {
        start_time = millis();
        while(!(((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_TXE))) {
            if(delay_elapsed_ms(start_time, I2C_TIMEOUT_MS)) {
                I2C_Stop(i2c_base);
                return I2C_TIMEOUT;
            }
        }
        (*(volatile uint32_t *)(i2c_base + I2C_DR)) = data[i];
    }
    
    start_time = millis();
    while(!(((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_BTF))) {
        if(delay_elapsed_ms(start_time, I2C_TIMEOUT_MS)) {
            I2C_Stop(i2c_base);
            return I2C_TIMEOUT;
        }
    }
    
    I2C_Stop(i2c_base);
    return I2C_OK;
}

I2C_Status I2C_Read(I2C_Instance instance, uint8_t slave_addr, uint8_t *data, uint16_t len) {
    uint32_t i2c_base = I2C_GetBase(instance);
    uint32_t start_time;
    uint16_t i;
    
    if(I2C_Start(i2c_base) != I2C_OK) return I2C_TIMEOUT;
    
    (*(volatile uint32_t *)(i2c_base + I2C_CR1)) |= I2C_CR1_ACK;
    
    if(I2C_SendAddress(i2c_base, (slave_addr << 1) | 1) != I2C_OK) {
        I2C_Stop(i2c_base);
        return I2C_NACK;
    }
    
    for(i = 0; i < len; i++) {
        if(i == len - 1) {
            (*(volatile uint32_t *)(i2c_base + I2C_CR1)) &= ~I2C_CR1_ACK;
        }
        
        start_time = millis();
        while(!(((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_RXNE))) {
            if(delay_elapsed_ms(start_time, I2C_TIMEOUT_MS)) {
                I2C_Stop(i2c_base);
                return I2C_TIMEOUT;
            }
        }
        
        data[i] = (*(volatile uint32_t *)(i2c_base + I2C_DR));
    }
    
    I2C_Stop(i2c_base);
    return I2C_OK;
}

I2C_Status I2C_WriteRegister(I2C_Instance instance, uint8_t slave_addr, uint8_t reg, uint8_t value) {
    uint8_t data[2];
    data[0] = reg;
    data[1] = value;
    return I2C_Write(instance, slave_addr, data, 2);
}

I2C_Status I2C_ReadRegister(I2C_Instance instance, uint8_t slave_addr, uint8_t reg, uint8_t *value) {
    uint32_t i2c_base = I2C_GetBase(instance);
    uint32_t start_time;
    
    if(I2C_Start(i2c_base) != I2C_OK) return I2C_TIMEOUT;
    
    if(I2C_SendAddress(i2c_base, slave_addr << 1) != I2C_OK) {
        I2C_Stop(i2c_base);
        return I2C_NACK;
    }
    
    start_time = millis();
    while(!(((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_TXE))) {
        if(delay_elapsed_ms(start_time, I2C_TIMEOUT_MS)) {
            I2C_Stop(i2c_base);
            return I2C_TIMEOUT;
        }
    }
    (*(volatile uint32_t *)(i2c_base + I2C_DR)) = reg;
    
    start_time = millis();
    while(!(((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_BTF))) {
        if(delay_elapsed_ms(start_time, I2C_TIMEOUT_MS)) {
            I2C_Stop(i2c_base);
            return I2C_TIMEOUT;
        }
    }
    
    return I2C_Read(instance, slave_addr, value, 1);
}

I2C_Status I2C_ReadRegisters(I2C_Instance instance, uint8_t slave_addr, uint8_t reg, uint8_t *data, uint16_t len) {
    uint32_t i2c_base = I2C_GetBase(instance);
    uint32_t start_time;
    
    if(I2C_Start(i2c_base) != I2C_OK) return I2C_TIMEOUT;
    
    if(I2C_SendAddress(i2c_base, slave_addr << 1) != I2C_OK) {
        I2C_Stop(i2c_base);
        return I2C_NACK;
    }
    
    start_time = millis();
    while(!(((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_TXE))) {
        if(delay_elapsed_ms(start_time, I2C_TIMEOUT_MS)) {
            I2C_Stop(i2c_base);
            return I2C_TIMEOUT;
        }
    }
    (*(volatile uint32_t *)(i2c_base + I2C_DR)) = reg;
    
    start_time = millis();
    while(!(((*(volatile uint32_t *)(i2c_base + I2C_SR1)) & I2C_SR1_BTF))) {
        if(delay_elapsed_ms(start_time, I2C_TIMEOUT_MS)) {
            I2C_Stop(i2c_base);
            return I2C_TIMEOUT;
        }
    }
    
    return I2C_Read(instance, slave_addr, data, len);
}

I2C_Status I2C_IsDeviceReady(I2C_Instance instance, uint8_t slave_addr) {
    uint32_t i2c_base = I2C_GetBase(instance);
    I2C_Status status;
    
    if(I2C_Start(i2c_base) != I2C_OK) return I2C_TIMEOUT;
    status = I2C_SendAddress(i2c_base, slave_addr << 1);
    I2C_Stop(i2c_base);
    
    return status;
}

