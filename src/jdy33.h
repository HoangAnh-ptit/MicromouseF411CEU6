#ifndef JDY33_H
#define JDY33_H

#include <stdint.h>
#include "stm32f4xx.h"

// S? d?ng USART2 (PA2 TX - PA3 RX)
#define JDY33_UART              USART2
#define JDY33_RX_BUFFER_SIZE    256

typedef struct {
    uint8_t rx_buffer[JDY33_RX_BUFFER_SIZE];
    volatile uint16_t rx_head;
    volatile uint16_t rx_tail;
    uint8_t initialized;
} JDY33_Handle_t;

// Kh?i t?o module JDY-33 BLE (PA2/PA3)
void JDY33_Init(JDY33_Handle_t *handle, uint32_t baud);

// G?i chu?i qua BLE
void JDY33_SendString(JDY33_Handle_t *handle, const char *str);

// Ki?m tra s? byte có th? d?c t? BLE
uint16_t JDY33_Available(JDY33_Handle_t *handle);

// Ð?c 1 byte t? BLE
uint8_t JDY33_ReadByte(JDY33_Handle_t *handle);

// Ð?c 1 dòng (k?t thúc b?i \n ho?c \r)
uint16_t JDY33_ReadLine(JDY33_Handle_t *handle, char *buffer, uint16_t max_len);

// Ð?i baudrate USART2 runtime
void JDY33_SetBaudrate(uint32_t baud);

// IRQ Handler - g?i trong USART2_IRQHandler
void JDY33_IRQHandler(JDY33_Handle_t *handle);

#endif
