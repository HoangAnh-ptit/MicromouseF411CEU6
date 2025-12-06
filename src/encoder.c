/**
 * @file encoder.c
 * @brief Encoder Driver Implementation
 */

#include "encoder.h"
#include "system_timer.h"

/**
 * @brief Configure GPIO for encoder input
 */
static void Encoder_GPIO_Init(Encoder_Select_t encoder) {
    if (encoder == ENCODER_RIGHT) {
        /* Enable GPIOB clock */
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
        
        /* PB4, PB5 - Alternate Function mode */
        GPIOB->MODER &= ~((3UL << 8) | (3UL << 10));
        GPIOB->MODER |= (2UL << 8) | (2UL << 10);
        
        /* High speed */
        GPIOB->OSPEEDR |= (3UL << 8) | (3UL << 10);
        
        /* Pull-up */
        GPIOB->PUPDR &= ~((3UL << 8) | (3UL << 10));
        GPIOB->PUPDR |= (1UL << 8) | (1UL << 10);
        
        /* AF2 = TIM3 */
        GPIOB->AFR[0] &= ~((0xFUL << 16) | (0xFUL << 20));
        GPIOB->AFR[0] |= (2UL << 16) | (2UL << 20);
    }
    else {
        /* Enable GPIOB clock */
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
        
        /* PB6, PB7 - Alternate Function mode */
        GPIOB->MODER &= ~((3UL << 12) | (3UL << 14));
        GPIOB->MODER |= (2UL << 12) | (2UL << 14);
        
        /* High speed */
        GPIOB->OSPEEDR |= (3UL << 12) | (3UL << 14);
        
        /* Pull-up */
        GPIOB->PUPDR &= ~((3UL << 12) | (3UL << 14));
        GPIOB->PUPDR |= (1UL << 12) | (1UL << 14);
        
        /* AF2 = TIM4 */
        GPIOB->AFR[0] &= ~((0xFUL << 24) | (0xFUL << 28));
        GPIOB->AFR[0] |= (2UL << 24) | (2UL << 28);
    }
}

/**
 * @brief Initialize encoder
 */
void Encoder_Init(Encoder_Handle_t *handle, Encoder_Select_t encoder, uint16_t resolution) {
    /* Set parameters */
    handle->resolution = resolution;
    handle->total_count = 0;
    handle->last_count = 0;
    handle->last_update_time = millis();
    handle->speed_rpm = 0.0f;
    
    /* Select timer */
    if (encoder == ENCODER_RIGHT) {
        handle->timer = TIM3;
        /* Enable TIM3 clock */
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    } else {
        handle->timer = TIM4;
        /* Enable TIM4 clock */
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    }
    
    /* Configure GPIO */
    Encoder_GPIO_Init(encoder);
    
    /* Disable timer */
    handle->timer->CR1 &= ~(1 << 0);
    
    /* Set timer to Encoder Mode 3 (count both edges on both channels) */
    /* SMS = 011 (Encoder mode 3) */
    handle->timer->SMCR &= ~(7UL << 0);
    handle->timer->SMCR |= (3UL << 0);
    
    /* Configure input capture */
    /* CC1S = 01 (TI1 mapped on IC1) */
    /* CC2S = 01 (TI2 mapped on IC2) */
    handle->timer->CCMR1 &= ~((3UL << 0) | (3UL << 8));
    handle->timer->CCMR1 |= (1UL << 0) | (1UL << 8);
    
    /* No input filter */
    handle->timer->CCMR1 &= ~((0xFUL << 4) | (0xFUL << 12));
    
    /* Enable capture */
    handle->timer->CCER |= (1UL << 0) | (1UL << 4);
    
    /* Set ARR to max (0xFFFF for 16-bit counter) */
    handle->timer->ARR = 0xFFFF;
    
    /* Reset counter */
    handle->timer->CNT = 0;
    
    /* Generate update event */
    handle->timer->EGR = (1 << 0);
    
    /* Clear flags */
    handle->timer->SR = 0;
    
    /* Enable timer */
    handle->timer->CR1 |= (1 << 0);
}

/**
 * @brief Get current encoder count (raw)
 */
int16_t Encoder_GetCount(Encoder_Handle_t *handle) {
    return (int16_t)(handle->timer->CNT);
}

/**
 * @brief Get total encoder count (with overflow handling)
 */
int32_t Encoder_GetTotalCount(Encoder_Handle_t *handle) {
    int16_t current_count;
    int16_t delta;
    
    current_count = Encoder_GetCount(handle);
    delta = current_count - (int16_t)handle->last_count;
    
    handle->total_count += delta;
    handle->last_count = current_count;
    
    return handle->total_count;
}

/**
 * @brief Reset encoder count
 */
void Encoder_Reset(Encoder_Handle_t *handle) {
    handle->timer->CNT = 0;
    handle->total_count = 0;
    handle->last_count = 0;
}

/**
 * @brief Update speed calculation
 */
void Encoder_UpdateSpeed(Encoder_Handle_t *handle) {
    uint32_t current_time;
    uint32_t delta_time;
    int32_t current_total;
    int32_t delta_count;
    float counts_per_sec;
    
    current_time = millis();
    delta_time = current_time - handle->last_update_time;
    
    if (delta_time == 0) return;
    
    current_total = Encoder_GetTotalCount(handle);
    delta_count = current_total - handle->last_count;
    
    /* Calculate counts per second */
    counts_per_sec = (float)delta_count * 1000.0f / (float)delta_time;
    
    /* Convert to RPM */
    handle->speed_rpm = (counts_per_sec * 60.0f) / (float)handle->resolution;
    
    /* Update for next calculation */
    handle->last_count = current_total;
    handle->last_update_time = current_time;
}

/**
 * @brief Get speed in RPM
 */
float Encoder_GetSpeedRPM(Encoder_Handle_t *handle) {
    return handle->speed_rpm;
}

/**
 * @brief Get speed in counts/second
 */
float Encoder_GetSpeedCPS(Encoder_Handle_t *handle) {
    return handle->speed_rpm * (float)handle->resolution / 60.0f;
}

