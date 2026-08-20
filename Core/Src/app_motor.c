#include "app_motor.h"
#include "pid.h"
#include "common.h"
#include "app_encoder.h"
#include "app_pwm.h"
#include "app_bat.h"

PID_TypeDef pidL, pidR;

void App_Motor_Init(void){
    PID_Init(&pidL, 0.5f, 7.0f, 0.0f);
    PID_Init(&pidR, 0.5f, 7.0f, 0.0f);
}

void App_Motor_Proc(void){
    PERIODIC(1)

    // 获取左右轮角速度
    float omega_L = Get_Omega_L();
    float omega_R = Get_Omega_R();

    // 计算PID输出
    float ua_L = PID_Compute(&pidL, omega_L);
    float ua_R = PID_Compute(&pidR, omega_R);

    // 将电压ua转换为占空比duty并加到电机上
    float vbat = App_Bat_Get();
    float duty_L = ua_L / vbat * 100.0f;
    float duty_R = ua_R / vbat * 100.0f;

    App_PWM_SetDuty_L(duty_L);
    App_PWM_SetDuty_R(duty_R);
}

void App_Motor_SetSP_L(float SP){
    PID_SetSP(&pidL, SP);
}

void App_Motor_SetSP_R(float SP){
    PID_SetSP(&pidR, SP);
}

