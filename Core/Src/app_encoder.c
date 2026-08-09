#include "app_encoder.h"



/**
 * @brief 编码器计数变量
 * encoder_L: 左编码器计数
 * 正数表示前进
 */
volatile int64_t encoder_L = 0;

/**
 * @brief 编码器计数变量
 * encoder_R: 右编码器计数
 * 正数表示前进
 */
volatile int64_t encoder_R = 0;

/**
 * @brief 左右轮速度
 */
volatile float omega_L = 0.0f, omega_R = 0.0f;

/**
 * @brief 左右轮方向 1表示前进，-1表示后退
 */
volatile int8_t direction_L = 1, direction_R = 1;

/**
 * @brief 左右轮编码器T法测速开始计数时间戳 单位us
 */
volatile uint64_t t0_L = 0 , t0_R = 0;

/**
 * @brief 左右轮编码器T法测速结束计数时间戳 单位us
 */
volatile uint64_t t1_L = 0 , t1_R = 0, deltaTL = 0, deltaTR = 0;

float Get_Position_L(void)
{
    int64_t encoder_snapshot;
    __disable_irq();
    encoder_snapshot = encoder_L;
    __enable_irq();
    return (float)encoder_snapshot * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2DEGREE;
}

float Get_Position_R(void)
{
    int64_t encoder_snapshot;
    __disable_irq();
    encoder_snapshot = encoder_R;
    __enable_irq();
    return (float)encoder_snapshot * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2DEGREE;
}

float Get_Omega_L(void){
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    int8_t direction_L_cpy = direction_L;
    uint64_t t1_L_cpy = t1_L;
    uint64_t deltaTL_cpy = deltaTL;
    __set_PRIMASK(primask);

    if(direction_L_cpy == 2 || direction_L_cpy == -2) return 0.0f;
    uint64_t now = GetUs();
    deltaTL_cpy = (deltaTL_cpy > now - t1_L_cpy) ? deltaTL_cpy : (now - t1_L_cpy); // 预测算法
    omega_L = direction_L_cpy * 1000000.0f/(deltaTL_cpy) * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2DEGREE; // 计算左轮角速度 
    return omega_L;
}

float Get_Omega_R(void){
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    int8_t direction_R_cpy = direction_R;
    uint64_t t1_R_cpy = t1_R;
    uint64_t deltaTR_cpy = deltaTR;
    __set_PRIMASK(primask);

    if(direction_R_cpy == 2 || direction_R_cpy == -2) return 0.0f;
    uint64_t now = GetUs();
    deltaTR_cpy = (deltaTR_cpy > now - t1_R_cpy) ? deltaTR_cpy : (now - t1_R_cpy); // 预测算法
    omega_R = direction_R_cpy * 1000000.0f/(deltaTR_cpy) * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2DEGREE; // 计算右轮角速度
    return omega_R;
}

/**
 * @brief 获取系统启动后的微秒级时间戳
 * @note  需在HAL_Init()完成且SysTick作为HAL时基时调用
 * @retval 当前时间，单位us
 */
uint64_t GetUs(void)
{
    uint32_t primask;
    uint32_t tick_ms; // 当前毫秒数
    uint32_t systick_value; // SysTick 当前倒计时值
    uint32_t systick_period; // SysTick 一个完整周期（1ms）有多少个计数
    uint32_t tick_period_us; // 一个 SysTick 周期（1ms）对应多少微秒

    primask = __get_PRIMASK(); // 看看中断有没有被禁止
    __disable_irq(); // 禁止中断，防止在读取 SysTick->VAL 时被中断打断

    // 获取当前毫秒数和 SysTick 当前倒计时值 必须连着读取，防止被中断打断
    tick_ms = HAL_GetTick();
    systick_value = SysTick->VAL;

    systick_period = SysTick->LOAD + 1U; // 获取systick倒数计数周期
    tick_period_us = (uint32_t)HAL_GetTickFreq() * 1000U; // 注意HAL_GetTickFreq()返回的是每个systick倒数周期的毫秒数

    /* SysTick已回卷但中断尚未执行时，补上本次HAL时基周期。 */
    if ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) != 0U) // 如果systick在上面的过程中倒数到0，需要补上本次HAL时基周期
    {
        tick_ms += (uint32_t)HAL_GetTickFreq();
        systick_value = SysTick->VAL;
    }

    if (primask == 0U) // 如果中断之前是开启的，那么在返回前重新开启中断
    {
        __enable_irq();
    }

    return ((uint64_t)tick_ms * 1000ULL) //毫秒转换成微秒
         + (((uint64_t)((systick_period - 1U) - systick_value) * tick_period_us)
            / systick_period);
}


