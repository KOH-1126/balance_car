#include "pid.h"
#include "app_bat.h"
#include "common.h"

extern PID_TypeDef pidL, pidR;

void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->SP = 0.0f; // 初始化设定值为0
    
    pid->last_time = GetUs(); 
    pid->last_error = 0.0f; // 初始化上一次误差为
    pid->last_err_integral = 0.0f; // 初始化上一次误差积分为0

    // 设置输出上限和下限为float类型的最大值和最小值,，即初始不设限
    pid->UpperLimit = 3.4e+38f; // 设置输出上限为float类型的最大值
    pid->LowerLimit = -3.4e+38f; // 设置输出下限为float类型的最小值

    pid->first_compute = 0; // 标记为未计算过
}

void PID_SetSP(PID_TypeDef *pid, float SP)
{
    pid->SP = SP;
}

void PID_SetLimits(PID_TypeDef *pid, float lowerLimit, float upperLimit){
    pid->LowerLimit = lowerLimit;
    pid->UpperLimit = upperLimit;
}

float PID_Compute(PID_TypeDef *pid, float FB)
{
    float error = pid->SP - FB;
    uint64_t current_time = GetUs();

    float deltaT = (current_time - pid->last_time) * 1.0e-6f; // 将时间差转换为秒

    float error_integral = 0.0f;
    float error_derivative = 0.0f;

    if(pid->first_compute == 0) {
        // 第一次计算时，积分和微分项为0
        pid->first_compute = 1; // 标记为已计算过一次
    } else {
        error_integral = pid->last_err_integral + (error + pid->last_error) / 2 * deltaT;
        error_derivative = (error - pid->last_error) / deltaT;
    }
    
    

    float P_out = pid->Kp * error;
    float I_out = pid->Ki * error_integral;
    float D_out = pid->Kd * error_derivative;

    // 更新PID状态
    pid->last_time = current_time;
    pid->last_error = error;
    pid->last_err_integral = error_integral;

    float output = P_out + I_out + D_out;

    // 限制输出范围
    if (output > pid->UpperLimit) {
        output = pid->UpperLimit;
    } else if (output < pid->LowerLimit) {
        output = pid->LowerLimit;
    }
 
    // 限制上次误差范围 防止积分器爆炸
    if(pid->last_err_integral > pid->UpperLimit){
        pid->last_err_integral = pid->UpperLimit;
    } else if(pid->last_err_integral < pid->LowerLimit){
        pid->last_err_integral = pid->LowerLimit;
    }

    return output;
}

void PID_Reset(PID_TypeDef *pid){
    pid->last_time = GetUs(); 
    pid->last_error = 0.0f; // 重置上一次误差为0
    pid->last_err_integral = 0.0f; // 重置上一次误差积分为0
    pid->first_compute = 0; // 重置为未计算过
    float vbat = App_Bat_Get();
    PID_SetLimits(&pidL, -vbat, vbat);
    PID_SetLimits(&pidR, -vbat, vbat);
}