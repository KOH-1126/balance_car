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
volatile uint16_t t0_L = 0 , t0_R = 0;

/**
 * @brief 左右轮编码器T法测速结束计数时间戳 单位us
 */
volatile uint16_t t1_L = 0 , t1_R = 0, deltaTL = 0, deltaTR = 0;

float Get_Position_L(void)
{
    return (float)encoder_L * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2DEGREE;
}

float Get_Position_R(void)
{
    return (float)encoder_R * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2DEGREE;
}


