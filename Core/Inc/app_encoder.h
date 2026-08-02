#ifndef APP_ENCODER_H
#define APP_ENCODER_H

#include "stm32f1xx_hal.h"

#define EDGE2ENCODER (1.0f / 22) // 编码器每增加1个计数，编码器转过的圈数
#define ENCODER2WHEEL (1500.0f / 30613) // 编码器每转一圈，车轮转过的圈数
#define WHEEL2DEGREE (360.0f) // 车轮转过一圈，车轮转过的角度

extern volatile int64_t encoder_L;
extern volatile int64_t encoder_R;
extern volatile float omega_L;
extern volatile float omega_R;
extern volatile int8_t direction_L;
extern volatile int8_t direction_R;
extern volatile uint16_t t0_L;
extern volatile uint16_t t0_R;
extern volatile uint16_t t1_L;
extern volatile uint16_t t1_R;
extern volatile uint16_t deltaTL;
extern volatile uint16_t deltaTR;

float Get_Position_L(void);
float Get_Position_R(void);

#endif
