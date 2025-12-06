#include "jdy33.h"
#include "stm32f4xx.h"
#include <string.h>

static void JDY33_GPIO_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    // PA2 (TX)
    GPIOA->MODER &= ~(3 << (2*2));
    GPIOA->MODER |= (2 << (2*2));
    GPIOA->AFR[0] &= ~(0xF << (4*2));
    GPIOA->AFR[0] |= (7 << (4*2));
    GPIOA->OSPEEDR |= (3 << (2*2));
    GPIOA->PUPDR &= ~(3 << (2*2));
    // PA3 (RX)
    GPIOA->MODER &= ~(3 << (2*3));
    GPIOA->MODER |= (2 << (2*3));
    GPIOA->AFR[0] &= ~(0xF << (4*3));
    GPIOA->AFR[0] |= (7 << (4*3));
    GPIOA->OSPEEDR |= (3 << (2*3));
    GPIOA->PUPDR |= (1 << (2*3));
}

void JDY33_Init(JDY33_Handle_t *handle, uint32_t baud) {
    if (!handle) return;
    memset(handle, 0, sizeof(JDY33_Handle_t));
    JDY33_GPIO_Init();
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    JDY33_UART->CR1 &= ~USART_CR1_UE;
    JDY33_UART->CR1 &= ~(USART_CR1_M | USART_CR1_PCE);
    JDY33_UART->CR2 &= ~USART_CR2_STOP;
    JDY33_UART->BRR = 50000000 / baud;
    JDY33_UART->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_SetPriority(USART2_IRQn, 2);
    JDY33_UART->CR1 |= USART_CR1_UE;
    handle->initialized = 1;
}

void JDY33_SetBaudrate(uint32_t baud) {
    JDY33_UART->CR1 &= ~USART_CR1_UE;
    JDY33_UART->BRR = 50000000 / baud;
    JDY33_UART->CR1 |= USART_CR1_UE;
}

void JDY33_SendString(JDY33_Handle_t *handle, const char *str) {
    if (!handle || !str) return;
    while (*str) {
        while(!(JDY33_UART->SR & USART_SR_TXE));
        JDY33_UART->DR = *str++;
    }
}

uint16_t JDY33_Available(JDY33_Handle_t *handle) {
    if (!handle) return 0;
    if (handle->rx_head >= handle->rx_tail) {
        return handle->rx_head - handle->rx_tail;
    } else {
        return JDY33_RX_BUFFER_SIZE - handle->rx_tail + handle->rx_head;
    }
}

uint8_t JDY33_ReadByte(JDY33_Handle_t *handle) {
    uint8_t byte = 0;
    if (!handle || JDY33_Available(handle) == 0) return 0;
    byte = handle->rx_buffer[handle->rx_tail];
    handle->rx_tail = (handle->rx_tail + 1) % JDY33_RX_BUFFER_SIZE;
    return byte;
}

uint16_t JDY33_ReadLine(JDY33_Handle_t *handle, char *buffer, uint16_t max_len) {
    uint16_t count = 0;
    uint8_t byte;
    if (!handle || !buffer) return 0;
    while (JDY33_Available(handle) > 0 && count < max_len - 1) {
        byte = JDY33_ReadByte(handle);
        if (byte == '\n' || byte == '\r') {
            if (count > 0) {
                buffer[count] = '\0';
                return count;
            }
        } else {
            buffer[count++] = byte;
        }
    }
    if (count > 0) buffer[count] = '\0';
    return count;
}

void JDY33_IRQHandler(JDY33_Handle_t *handle) {
    uint8_t byte;
    if (!handle) return;
    if (JDY33_UART->SR & USART_SR_RXNE) {
        byte = JDY33_UART->DR;
        handle->rx_buffer[handle->rx_head] = byte;
        handle->rx_head = (handle->rx_head + 1) % JDY33_RX_BUFFER_SIZE;
    }
}
