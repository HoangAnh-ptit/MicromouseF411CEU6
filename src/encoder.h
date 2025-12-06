/**
 * @file encoder.h
 * @brief Quadrature Encoder Driver for STM32F411CEU6
 * @author HELLO12312E
 * @date 2025-01-29
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include "stm32f4xx.h"  /* ? Use CMSIS only */
#include "pinout.h"

/* Encoder Selection */
typedef enum {
    ENCODER_RIGHT = 0,  /* TIM3 - PA6, PA7 */
    ENCODER_LEFT = 1    /* TIM4 - PB6, PB7 */
} Encoder_Select_t;

/* Encoder Handle */
typedef struct {
    TIM_TypeDef *timer;         /* Timer peripheral */
    uint16_t resolution;        /* Counts per revolution */
    int32_t total_count;        /* Total count (with overflow) */
    int32_t last_count;         /* Last raw count */
    uint32_t last_update_time;  /* Last speed update time (ms) */
    float speed_rpm;            /* Current speed in RPM */
} Encoder_Handle_t;

/* Function Prototypes */

void Encoder_Init(Encoder_Handle_t *handle, Encoder_Select_t encoder, uint16_t resolution);
int16_t Encoder_GetCount(Encoder_Handle_t *handle);
int32_t Encoder_GetTotalCount(Encoder_Handle_t *handle);
void Encoder_Reset(Encoder_Handle_t *handle);
void Encoder_UpdateSpeed(Encoder_Handle_t *handle);
float Encoder_GetSpeedRPM(Encoder_Handle_t *handle);
float Encoder_GetSpeedCPS(Encoder_Handle_t *handle);

#endif /* ENCODER_H */
