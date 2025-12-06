/**
 * @file adc.h
 * @brief Simple ADC Driver for IR Sensors
 * @author HoangAnh
 * @date 2025-01-30
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "stm32f4xx.h"

/**
 * @brief Initialize ADC1
 */
void ADC_Init(void);

/**
 * @brief Read ADC value from a channel
 * @param channel: ADC channel (0-15)
 * @return ADC value (0-4095)
 */
uint16_t ADC_Read(uint8_t channel);

#endif /* ADC_H */
