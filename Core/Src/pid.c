#include "pid.h"
#include "common.h"

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

    float error_integral = pid->last_err_integral;
    float error_derivative = 0.0f;

    if(pid->first_compute == 0) {
        // 第一次计算时，积分和微分项为0
        pid->first_compute = 1; // 标记为已计算过一次
    } else if(deltaT > 0.0f && deltaT < 0.1f) {
        error_integral = pid->last_err_integral + (error + pid->last_error) / 2 * deltaT;
        error_derivative = (error - pid->last_error) / deltaT;
    }

    float P_out = pid->Kp * error;
    float I_out = pid->Ki * error_integral;
    float D_out = pid->Kd * error_derivative;
    float output = P_out + I_out + D_out;

    /* 条件积分抗饱和：若本次积分会把输出继续推向饱和区，则撤销本次积分。 */
    if (output > pid->UpperLimit) {
        if (pid->Ki * (error_integral - pid->last_err_integral) > 0.0f) {
            error_integral = pid->last_err_integral;
            output = P_out + pid->Ki * error_integral + D_out;
        }
        if (output > pid->UpperLimit) output = pid->UpperLimit;
    } else if (output < pid->LowerLimit) {
        if (pid->Ki * (error_integral - pid->last_err_integral) < 0.0f) {
            error_integral = pid->last_err_integral;
            output = P_out + pid->Ki * error_integral + D_out;
        }
        if (output < pid->LowerLimit) output = pid->LowerLimit;
    }

    // 更新PID状态
    pid->last_time = current_time;
    pid->last_error = error;
    pid->last_err_integral = error_integral;

    // 防止参数或反馈异常时越过最终输出限幅
    if (output > pid->UpperLimit) {
        output = pid->UpperLimit;
    } else if (output < pid->LowerLimit) {
        output = pid->LowerLimit;
    }

    return output;
}

void PID_Reset(PID_TypeDef *pid){
    pid->last_time = GetUs(); 
    pid->last_error = 0.0f; // 重置上一次误差为0
    pid->last_err_integral = 0.0f; // 重置上一次误差积分为0
    pid->first_compute = 0; // 重置为未计算过
}
