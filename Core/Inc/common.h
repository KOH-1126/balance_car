/**
 ******************************************************************************
 * @file    common.h
 * @brief   用于存储一些在不同模块都会用到的宏定义和函数声明
 */
#ifndef COMMON_H
#define COMMON_H

#include "stm32f1xx_hal.h"

#define R2D (57.2958f) // 弧度转角度的系数
#define D2R (0.0174533f) // 角度转弧度的系数
#define PI (3.14159265359f)
#define g_in_NTU (9.78f) // 1g在NTU的加速度计输出值，单位为g


/**
 * @brief 获取系统启动后的微秒级时间戳
 * @note  需在HAL_Init()完成且SysTick作为HAL时基时调用
 * @retval 当前时间，单位us
 */
uint64_t GetUs(void);

/**
 * @brief 定义一个周期性执行的宏
 * 在函数使用，不到执行时间直接退出函数
 * @param T 周期时间，单位为毫秒
 */
#define PERIODIC(T) \
static uint32_t nxt = 0; \
if (HAL_GetTick() < nxt) return; \
nxt += (T);

/**
 * @brief 开始一个周期性执行的代码块
 * 到了执行时间才会执行代码块内的内容，不到执行时间不会直接退出函数，还可以在同一个函数内做其他事情
 * 代码块内的内容必须用PERIODIC_END宏结束
 * @param NAME 代码块名称
 * @param T 周期时间，单位为毫秒
 */
#define PERIODIC_START(NAME, T) \
static uint32_t NAME##_nxt = 0; \
if(HAL_GetTick() >= NAME##_nxt) {\
NAME##_nxt += (T);

#define PERIODIC_END }

#endif