#ifndef PID_H
#define PID_H

#include "stm32f1xx_hal.h"

typedef struct {
    float Kp; // 比例系数
    float Ki; // 积分系数
    float Kd; // 微分系数
    float SP; // 设定值

    uint64_t last_time; // 上一次计算的时间
    float last_error; // 上一次的误差值
    float last_err_integral; // 上一次的误差积分值

    float UpperLimit; // 输出上限
    float LowerLimit; // 输出下限

    uint8_t first_compute; // 标记是否为第一次计算 为0表示是第一次，否则表示不是
} PID_TypeDef;

/**
 * @brief 初始化PID控制器
 * @param pid PID控制器结构体指针
 * @param Kp 比例系数
 * @param Ki 积分系数
 * @param Kd 微分系数
 */
void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd);

/**
 * @brief 设置PID控制器的输出上限和下限
 */
void PID_SetLimits(PID_TypeDef *pid, float lowerLimit, float upperLimit);

/**
 * @brief 设置PID控制器的设定值
 * @param pid PID控制器结构体指针
 * @param SP 设定值
 */
void PID_SetSP(PID_TypeDef *pid, float SP);

/**
 * @brief 执行一次PID计算
 * @param pid PID控制器结构体指针
 * @param FB 反馈值
 * @return PID输出值
 */
float PID_Compute(PID_TypeDef *pid, float FB);

/**
 * @brief 重置PID控制器的状态
 * @param pid PID控制器结构体指针
 */
void PID_Reset(PID_TypeDef *pid);

#endif