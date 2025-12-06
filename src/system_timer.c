/**
 * @file system_timer.c
 * @brief System Timer Implementation with Auto Clock Detection
 * @author HELLO12312E
 * @date 2025-01-29
 */

#include "system_timer.h"

/* RCC Registers for clock detection */
#define RCC_CFGR    (*(volatile uint32_t *)(RCC_BASE + 0x08))
#define RCC_PLLCFGR (*(volatile uint32_t *)(RCC_BASE + 0x04))

/* RCC CFGR bits */
#define RCC_CFGR_SWS_Pos    2
#define RCC_CFGR_SWS_Msk    (3UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_SWS_HSI    (0UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_SWS_HSE    (1UL << RCC_CFGR_SWS_Pos)
#define RCC_CFGR_SWS_PLL    (2UL << RCC_CFGR_SWS_Pos)

#define RCC_CFGR_PPRE1_Pos  10
#define RCC_CFGR_PPRE1_Msk  (7UL << RCC_CFGR_PPRE1_Pos)

/* PLL Configuration bits */
#define RCC_PLLCFGR_PLLM_Msk    0x3F
#define RCC_PLLCFGR_PLLN_Pos    6
#define RCC_PLLCFGR_PLLN_Msk    (0x1FF << RCC_PLLCFGR_PLLN_Pos)
#define RCC_PLLCFGR_PLLP_Pos    16
#define RCC_PLLCFGR_PLLP_Msk    (3UL << RCC_PLLCFGR_PLLP_Pos)
#define RCC_PLLCFGR_PLLSRC      (1UL << 22)

/**
 * @brief Get System Core Clock (SYSCLK)
 */
static uint32_t GetSystemClock(void) {
    uint32_t sysclk;
    uint32_t pllm, plln, pllp;
    uint32_t pllsource;
    uint32_t sws;
    
    /* Check current clock source */
    sws = RCC_CFGR & RCC_CFGR_SWS_Msk;
    
    if (sws == RCC_CFGR_SWS_HSI) {
        /* HSI (16MHz) */
        sysclk = 16000000UL;
    }
    else if (sws == RCC_CFGR_SWS_HSE) {
        /* HSE (usually 25MHz or 8MHz) */
        sysclk = 25000000UL;  /* Adjust if different */
    }
    else if (sws == RCC_CFGR_SWS_PLL) {
        /* PLL as system clock */
        pllm = RCC_PLLCFGR & RCC_PLLCFGR_PLLM_Msk;
        plln = (RCC_PLLCFGR & RCC_PLLCFGR_PLLN_Msk) >> RCC_PLLCFGR_PLLN_Pos;
        pllp = (((RCC_PLLCFGR & RCC_PLLCFGR_PLLP_Msk) >> RCC_PLLCFGR_PLLP_Pos) + 1) * 2;
        
        /* PLL source: HSI or HSE */
        pllsource = (RCC_PLLCFGR & RCC_PLLCFGR_PLLSRC) ? 25000000UL : 16000000UL;
        
        /* SYSCLK = PLL_source * (PLLN / PLLM) / PLLP */
        sysclk = (pllsource / pllm) * plln / pllp;
    }
    else {
        sysclk = 16000000UL;  /* Default HSI */
    }
    
    return sysclk;
}

/**
 * @brief Get APB1 Timer Clock
 */
static uint32_t GetAPB1TimerClock(void) {
    uint32_t sysclk;
    uint32_t hpre, ppre1;
    uint32_t ahb_prescaler;
    uint32_t apb1_prescaler;
    uint32_t apb1_clock;
    
    /* ? FIX: Khai báo t?t c? bi?n ? d?u hàm (C89) */
    sysclk = GetSystemClock();
    ahb_prescaler = 1;
    apb1_prescaler = 1;
    
    /* AHB Prescaler */
    hpre = (RCC_CFGR >> 4) & 0xF;
    if (hpre & 0x8) {
        ahb_prescaler = 1 << ((hpre & 0x7) + 1);
    }
    
    /* APB1 Prescaler */
    ppre1 = (RCC_CFGR & RCC_CFGR_PPRE1_Msk) >> RCC_CFGR_PPRE1_Pos;
    if (ppre1 & 0x4) {
        apb1_prescaler = 1 << ((ppre1 & 0x3) + 1);
    }
    
    /* APB1 clock */
    apb1_clock = sysclk / ahb_prescaler / apb1_prescaler;
    
    /* Timer clock = APB1 × 2 if APB1 prescaler != 1 */
    if (apb1_prescaler != 1) {
        return apb1_clock * 2;
    }
    return apb1_clock;
}

/**
 * @brief Initialize TIM5 as system timer (1MHz = 1us tick)
 */
void SystemTimer_Init(void) {
    uint32_t tim_clock;
    uint32_t prescaler;
    
    /* Enable TIM5 clock */
    RCC_APB1ENR |= RCC_APB1ENR_TIM5EN;
    
    /* Get actual TIM5 clock frequency */
    tim_clock = GetAPB1TimerClock();
    
    /* Calculate prescaler for 1MHz (1us tick) */
    prescaler = (tim_clock / 1000000UL) - 1;
    
    /* Disable counter */
    TIM5->CR1 &= ~(1 << 0);  /* CEN = 0 */
    
    /* Set prescaler */
    TIM5->PSC = prescaler;
    
    /* Set auto-reload to max (32-bit) */
    TIM5->ARR = 0xFFFFFFFF;
    
    /* Reset counter */
    TIM5->CNT = 0;
    
    /* Generate update event */
    TIM5->EGR = (1 << 0);  /* UG = 1 */
    
    /* Clear flags */
    TIM5->SR = 0;
    
    /* Enable counter */
    TIM5->CR1 |= (1 << 0);  /* CEN = 1 */
}

/**
 * @brief Get current microseconds
 */
uint32_t micros(void) {
    return TIM5->CNT;
}

/**
 * @brief Get current milliseconds
 */
uint32_t millis(void) {
    return TIM5->CNT / 1000UL;
}

/**
 * @brief Blocking delay in microseconds
 * @warning ONLY use for critical initialization!
 */
void delay_us_blocking(uint32_t us) {
    uint32_t start = micros();
    while((micros() - start) < us);
}

/**
 * @brief Blocking delay in milliseconds
 * @warning ONLY use for critical initialization!
 */
void delay_ms_blocking(uint32_t ms) {
    delay_us_blocking(ms * 1000UL);
}
