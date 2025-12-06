/**
 * @file system_timer.h
 * @brief Non-blocking System Timer using TIM5 (32-bit, 1us resolution)
 * @author HELLO12312E
 * @date 2025-01-29
 */

#ifndef SYSTEM_TIMER_H
#define SYSTEM_TIMER_H

#include <stdint.h>

/* KHONG dinh nghia lai neu da co tu CMSIS */
#ifndef TIM5
/* TIM5 Base Address */
#define TIM5_BASE   0x40000C00UL

/* TIM5 Registers */
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    uint32_t RESERVED;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
} TIM5_TypeDef;

#define TIM5 ((TIM5_TypeDef *) TIM5_BASE)
#endif /* TIM5 */

#ifndef RCC_BASE
/* RCC Registers */
#define RCC_BASE    0x40023800UL
#endif

#ifndef RCC_APB1ENR_TIM5EN
#define RCC_APB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x40))
#define RCC_APB1ENR_TIM5EN (1 << 3)
#endif

/* Function Prototypes */

/**
 * @brief Initialize system timer (TIM5 at 1MHz = 1us tick)
 */
void SystemTimer_Init(void);

/**
 * @brief Get current microseconds count
 * @return Current time in microseconds (32-bit, wraps every ~71 minutes)
 */
uint32_t micros(void);

/**
 * @brief Get current milliseconds count
 * @return Current time in milliseconds
 */
uint32_t millis(void);

/**
 * @brief Non-blocking delay check (microseconds)
 * @param start_time: Start timestamp (from micros())
 * @param delay: Delay duration in microseconds
 * @return 1 if delay elapsed, 0 otherwise
 */
__inline static uint8_t delay_elapsed_us(uint32_t start_time, uint32_t delay) {
    return (micros() - start_time) >= delay;
}

/**
 * @brief Non-blocking delay check (milliseconds)
 * @param start_time: Start timestamp (from millis())
 * @param delay: Delay duration in milliseconds
 * @return 1 if delay elapsed, 0 otherwise
 */
__inline static uint8_t delay_elapsed_ms(uint32_t start_time, uint32_t delay) {
    return (millis() - start_time) >= delay;
}

/**
 * @brief Blocking delay in microseconds (ONLY for critical initialization)
 * @param us: Microseconds to delay
 */
void delay_us_blocking(uint32_t us);

/**
 * @brief Blocking delay in milliseconds (ONLY for critical initialization)
 * @param ms: Milliseconds to delay
 */
void delay_ms_blocking(uint32_t ms);

#endif /* SYSTEM_TIMER_H */
