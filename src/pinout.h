/**
 * @file pinout.h
 * @brief Micromouse Robot - Pin Configuration (FINAL ROUTING READY FOR PCB)
 * @author HoangAnhNguyenHuu & Gemini
 * @date 2025-Final
 * * Hardware: STM32F411CEU6 (Black Pill)
 * * --- CRITICAL NOTES ---
 * 1. JTAG: Must DISABLE JTAG (SWD Only) in main code to use PA15, PB3, PB4.
 * 2. PB2 (IR_TX): Must have 10k Pull-down resistor to GND.
 * 3. PC14/PC15: Left floating for Crystal stability.
 * 4. SENSOR OPTIMIZATION: Pins remapped for best physical routing cluster (PA1, PA4-PA7, PB0).
 */

#ifndef PINOUT_H
#define PINOUT_H

#include <stdint.h>
#include "stm32f4xx.h"

/* ============================================================================ */
/* TIMER & MOTOR SYSTEM (TIM2)                     */
/* ============================================================================ */

/* Motor Right (Motor A) */
#define MOTOR_R_PWM_PORT        GPIOA
#define MOTOR_R_PWM_PIN         15          /* PA15 - TIM2_CH1 (JTAG Pin) */
#define MOTOR_R_PWM_AF          1
#define MOTOR_R_PWM_TIMER       TIM2
#define MOTOR_R_PWM_CHANNEL     1

#define MOTOR_R_IN1_PORT        GPIOB
#define MOTOR_R_IN1_PIN         12          /* PB12 */

#define MOTOR_R_IN2_PORT        GPIOB
#define MOTOR_R_IN2_PIN         13          /* PB13 */

/* Motor Left (Motor B) */
#define MOTOR_L_PWM_PORT        GPIOB
#define MOTOR_L_PWM_PIN         3           /* PB3 - TIM2_CH2 (JTAG Pin) */
#define MOTOR_L_PWM_AF          1
#define MOTOR_L_PWM_TIMER       TIM2
#define MOTOR_L_PWM_CHANNEL     2

#define MOTOR_L_IN1_PORT        GPIOB
#define MOTOR_L_IN1_PIN         14          /* PB14 */

#define MOTOR_L_IN2_PORT        GPIOB
#define MOTOR_L_IN2_PIN         15          /* PB15 */

/* PWM Configuration */
#define MOTOR_PWM_FREQUENCY     20000
#define MOTOR_PWM_PRESCALER     0
#define MOTOR_PWM_PERIOD        4999

/* ============================================================================ */
/* AUX PWM (FAN / BUZZER)                             */
/* ============================================================================ */

#define AUX_PWM_PORT            GPIOB
#define AUX_PWM_PIN             11          /* PB11 - TIM2_CH4 (Aux PWM) */
#define AUX_PWM_AF              1
#define AUX_PWM_TIMER           TIM2
#define AUX_PWM_CHANNEL         4

/* ============================================================================ */
/* ENCODERS - N20 GA12 (Quadrature)                            */
/* ============================================================================ */

/* Encoder Right - TIM3 (PB4/PB5) */
#define ENCODER_R_CHA_PORT      GPIOB       
#define ENCODER_R_CHA_PIN       4           /* PB4 - TIM3_CH1 (JTAG Pin) */
#define ENCODER_R_CHA_AF        2

#define ENCODER_R_CHB_PORT      GPIOB       
#define ENCODER_R_CHB_PIN       5           /* PB5 - TIM3_CH2 */
#define ENCODER_R_CHB_AF        2

#define ENCODER_R_TIMER         TIM3
#define ENCODER_R_RESOLUTION    2800

/* Encoder Left - TIM4 (PB6/PB7) */
#define ENCODER_L_CHA_PORT      GPIOB
#define ENCODER_L_CHA_PIN       6           /* PB6 - TIM4_CH1 */
#define ENCODER_L_CHA_AF        2

#define ENCODER_L_CHB_PORT      GPIOB
#define ENCODER_L_CHB_PIN       7           /* PB7 - TIM4_CH2 */
#define ENCODER_L_CHB_AF        2

#define ENCODER_L_TIMER         TIM4
#define ENCODER_L_RESOLUTION    2800

/* ============================================================================ */
/* IR DISTANCE SENSORS (6 RX + 6 TX) - CLUSTER LI?N M?CH          */
/* ============================================================================ */

/* --- IR RECEIVERS (ADC1 Inputs) - C?m ADC li?n m?ch 6 chân --- */
#define IR_RX_L0_PORT           GPIOA
#define IR_RX_L0_PIN            1           /* PA1 - ADC1_IN1 */
#define IR_RX_L0_ADC_CHANNEL    1

#define IR_RX_L45_PORT          GPIOA
#define IR_RX_L45_PIN           4           /* PA4 - ADC1_IN4 */
#define IR_RX_L45_ADC_CHANNEL   4

#define IR_RX_L90_PORT          GPIOA       
#define IR_RX_L90_PIN           5           /* PA5 - ADC1_IN5 */
#define IR_RX_L90_ADC_CHANNEL   5

#define IR_RX_R0_PORT           GPIOA       
#define IR_RX_R0_PIN            6           /* PA6 - ADC1_IN6 */
#define IR_RX_R0_ADC_CHANNEL    6           

#define IR_RX_R45_PORT          GPIOA       
#define IR_RX_R45_PIN           7           /* PA7 - ADC1_IN7 */
#define IR_RX_R45_ADC_CHANNEL   7           

#define IR_RX_R90_PORT          GPIOB       
#define IR_RX_R90_PIN           0           /* PB0 - ADC1_IN8 */
#define IR_RX_R90_ADC_CHANNEL   8


/* --- IR EMITTERS (GPIO Output) - S?p x?p li?n m?ch GPIO --- */
#define IR_LED_L0_PORT          GPIOA
#define IR_LED_L0_PIN           8           /* PA8 */

#define IR_LED_L45_PORT         GPIOA
#define IR_LED_L45_PIN          11          /* PA11 */

#define IR_LED_L90_PORT         GPIOA
#define IR_LED_L90_PIN          12          /* PA12 */

#define IR_LED_R0_PORT          GPIOB
#define IR_LED_R0_PIN           1           /* PB1 */

#define IR_LED_R45_PORT         GPIOB
#define IR_LED_R45_PIN          2           /* PB2 (BOOT1 - Requires 10k Pull-down) */

#define IR_LED_R90_PORT         GPIOB
#define IR_LED_R90_PIN          10          /* PB10 */


typedef enum {
    IR_L0 = 0,
    IR_L45 = 1,
    IR_L90 = 2,
    IR_R0 = 3,
    IR_R45 = 4,
    IR_R90 = 5
} IR_Sensor_t;

#define IR_SENSOR_COUNT 6

/* ============================================================================ */
/* I2C1 - OLED + MPU6050                                       */
/* ============================================================================ */

#define I2C_SCL_PORT            GPIOB
#define I2C_SCL_PIN             8           /* PB8 - I2C1_SCL */
#define I2C_SCL_AF              4

#define I2C_SDA_PORT            GPIOB
#define I2C_SDA_PIN             9           /* PB9 - I2C1_SDA */
#define I2C_SDA_AF              4

#define MPU6050_I2C_ADDR        0x68
#define OLED_I2C_ADDR           0x3C

/* ============================================================================ */
/* USART1 - Debug Serial (USB-TTL)                             */
/* ============================================================================ */

#define DEBUG_TX_PORT           GPIOA
#define DEBUG_TX_PIN            9           /* PA9 - USART1_TX */
#define DEBUG_TX_AF             7

#define DEBUG_RX_PORT           GPIOA
#define DEBUG_RX_PIN            10          /* PA10 - USART1_RX */
#define DEBUG_RX_AF             7

#define DEBUG_BAUDRATE          115200

/* ============================================================================ */
/* USART2 - Bluetooth HC-05                                    */
/* ============================================================================ */

#define BT_TX_PORT              GPIOA
#define BT_TX_PIN               2           /* PA2 - USART2_TX */
#define BT_TX_AF                7

#define BT_RX_PORT              GPIOA
#define BT_RX_PIN               3           /* PA3 - USART2_RX */
#define BT_RX_AF                7

#define BT_BAUDRATE             9600

/* ============================================================================ */
/* BUTTONS (Only 1 Left)                                       */
/* ============================================================================ */

#define BTN_KEY_PORT            GPIOA
#define BTN_KEY_PIN             0           /* PA0 - Onboard KEY */

#define BUTTON_PRESSED          0
#define BUTTON_RELEASED         1

/* ============================================================================ */
/* BUZZER / FAN (PB11) & LED (PC13)                            */
/* ============================================================================ */

#define BUZZER_PORT             GPIOB
#define BUZZER_PIN              11          /* PB11 (PWM) */

#define LED_STATUS_PORT         GPIOC
#define LED_STATUS_PIN          13          /* PC13 */

/* ============================================================================ */
/* ST-LINK SWD (RESERVED)                                      */
/* ============================================================================ */

#define SWDIO_PORT              GPIOA
#define SWDIO_PIN               13          /* PA13 - RESERVED */

#define SWCLK_PORT              GPIOA
#define SWCLK_PIN               14          /* PA14 - RESERVED */

/* ============================================================================ */
/* HELPER MACROS                                               */
/* ============================================================================ */

#define GPIO_SET_PIN(port, pin)     ((port)->BSRRL = (1UL << (pin)))
#define GPIO_RESET_PIN(port, pin)   ((port)->BSRRH = (1UL << (pin)))
#define GPIO_TOGGLE_PIN(port, pin)  ((port)->ODR ^= (1UL << (pin)))
#define GPIO_READ_PIN(port, pin)    (((port)->IDR & (1UL << (pin))) ? 1 : 0)

#define MOTOR_R_IN1_HIGH()      GPIO_SET_PIN(MOTOR_R_IN1_PORT, MOTOR_R_IN1_PIN)
#define MOTOR_R_IN1_LOW()       GPIO_RESET_PIN(MOTOR_R_IN1_PORT, MOTOR_R_IN1_PIN)
#define MOTOR_R_IN2_HIGH()      GPIO_SET_PIN(MOTOR_R_IN2_PORT, MOTOR_R_IN2_PIN)
#define MOTOR_R_IN2_LOW()       GPIO_RESET_PIN(MOTOR_R_IN2_PORT, MOTOR_R_IN2_PIN)

#define MOTOR_L_IN1_HIGH()      GPIO_SET_PIN(MOTOR_L_IN1_PORT, MOTOR_L_IN1_PIN)
#define MOTOR_L_IN1_LOW()       GPIO_RESET_PIN(MOTOR_L_IN1_PORT, MOTOR_L_IN1_PIN)
#define MOTOR_L_IN2_HIGH()      GPIO_SET_PIN(MOTOR_L_IN2_PORT, MOTOR_L_IN2_PIN)
#define MOTOR_L_IN2_LOW()       GPIO_RESET_PIN(MOTOR_L_IN2_PORT, MOTOR_L_IN2_PIN)

#define IR_LED_ON(port, pin)    GPIO_SET_PIN(port, pin)
#define IR_LED_OFF(port, pin)   GPIO_RESET_PIN(port, pin)

// Buzzer Macros (Digital ON/OFF control on PB11)
#define BUZZER_ENABLE()         GPIO_SET_PIN(BUZZER_PORT, BUZZER_PIN)
#define BUZZER_DISABLE()        GPIO_RESET_PIN(BUZZER_PORT, BUZZER_PIN)

#define READ_BTN_KEY()          GPIO_READ_PIN(BTN_KEY_PORT, BTN_KEY_PIN)

// LED Status Macros
#define LED_ON()                GPIO_RESET_PIN(LED_STATUS_PORT, LED_STATUS_PIN) // Active Low
#define LED_OFF()               GPIO_SET_PIN(LED_STATUS_PORT, LED_STATUS_PIN)
#define LED_TOGGLE()            GPIO_TOGGLE_PIN(LED_STATUS_PORT, LED_STATUS_PIN)

// Helper cho Buzzer/Fan (Ghi vào thanh ghi CCR4 c?a TIM2)
#define SET_AUX_PWM(val)        (TIM2->CCR4 = (val))

/* ============================================================================ */
/* CONFIGURATION SUMMARY                                       */
/* ============================================================================ */

/*
 * FINAL CONFIGURATION (OPTIMIZED):
 * * SENSOR GROUPING: 6 RX (PA1, PA4-A7, PB0) + 6 TX (PA8, PA11, PA12, PB1, PB2, PB10).
 * * Ðây là s? s?p x?p t?i uu nh?t v? m?t v?t lý d? di dây ra Sensor Array.
 * * NOTES:
 * - JTAG must be disabled in software.
 * - PB2 needs 10k pull-down resistor.
 */

#endif /* PINOUT_H */