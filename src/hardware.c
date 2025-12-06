/**
 * @file hardware.c
 * @brief Hardware Abstraction Layer Implementation (Fixed for New Pinout)
 * @author HoangAnhNguyenHuu & Gemini
 * @date 2025-01-30
 * * FIXLOG:
 * - Removed TIM1 (Motor L moved to TIM2).
 * - Removed BTN_MODE (Pin PB11 used for PWM).
 * - Added TIM2_CH4 init for Buzzer/Fan.
 * - Handled JTAG pins (PA15, PB3, PB4) correctly.
 */

#include "hardware.h"
#include "tb6612fng.h"
#include "encoder.h"
#include "system_timer.h"
#include "uart.h"
#include "adc.h"
#include <stdio.h>

/* Private variables */
static TB6612_Handle_t motor_driver;
static Encoder_Handle_t encoder_right;
static Encoder_Handle_t encoder_left;

/* ============================================================================ */
/* CLOCK INITIALIZATION                                 */
/* ============================================================================ */

void Hardware_InitClocks(void) {
    /* 1. Enable GPIO clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    
    /* 2. Enable Timer clocks */
    // TIM1 Removed (Not used anymore)
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;  /* Motor R, Motor L, Fan/Buzzer */
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;  /* Encoder Right */
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;  /* Encoder Left */
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;  /* System timer */
}

/* ============================================================================ */
/* GPIO HELPER FUNCTIONS                                */
/* ============================================================================ */

void GPIO_ConfigOutput(GPIO_TypeDef *port, uint8_t pin) {
    port->MODER &= ~(3UL << (pin * 2));
    port->MODER |= (1UL << (pin * 2));
    port->OTYPER &= ~(1UL << pin);      // Push-pull
    port->OSPEEDR |= (3UL << (pin * 2));// High speed
    port->PUPDR &= ~(3UL << (pin * 2)); // No pull
}

void GPIO_ConfigInput(GPIO_TypeDef *port, uint8_t pin, uint8_t pull) {
    port->MODER &= ~(3UL << (pin * 2)); // Input mode
    port->PUPDR &= ~(3UL << (pin * 2));
    if (pull == 1) {
        port->PUPDR |= (1UL << (pin * 2)); // Pull-up
    } else if (pull == 2) {
        port->PUPDR |= (2UL << (pin * 2)); // Pull-down
    }
}

void GPIO_ConfigAF(GPIO_TypeDef *port, uint8_t pin, uint8_t af) {
    port->MODER &= ~(3UL << (pin * 2));
    port->MODER |= (2UL << (pin * 2));  // AF Mode
    port->OTYPER &= ~(1UL << pin);
    port->OSPEEDR |= (3UL << (pin * 2));
    port->PUPDR &= ~(3UL << (pin * 2));
    
    if (pin < 8) {
        port->AFR[0] &= ~(0xFUL << (pin * 4));
        port->AFR[0] |= (af << (pin * 4));
    } else {
        port->AFR[1] &= ~(0xFUL << ((pin - 8) * 4));
        port->AFR[1] |= (af << ((pin - 8) * 4));
    }
}

void GPIO_ConfigAnalog(GPIO_TypeDef *port, uint8_t pin) {
    port->MODER |= (3UL << (pin * 2));  // Analog mode
    port->PUPDR &= ~(3UL << (pin * 2));
}

/* ============================================================================ */
/* FULL GPIO INITIALIZATION                             */
/* ============================================================================ */

void Hardware_InitGPIO(void) {
    /* 1. Buttons */
    // Ch? còn 1 nút Key (PA0)
    GPIO_ConfigInput(BTN_KEY_PORT, BTN_KEY_PIN, 1); 
    
    // REMOVED: BTN_MODE config (PB11 is now PWM)

    /* 2. LED Status */
    GPIO_ConfigOutput(LED_STATUS_PORT, LED_STATUS_PIN);
    LED_OFF(); // T?t m?c d?nh

    /* 3. Buzzer/Fan PWM Pin (PB11) */
    // C?u hình AF01 (TIM2) cho PB11
    GPIO_ConfigAF(AUX_PWM_PORT, AUX_PWM_PIN, AUX_PWM_AF);
    
    /* 4. IR LEDs */
    // PA11, PA12, PB2, PA7, PB10, PA8
    GPIO_ConfigOutput(IR_LED_L0_PORT, IR_LED_L0_PIN);
    GPIO_ConfigOutput(IR_LED_L45_PORT, IR_LED_L45_PIN);
    GPIO_ConfigOutput(IR_LED_L90_PORT, IR_LED_L90_PIN);
    
    GPIO_ConfigOutput(IR_LED_R0_PORT, IR_LED_R0_PIN);
    GPIO_ConfigOutput(IR_LED_R45_PORT, IR_LED_R45_PIN);
    GPIO_ConfigOutput(IR_LED_R90_PORT, IR_LED_R90_PIN);
    
    /* 5. IR Receivers (ADC) */
    GPIO_ConfigAnalog(IR_RX_L0_PORT, IR_RX_L0_PIN);
    GPIO_ConfigAnalog(IR_RX_L45_PORT, IR_RX_L45_PIN);
    GPIO_ConfigAnalog(IR_RX_L90_PORT, IR_RX_L90_PIN);
    GPIO_ConfigAnalog(IR_RX_R0_PORT, IR_RX_R0_PIN);
    GPIO_ConfigAnalog(IR_RX_R45_PORT, IR_RX_R45_PIN);
    GPIO_ConfigAnalog(IR_RX_R90_PORT, IR_RX_R90_PIN);
}

void Hardware_InitSystemTimer(void) {
    SystemTimer_Init();
}

void Hardware_Init(void) {
    Hardware_InitClocks();
    Hardware_InitSystemTimer();
    Hardware_InitGPIO();
    // Các init khác g?i sau (Motors, Encoders...)
}

/* ============================================================================ */
/* MOTOR CONTROL                                        */
/* ============================================================================ */

void Hardware_InitMotors(void) {
    /* * LUU Ý QUAN TR?NG: 
     * PA15 và PB3 m?c d?nh là JTAG. Hàm TB6612_Init bên du?i s? g?i GPIO setup.
     * C?n d?m b?o thu vi?n TB6612 c?u hình dúng AF (GPIO_ConfigAF).
     * V?i code GPIO_ConfigAF ? trên, nó s? t? d?ng override JTAG.
     */

    /* Motor A (Right) - TIM2_CH1 (PA15) */
    motor_driver.motor_a.gpio_port = MOTOR_R_IN1_PORT;
    motor_driver.motor_a.in1_pin = MOTOR_R_IN1_PIN;
    motor_driver.motor_a.in2_pin = MOTOR_R_IN2_PIN;
    motor_driver.motor_a.pwm_port = MOTOR_R_PWM_PORT;
    motor_driver.motor_a.pwm_pin = MOTOR_R_PWM_PIN;
    motor_driver.motor_a.pwm_af = MOTOR_R_PWM_AF;
    motor_driver.motor_a.pwm_timer = MOTOR_R_PWM_TIMER;
    motor_driver.motor_a.pwm_channel = MOTOR_R_PWM_CHANNEL;
    
    /* Motor B (Left) - TIM2_CH2 (PB3) - FIXED */
    motor_driver.motor_b.gpio_port = MOTOR_L_IN1_PORT;
    motor_driver.motor_b.in1_pin = MOTOR_L_IN1_PIN;
    motor_driver.motor_b.in2_pin = MOTOR_L_IN2_PIN;
    motor_driver.motor_b.pwm_port = MOTOR_L_PWM_PORT;      
    motor_driver.motor_b.pwm_pin = MOTOR_L_PWM_PIN;        /* PB3 */
    motor_driver.motor_b.pwm_af = MOTOR_L_PWM_AF;          /* AF1 */
    motor_driver.motor_b.pwm_timer = MOTOR_L_PWM_TIMER;    /* TIM2 */
    motor_driver.motor_b.pwm_channel = MOTOR_L_PWM_CHANNEL;/* CH2 */
    
    motor_driver.pwm_prescaler = MOTOR_PWM_PRESCALER;
    motor_driver.pwm_period = MOTOR_PWM_PERIOD;
    
    /* Init Drivers (TIM2 CH1 & CH2) */
    TB6612_Init(&motor_driver);

    /* * --- INIT AUX PWM (FAN/BUZZER) ON TIM2_CH4 (PB11) --- 
     * Vì TIM2 dã du?c TB6612_Init kh?i d?ng, ta ch? c?n enable thêm Channel 4.
     */
    // 1. Configure PWM Mode 1 for Channel 4
    TIM2->CCMR2 &= ~(TIM_CCMR2_OC4M); 
    TIM2->CCMR2 |= (6U << 12); // PWM Mode 1
    TIM2->CCMR2 |= TIM_CCMR2_OC4PE; // Preload enable

    // 2. Enable Capture/Compare for Channel 4
    TIM2->CCER |= TIM_CCER_CC4E;
    
    // 3. Set Duty Cycle = 0 initially
    TIM2->CCR4 = 0;
}

void Hardware_SetMotor(uint8_t motor, int8_t speed) {
    Motor_Select_t m;
    Motor_Direction_t dir;
    uint8_t abs_speed;
    
    m = (motor == 0) ? MOTOR_A : MOTOR_B;
    
    if (speed > 0) {
        dir = MOTOR_FORWARD;
        abs_speed = speed;
    } else if (speed < 0) {
        dir = MOTOR_BACKWARD;
        abs_speed = -speed;
    } else {
        dir = MOTOR_STOP;
        abs_speed = 0;
    }
    
    TB6612_SetMotor(&motor_driver, m, dir, abs_speed);
}

void Hardware_StopMotors(void) {
    TB6612_StopAll(&motor_driver);
    SET_AUX_PWM(0); // T?t luôn Fan/Buzzer khi Stop All
}

/* ============================================================================ */
/* ENCODER FUNCTIONS                                    */
/* ============================================================================ */

void Hardware_InitEncoders(void) {
    // PB4 (JTAG) and PB5 -> TIM3
    // PB6 and PB7 -> TIM4
    Encoder_Init(&encoder_right, ENCODER_RIGHT, ENCODER_R_RESOLUTION);
    Encoder_Init(&encoder_left, ENCODER_LEFT, ENCODER_L_RESOLUTION);
}

int32_t Hardware_GetEncoderCount(uint8_t encoder) {
    if (encoder == 0) return Encoder_GetTotalCount(&encoder_right);
    else return Encoder_GetTotalCount(&encoder_left);
}

float Hardware_GetEncoderSpeed(uint8_t encoder) {
    if (encoder == 0) return Encoder_GetSpeedRPM(&encoder_right);
    else return Encoder_GetSpeedRPM(&encoder_left);
}

void Hardware_ResetEncoder(uint8_t encoder) {
    if (encoder == 0) Encoder_Reset(&encoder_right);
    else Encoder_Reset(&encoder_left);
}

void Hardware_UpdateEncoders(void) {
    Encoder_UpdateSpeed(&encoder_right);
    Encoder_UpdateSpeed(&encoder_left);
}

/* ============================================================================ */
/* IR SENSORS                                           */
/* ============================================================================ */

void Hardware_InitIRSensors(void) {
    /* ADC initialization */
    ADC_Init();
}

// Các hàm IR_LED mapping v?n dúng nh? macro trong pinout.h
void Hardware_IRLedOn(IR_Sensor_t sensor) {
    switch(sensor) {
        case IR_L0:  IR_LED_ON(IR_LED_L0_PORT, IR_LED_L0_PIN); break;
        case IR_L45: IR_LED_ON(IR_LED_L45_PORT, IR_LED_L45_PIN); break;
        case IR_L90: IR_LED_ON(IR_LED_L90_PORT, IR_LED_L90_PIN); break;
        case IR_R0:  IR_LED_ON(IR_LED_R0_PORT, IR_LED_R0_PIN); break;
        case IR_R45: IR_LED_ON(IR_LED_R45_PORT, IR_LED_R45_PIN); break;
        case IR_R90: IR_LED_ON(IR_LED_R90_PORT, IR_LED_R90_PIN); break;
    }
}

void Hardware_IRLedOff(IR_Sensor_t sensor) {
    switch(sensor) {
        case IR_L0:  IR_LED_OFF(IR_LED_L0_PORT, IR_LED_L0_PIN); break;
        case IR_L45: IR_LED_OFF(IR_LED_L45_PORT, IR_LED_L45_PIN); break;
        case IR_L90: IR_LED_OFF(IR_LED_L90_PORT, IR_LED_L90_PIN); break;
        case IR_R0:  IR_LED_OFF(IR_LED_R0_PORT, IR_LED_R0_PIN); break;
        case IR_R45: IR_LED_OFF(IR_LED_R45_PORT, IR_LED_R45_PIN); break;
        case IR_R90: IR_LED_OFF(IR_LED_R90_PORT, IR_LED_R90_PIN); break;
    }
}

void Hardware_IRLedAllOff(void) {
    // T?t h?t 6 dèn
    IR_LED_OFF(IR_LED_L0_PORT, IR_LED_L0_PIN);
    IR_LED_OFF(IR_LED_L45_PORT, IR_LED_L45_PIN);
    IR_LED_OFF(IR_LED_L90_PORT, IR_LED_L90_PIN);
    IR_LED_OFF(IR_LED_R0_PORT, IR_LED_R0_PIN);
    IR_LED_OFF(IR_LED_R45_PORT, IR_LED_R45_PIN);
    IR_LED_OFF(IR_LED_R90_PORT, IR_LED_R90_PIN);
}

uint16_t Hardware_ReadIRSensor(IR_Sensor_t sensor) {
    uint8_t channel;
    switch(sensor) {
        case IR_L0:  channel = IR_RX_L0_ADC_CHANNEL;   break;
        case IR_L45: channel = IR_RX_L45_ADC_CHANNEL;  break;
        case IR_L90: channel = IR_RX_L90_ADC_CHANNEL;  break;
        case IR_R0:  channel = IR_RX_R0_ADC_CHANNEL;   break;
        case IR_R45: channel = IR_RX_R45_ADC_CHANNEL;  break;
        case IR_R90: channel = IR_RX_R90_ADC_CHANNEL;  break;
        default: return 0;
    }
    return ADC_Read(channel);
}

/* ============================================================================ */
/* BUTTONS (Fixed)                                      */
/* ============================================================================ */

void Hardware_InitButtons(void) {
    // Config Input dã làm trong Hardware_InitGPIO
}

uint8_t Hardware_ReadButton(uint8_t button) {
    uint8_t state = 1; // Default Released
    
    switch(button) {
        case 0: // KEY Button
            state = READ_BTN_KEY(); 
            break;
        case 1: // MODE Button - REMOVED!
            // Có th? dùng nút Key làm ch?c nang ph?, ho?c tr? v? 1 (ko b?m)
            state = 1; 
            break;
        default: 
            state = 1; 
            break;
    }
    return (state == BUTTON_PRESSED) ? 1 : 0;
}

/* ============================================================================ */
/* BUZZER / FAN CONTROL (PWM)                           */
/* ============================================================================ */

void Hardware_InitBuzzer(void) {
    // Ðã init trong Hardware_InitMotors (TIM2_CH4)
}

void Hardware_BuzzerOn(void) {
    SET_AUX_PWM(2500); // 50% Duty Cycle -> Kêu Bíp (ho?c Fan quay 50%)
}

void Hardware_BuzzerOff(void) {
    SET_AUX_PWM(0); // 0% Duty -> T?t
}

void Hardware_Beep(uint16_t duration_ms) {
    Hardware_BuzzerOn();
    delay_ms_blocking(duration_ms);
    Hardware_BuzzerOff();
}

/* ============================================================================ */
/* SYSTEM STATUS                                        */
/* ============================================================================ */

float Hardware_GetBatteryVoltage(void) {
    return 8.2f; 
}

void Hardware_PrintStatus(void) {
    char buffer[100];
    
    UART_SendString("\r\n=== Hardware Status ===\r\n");
    
    sprintf(buffer, "Uptime: %lu ms\r\n", millis());
    UART_SendString(buffer);
    
    sprintf(buffer, "Enc R: %ld, %.1f RPM\r\n", Hardware_GetEncoderCount(0), Hardware_GetEncoderSpeed(0));
    UART_SendString(buffer);
    
    sprintf(buffer, "Enc L: %ld, %.1f RPM\r\n", Hardware_GetEncoderCount(1), Hardware_GetEncoderSpeed(1));
    UART_SendString(buffer);
    
    UART_SendString("=======================\r\n");
}
