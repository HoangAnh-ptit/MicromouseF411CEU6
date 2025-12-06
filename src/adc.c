/**
 * @file adc.c
 * @brief Simple ADC Driver Implementation
 * @author HoangAnh
 * @date 2025-01-30
 */

#include "adc.h"

void ADC_Init(void) {
    volatile int i;
    
    /* B?t clock cho ADC1 */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    
    /* Ð?m b?o GPIOA và GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    
    /* T?t ADC tru?c khi c?u hình */
    ADC1->CR2 = 0;
    ADC1->CR1 = 0;
    
    /* Sample time: 84 cycles cho T?T C? channels (0b100) */
    ADC1->SMPR2 = 0x04924924;  /* Channels 0-9 */
    ADC1->SMPR1 = 0x04924924;  /* Channels 10-18 */
    
    /* Ch? convert 1 channel m?i l?n */
    ADC1->SQR1 = 0;
    
    /* B?t ADC */
    ADC1->CR2 |= ADC_CR2_ADON;
    
    /* Ð?i ADC ?n d?nh (~10µs) */
    for(i = 0; i < 10000; i++);
}

uint16_t ADC_Read(uint8_t channel) {
    /* Ch?n channel (SQ1) */
    ADC1->SQR3 = channel;
    
    /* B?t d?u conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;
    
    /* Ch? conversion hoàn t?t */
    while(!(ADC1->SR & ADC_SR_EOC));
    
    /* Ð?c và tr? v? giá tr? */
    return ADC1->DR;
}
