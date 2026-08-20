#ifndef __APP_MOTOR_H
#define __APP_MOTOR_H

#include "stm32f1xx_hal.h"

/**
 * @brief 初始化电机控制模块
 */
void App_Motor_Init(void);

/**
 * @brief 电机进程函数
 */
void App_Motor_Proc(void);

/**
 * @brief 设置左轮速度设定值
 * @param SP 速度设定值 单位：rad/s
 */
void App_Motor_SetSP_L(float SP);

/**
 * @brief 设置右轮速度设定值
 * @param SP 速度设定值 单位：rad/s
 */
void App_Motor_SetSP_R(float SP);

#endif