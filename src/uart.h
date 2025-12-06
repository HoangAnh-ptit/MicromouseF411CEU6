/* uart.h - Register Level cho STM32F411 */
#ifndef __UART_H
#define __UART_H

#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>

/* Cau hinh UART */
#define UART_BAUDRATE 115200

/* Prototype functions */
void UART_GPIO_Config(void);
void UART_Init(void);
void UART_SendChar(char c);
void UART_SendString(char *str);
void UART_SendNumber(uint32_t num);
char UART_ReceiveChar(void);
uint8_t UART_Available(void);

#endif /* __UART_H */
