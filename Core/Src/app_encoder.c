#include "app_encoder.h"
#include "common.h"

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

/**
 * @brief 获取左轮位置
 * @return 左轮位置，单位为度
 */
float Get_Position_L(void)
{
    int64_t encoder_snapshot;
    __disable_irq();
    encoder_snapshot = encoder_L;
    __enable_irq();
    return (float)encoder_snapshot * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2DEGREE;
}

/**
 * @brief 获取右轮位置
 * @return 右轮位置，单位为度
 */
float Get_Position_R(void)
{
    int64_t encoder_snapshot;
    __disable_irq();
    encoder_snapshot = encoder_R;
    __enable_irq();
    return (float)encoder_snapshot * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2DEGREE;
}

/**
 * @brief 获取左轮角速度
 * @return 左轮角速度，单位为弧度每秒
 */
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
    omega_L = direction_L_cpy * 1000000.0f/(deltaTL_cpy) * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2RADIAN; // 计算左轮角速度 
    return omega_L;
}

/**
 * @brief 获取右轮角速度
 * @return 右轮角速度，单位为弧度每秒
 */
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
    omega_R = direction_R_cpy * 1000000.0f/(deltaTR_cpy) * EDGE2ENCODER * ENCODER2WHEEL * WHEEL2RADIAN; // 计算右轮角速度
    return omega_R;
}


