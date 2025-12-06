/* uart.c - Register Level cho STM32F411 */
#include "uart.h"

/**
 * @brief Cau hinh GPIO PA9 (TX), PA10 (RX) cho USART1
 */
void UART_GPIO_Config(void) {
    /* Bat clock cho GPIOA */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    
    /* PA9 (TX) va PA10 (RX) - Alternate Function mode (10) */
    GPIOA->MODER &= ~((3UL << 18) | (3UL << 20));  /* Clear bit */
    GPIOA->MODER |= (2UL << 18) | (2UL << 20);     /* AF mode */
    
    /* Output type: Push-pull (0) */
    GPIOA->OTYPER &= ~((1UL << 9) | (1UL << 10));
    
    /* Speed: Very high (11) */
    GPIOA->OSPEEDR |= (3UL << 18) | (3UL << 20);
    
    /* Pull-up (01) */
    GPIOA->PUPDR &= ~((3UL << 18) | (3UL << 20));
    GPIOA->PUPDR |= (1UL << 18) | (1UL << 20);
    
    /* Alternate Function 7 (USART1) cho PA9 va PA10 */
    GPIOA->AFR[1] &= ~((0xFUL << 4) | (0xFUL << 8));  /* Clear AF */
    GPIOA->AFR[1] |= (7UL << 4) | (7UL << 8);         /* AF7 */
}

/**
 * @brief Lay tan so APB2 Clock (USART1 nam tren APB2)
 */
uint32_t Get_APB2_Clock(void) {
    uint32_t SystemCoreClock;
    uint32_t tmp;
    uint32_t pllm, plln, pllp;
    uint32_t ahbprescaler, apb2prescaler;
    
    /* Kiem tra nguon clock hien tai */
    tmp = RCC->CFGR & RCC_CFGR_SWS;
    
    if (tmp == RCC_CFGR_SWS_HSI) {
        /* Dang dung HSI (16MHz) */
        SystemCoreClock = 16000000;
    }
    else if (tmp == RCC_CFGR_SWS_HSE) {
        /* Dang dung HSE (25MHz hoac 8MHz) */
        SystemCoreClock = 25000000;  /* Gia su 25MHz */
    }
    else if (tmp == RCC_CFGR_SWS_PLL) {
        /* Dang dung PLL - Tinh toan chinh xac */
        pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
        plln = (RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6;
        pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> 16) + 1) * 2;
        
        if (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) {
            /* PLL source = HSE */
            SystemCoreClock = (25000000 / pllm) * plln / pllp;
        } else {
            /* PLL source = HSI */
            SystemCoreClock = (16000000 / pllm) * plln / pllp;
        }
    }
    else {
        SystemCoreClock = 16000000;  /* Mac dinh HSI */
    }
    
    /* Tinh AHB prescaler */
    tmp = (RCC->CFGR & RCC_CFGR_HPRE) >> 4;
    if (tmp & 0x08) {
        ahbprescaler = 1 << ((tmp & 0x07) + 1);
    } else {
        ahbprescaler = 1;
    }
    
    /* Tinh APB2 prescaler */
    tmp = (RCC->CFGR & RCC_CFGR_PPRE2) >> 13;
    if (tmp & 0x04) {
        apb2prescaler = 1 << ((tmp & 0x03) + 1);
    } else {
        apb2prescaler = 1;
    }
    
    return (SystemCoreClock / ahbprescaler / apb2prescaler);
}

/**
 * @brief Khoi tao USART1: 115200 baud, 8-N-1
 */
void UART_Init(void) {
    uint32_t apb2_clock;
    uint32_t brr_value;
    
    /* Bat clock cho USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    
    /* Cau hinh GPIO truoc */
    UART_GPIO_Config();
    
    /* Disable USART truoc khi cau hinh */
    USART1->CR1 &= ~USART_CR1_UE;
    
    /* 8 data bits (M=0) */
    USART1->CR1 &= ~USART_CR1_M;
    
    /* No parity (PCE=0) */
    USART1->CR1 &= ~USART_CR1_PCE;
    
    /* 1 stop bit (STOP=00) */
    USART1->CR2 &= ~USART_CR2_STOP;
    
    /* Tu dong tinh BRR dua vao APB2 Clock */
    apb2_clock = Get_APB2_Clock();
    brr_value = apb2_clock / UART_BAUDRATE;
    USART1->BRR = brr_value;
    
    /* Enable TX va RX */
    USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
    
    /* Enable USART */
    USART1->CR1 |= USART_CR1_UE;
}

/**
 * @brief Gui 1 ky tu qua UART
 */
void UART_SendChar(char c) {
    /* Doi TXE (Transmit Data Register Empty) = 1 */
    while (!(USART1->SR & USART_SR_TXE));
    
    /* Ghi du lieu vao Data Register */
    USART1->DR = c;
}

/**
 * @brief Gui chuoi ky tu qua UART
 */
void UART_SendString(char *str) {
    while (*str) {
        UART_SendChar(*str++);
    }
}

/**
 * @brief Gui so nguyen qua UART
 */
void UART_SendNumber(uint32_t num) {
    char buffer[12];
    sprintf(buffer, "%lu", num);
    UART_SendString(buffer);
}

/**
 * @brief Nhan 1 ky tu tu UART (blocking)
 */
char UART_ReceiveChar(void) {
    /* Doi RXNE (Read Data Register Not Empty) = 1 */
    while (!(USART1->SR & USART_SR_RXNE));
    
    /* Doc du lieu tu Data Register */
    return (char)(USART1->DR);
}

/**
 * @brief Kiem tra co du lieu nhan khong
 * @return 1 neu co du lieu, 0 neu khong
 */
uint8_t UART_Available(void) {
    return (USART1->SR & USART_SR_RXNE) ? 1 : 0;
}
