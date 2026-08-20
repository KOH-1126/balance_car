#ifndef APP_PWM_H
#define APP_PWM_H

#include "stm32f1xx_hal.h"

void App_PWM_Init(void);

/**
 * @brief 控制pwm处于开启或休眠 
 * @param on 不为0 开启 向STBY引脚输出高电平 （PA1）
 * @param on =0 休眠 向STBY引脚输出低电平
 */
void App_PWM_Cmd(uint8_t on); 

/**
 * @brief 设置左电机占空比
 * @param duty 占空比 -100.0f~100.0f 正数正转 负数反转 
 * @正转前进，车头为丝印字体的一侧
 */
void App_PWM_SetDuty_L(float duty);  

/**
 * @brief 设置右电机占空比
 * @param duty 占空比 -100.0f~100.0f 正数正转 负数反转
 */
void App_PWM_SetDuty_R(float duty);  

#endif