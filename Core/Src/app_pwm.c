#include "app.pwm.h"
#include "main.h"
#include <math.h>
#include "tim.h"

// static void STBY_Pin_Init(void);


/**
 * @brief 初始化TB6612电机控制芯片 */
void App_PWM_Init(void)
{
    // 初始化STBY引脚 PA1 OUT PP 
    // 初始化AIN1 AIN2 BIN1 BIN2引脚，分别控制左右电机的转向
    //初始化PWMA PWMB引脚，分别控制左右电机的转速
    //以上步骤均由mx完成

    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK)
        Error_Handler();

    if (HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1) != HAL_OK)
        Error_Handler();
}

void App_PWM_Cmd(uint8_t on){
    if(on == 0){
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    }else{
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    }
}


void App_PWM_SetDuty_L(float duty)
{
    uint8_t sign;
    if(duty >= 0) sign = 1; //正转
    else sign = 0; //反转
    duty = fabsf(duty);
    if(sign == 1){
        HAL_GPIO_WritePin(GPIOA, L_IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, L_IN2_Pin, GPIO_PIN_RESET);
    }
    else{
        HAL_GPIO_WritePin(GPIOA, L_IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, L_IN2_Pin, GPIO_PIN_SET);
    }

    uint16_t ccr = (uint16_t)(duty * 1000 / 100.0f);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);
}

void App_PWM_SetDuty_R(float duty)
{
    uint8_t sign;
    if(duty >= 0) sign = 1; //正转
    else sign = 0; //反转
    duty = fabsf(duty);
    if(sign == 0){
        HAL_GPIO_WritePin(GPIOB, R_IN1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB, R_IN2_Pin, GPIO_PIN_RESET);
    }
    else{
        HAL_GPIO_WritePin(GPIOB, R_IN1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, R_IN2_Pin, GPIO_PIN_SET);
    }

    uint16_t ccr = (uint16_t)(duty * 1000 / 100.0f);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ccr);
}

