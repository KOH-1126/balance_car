#include "stm32f1xx_hal.h"

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