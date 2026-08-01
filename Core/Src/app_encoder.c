#include "app_encoder.h"

/**
 * @brief 编码器计数变量
 * encoder_L: 左编码器计数
 * 正数表示顺时针旋转
 */
volatile int64_t encoder_L = 0;

/**
 * @brief 编码器计数变量
 * encoder_R: 右编码器计数
 * 正数表示顺时针旋转
 */
volatile int64_t encoder_R = 0;

