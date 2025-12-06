/**
 * @file tb6612fng.c
 * @brief TB6612FNG Dual Motor Driver Implementation - Register Level
 * @author HELLO12312E
 * @date 2025-01-30
 * 
 * CHANGELOG:
 * - 2025-01-30: Fixed PWM init for different timers (no longer assumes shared timer)
 * - 2025-01-30: Fixed GPIO_WritePin to use BSRRL/BSRRH instead of BSRR
 */

#include "tb6612fng.h"

/* Private Functions */

/**
 * @brief Enable GPIO clock
 */
static void GPIO_ClockEnable(GPIO_TypeDef *GPIOx)
{
    if (GPIOx == GPIOA) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    }
    else if (GPIOx == GPIOB) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    }
    else if (GPIOx == GPIOC) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    }
}

/**
 * @brief Configure GPIO pin as output
 */
static void GPIO_ConfigOutput(GPIO_TypeDef *GPIOx, uint8_t pin)
{
    /* Set mode to output (01) */
    GPIOx->MODER &= ~(3UL << (pin * 2));
    GPIOx->MODER |= (1UL << (pin * 2));
    
    /* Set output type to push-pull (0) */
    GPIOx->OTYPER &= ~(1UL << pin);
    
    /* Set speed to high (10) */
    GPIOx->OSPEEDR &= ~(3UL << (pin * 2));
    GPIOx->OSPEEDR |= (2UL << (pin * 2));
    
    /* No pull-up/pull-down (00) */
    GPIOx->PUPDR &= ~(3UL << (pin * 2));
}

/**
 * @brief Configure GPIO pin as alternate function
 */
static void GPIO_ConfigAF(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t af)
{
    /* Set mode to alternate function (10) */
    GPIOx->MODER &= ~(3UL << (pin * 2));
    GPIOx->MODER |= (2UL << (pin * 2));
    
    /* Set output type to push-pull */
    GPIOx->OTYPER &= ~(1UL << pin);
    
    /* Set speed to high */
    GPIOx->OSPEEDR &= ~(3UL << (pin * 2));
    GPIOx->OSPEEDR |= (2UL << (pin * 2));
    
    /* No pull-up/pull-down */
    GPIOx->PUPDR &= ~(3UL << (pin * 2));
    
    /* Set alternate function */
    if (pin < 8) {
        GPIOx->AFR[0] &= ~(0xFUL << (pin * 4));
        GPIOx->AFR[0] |= (af << (pin * 4));
    } else {
        GPIOx->AFR[1] &= ~(0xFUL << ((pin - 8) * 4));
        GPIOx->AFR[1] |= (af << ((pin - 8) * 4));
    }
}

/**
 * @brief Write GPIO pin - FIXED for CMSIS
 */
static void GPIO_WritePin(GPIO_TypeDef *GPIOx, uint8_t pin, uint8_t state)
{
    if (state) {
        GPIOx->BSRRL = (1UL << pin);  /* Set bit using BSRRL */
    } else {
        GPIOx->BSRRH = (1UL << pin);  /* Reset bit using BSRRH */
    }
}

/**
 * @brief Enable Timer clock
 */
static void Timer_ClockEnable(TIM_TypeDef *TIMx)
{
    if (TIMx == TIM1) {
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    }
    else if (TIMx == TIM2) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    }
    else if (TIMx == TIM3) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    }
    else if (TIMx == TIM4) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    }
    else if (TIMx == TIM5) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    }
}

/**
 * @brief Initialize PWM for a channel
 */
static void PWM_Init(TIM_TypeDef *TIMx, uint8_t channel, uint16_t prescaler, uint32_t period)
{
    /* Enable timer clock */
    Timer_ClockEnable(TIMx);
    
    /* Disable counter */
    TIMx->CR1 &= ~(1 << 0);  /* CEN = 0 */
    
    /* Set prescaler and period */
    TIMx->PSC = prescaler;
    TIMx->ARR = period;
    
    /* Auto-reload preload enable */
    TIMx->CR1 |= (1 << 7);  /* ARPE = 1 */
    
    /* Configure PWM mode for channel */
    switch(channel) {
        case 1:
            /* PWM mode 1 (0110), Output compare preload enable */
            TIMx->CCMR1 &= ~(0xFFUL << 0);
            TIMx->CCMR1 |= (0x6UL << 4) | (1UL << 3);
            /* Enable channel, active high */
            TIMx->CCER |= (1UL << 0);
            TIMx->CCER &= ~(1UL << 1);
            TIMx->CCR1 = 0;
            break;
            
        case 2:
            TIMx->CCMR1 &= ~(0xFFUL << 8);
            TIMx->CCMR1 |= (0x6UL << 12) | (1UL << 11);
            TIMx->CCER |= (1UL << 4);
            TIMx->CCER &= ~(1UL << 5);
            TIMx->CCR2 = 0;
            break;
            
        case 3:
            TIMx->CCMR2 &= ~(0xFFUL << 0);
            TIMx->CCMR2 |= (0x6UL << 4) | (1UL << 3);
            TIMx->CCER |= (1UL << 8);
            TIMx->CCER &= ~(1UL << 9);
            TIMx->CCR3 = 0;
            break;
            
        case 4:
            TIMx->CCMR2 &= ~(0xFFUL << 8);
            TIMx->CCMR2 |= (0x6UL << 12) | (1UL << 11);
            TIMx->CCER |= (1UL << 12);
            TIMx->CCER &= ~(1UL << 13);
            TIMx->CCR4 = 0;
            break;
    }
    
    /* For TIM1, enable main output */
    if (TIMx == TIM1) {
        TIMx->BDTR |= (1UL << 15);  /* MOE = 1 */
    }
    
    /* Generate update event */
    TIMx->EGR |= (1 << 0);  /* UG = 1 */
    
    /* Clear flags */
    TIMx->SR = 0;
    
    /* Start counter */
    TIMx->CR1 |= (1 << 0);  /* CEN = 1 */
}

/**
 * @brief Set PWM duty cycle
 */
static void PWM_SetDuty(TIM_TypeDef *TIMx, uint8_t channel, uint32_t duty)
{
    switch(channel) {
        case 1:
            TIMx->CCR1 = duty;
            break;
        case 2:
            TIMx->CCR2 = duty;
            break;
        case 3:
            TIMx->CCR3 = duty;
            break;
        case 4:
            TIMx->CCR4 = duty;
            break;
    }
}

/* Public Functions */

/**
 * @brief Initialize TB6612FNG motor driver
 */
void TB6612_Init(TB6612_Handle_t *handle)
{
    /* Enable GPIO clocks */
    GPIO_ClockEnable(handle->motor_a.gpio_port);
    GPIO_ClockEnable(handle->motor_a.pwm_port);
    GPIO_ClockEnable(handle->motor_b.gpio_port);
    GPIO_ClockEnable(handle->motor_b.pwm_port);
    
    /* Configure Motor A pins */
    GPIO_ConfigOutput(handle->motor_a.gpio_port, handle->motor_a.in1_pin);
    GPIO_ConfigOutput(handle->motor_a.gpio_port, handle->motor_a.in2_pin);
    GPIO_ConfigAF(handle->motor_a.pwm_port, handle->motor_a.pwm_pin, handle->motor_a.pwm_af);
    
    /* Configure Motor B pins */
    GPIO_ConfigOutput(handle->motor_b.gpio_port, handle->motor_b.in1_pin);
    GPIO_ConfigOutput(handle->motor_b.gpio_port, handle->motor_b.in2_pin);
    GPIO_ConfigAF(handle->motor_b.pwm_port, handle->motor_b.pwm_pin, handle->motor_b.pwm_af);
    
    /* Initialize PWM - Always init both timers separately */
    PWM_Init(handle->motor_a.pwm_timer, handle->motor_a.pwm_channel, 
             handle->pwm_prescaler, handle->pwm_period);
    
    /* Check if different timer before init */
    if (handle->motor_b.pwm_timer != handle->motor_a.pwm_timer) {
        PWM_Init(handle->motor_b.pwm_timer, handle->motor_b.pwm_channel,
                 handle->pwm_prescaler, handle->pwm_period);
    }
    
    /* Initialize speeds */
    handle->speed_a = 0;
    handle->speed_b = 0;
    
    /* Stop all motors initially */
    TB6612_StopAll(handle);
}

/**
 * @brief Set motor speed and direction
 */
void TB6612_SetMotor(TB6612_Handle_t *handle, Motor_Select_t motor, 
                     Motor_Direction_t direction, uint8_t speed)
{
    Motor_Config_t *motor_cfg;
    uint32_t pwm_value;
    
    /* Limit speed to 100% */
    if (speed > 100) speed = 100;
    
    /* Select motor configuration */
    if (motor == MOTOR_A) {
        motor_cfg = &handle->motor_a;
        handle->speed_a = speed;
    } else {
        motor_cfg = &handle->motor_b;
        handle->speed_b = speed;
    }
    
    /* Calculate PWM value */
    pwm_value = (uint32_t)((speed * handle->pwm_period) / 100UL);
    
    /* Set direction and PWM */
    switch(direction) {
        case MOTOR_FORWARD:
            GPIO_WritePin(motor_cfg->gpio_port, motor_cfg->in1_pin, 1);
            GPIO_WritePin(motor_cfg->gpio_port, motor_cfg->in2_pin, 0);
            PWM_SetDuty(motor_cfg->pwm_timer, motor_cfg->pwm_channel, pwm_value);
            break;
            
        case MOTOR_BACKWARD:
            GPIO_WritePin(motor_cfg->gpio_port, motor_cfg->in1_pin, 0);
            GPIO_WritePin(motor_cfg->gpio_port, motor_cfg->in2_pin, 1);
            PWM_SetDuty(motor_cfg->pwm_timer, motor_cfg->pwm_channel, pwm_value);
            break;
            
        case MOTOR_BRAKE:
            GPIO_WritePin(motor_cfg->gpio_port, motor_cfg->in1_pin, 1);
            GPIO_WritePin(motor_cfg->gpio_port, motor_cfg->in2_pin, 1);
            PWM_SetDuty(motor_cfg->pwm_timer, motor_cfg->pwm_channel, handle->pwm_period);
            break;
            
        case MOTOR_STOP:
            GPIO_WritePin(motor_cfg->gpio_port, motor_cfg->in1_pin, 0);
            GPIO_WritePin(motor_cfg->gpio_port, motor_cfg->in2_pin, 0);
            PWM_SetDuty(motor_cfg->pwm_timer, motor_cfg->pwm_channel, 0);
            break;
    }
}

/**
 * @brief Stop motor (coast mode)
 */
void TB6612_StopMotor(TB6612_Handle_t *handle, Motor_Select_t motor)
{
    TB6612_SetMotor(handle, motor, MOTOR_STOP, 0);
}

/**
 * @brief Brake motor (short brake)
 */
void TB6612_BrakeMotor(TB6612_Handle_t *handle, Motor_Select_t motor)
{
    TB6612_SetMotor(handle, motor, MOTOR_BRAKE, 0);
}

/**
 * @brief Stop all motors
 */
void TB6612_StopAll(TB6612_Handle_t *handle)
{
    TB6612_StopMotor(handle, MOTOR_A);
    TB6612_StopMotor(handle, MOTOR_B);
}

/**
 * @brief Brake all motors
 */
void TB6612_BrakeAll(TB6612_Handle_t *handle)
{
    TB6612_BrakeMotor(handle, MOTOR_A);
    TB6612_BrakeMotor(handle, MOTOR_B);
}

/**
 * @brief Get current speed of motor
 */
uint8_t TB6612_GetSpeed(TB6612_Handle_t *handle, Motor_Select_t motor)
{
    if (motor == MOTOR_A) {
        return handle->speed_a;
    } else {
        return handle->speed_b;
    }
}
