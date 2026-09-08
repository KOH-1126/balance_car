#include "app_control.h"
#include "pid.h"
#include "common.h"
#include "qmath.h"
#include "app_mpu6050.h"
#include "app_motor.h"
#include "app_encoder.h"

#define Lp (0.062f) // 倒立摆长度，单位为米
#define Rw (0.032f) // 轮子半径，单位为米

static PID_TypeDef pid_dot_y; // 前进速度的PID控制器
static PID_TypeDef pid_roll; // 滚转角的PID控制器
static PID_TypeDef pid_dot_roll; // 滚转角速度的PID控制器


static float omega_ref = 0.0f; // 轮子角速度的目标值，单位为rad/s
static uint64_t last_time = 0;

/**
 * @brief 初始化控制模块
 */
void App_Control_Init(void)
{
    PID_Init(&pid_dot_y, 10.0f, 1.0f, 0.0f); 
    PID_SetLimits(&pid_dot_y, -0.5f*g_in_NTU, 0.5f*g_in_NTU); // 设置输出上限和下限为-0.5g到0.5g
    PID_SetSP(&pid_dot_y, 0.0f); // 设置前进速度的目标值为0

    // 初始化滚转角的PID控制器
    PID_Init(&pid_roll, 4.0f, 0.0f, 0.0f); // 设置Kp=4.0, Ki=0.0, Kd=0.0
    PID_SetLimits(&pid_roll, -4*PI, 4*PI); // 设置输出上限和下限为-4π到4π
    PID_SetSP(&pid_roll, 0.0f); // 设置滚转角的目标值为0度

    PID_Init(&pid_dot_roll, 10.0f, 10.0f, 0.0f); // 设置Kp=10.0, Ki=10.0, Kd=0.0
    PID_SetLimits(&pid_dot_roll, -40*PI, 40*PI); // 设置输出上限和下限为-40π到40π

    last_time = GetUs(); // 初始化last_time为当前时间
}


/**
 * @brief 控制模块进程函数 需要在while循环中调用
 */
void App_Control_Proc(void)
{
    PERIODIC(5); // 每5ms执行一次
    
    uint64_t current_time = GetUs();
    float deltaT = (current_time - last_time) * 1.0e-6f; // 将时间差转换为秒

    // ### 速度控制器 起始 #####

    // 设定前进速度的目标值为0
    PID_SetSP(&pid_dot_y, 0.0f);

    // 获取车轮转速
    float omega = (Get_Omega_L() + Get_Omega_R()) / 2.0f; // 取左右轮平均值

    // 获取当前滚转角和滚转角速度
    float roll = App_MPU6050_GetRoll() * D2R;
    float dot_roll = App_MPU6050_GetGx() * D2R;

    // 计算速度环的反馈值 dot_y
    float omega2 = (Lp+Rw)*dot_roll/Rw;
    float dot_y = (omega - omega2)*Rw;

    // 计算前进速度的PID输出，输出是前进加速度的参考值 ddot_y_ref
    float ddot_y_ref = PID_Compute(&pid_dot_y, dot_y);

    // 计算滚转角的参考值 roll_ref
    float roll_ref = -qatan(ddot_y_ref/g_in_NTU);

    // 设置滚转角的目标值
    PID_SetSP(&pid_roll, roll_ref);

    // ### 速度控制器 结束 #####

    // ### 滚转角控制器 起始 #####

    // 计算滚转角的PID输出，作为滚转角速度的目标值
    float target_dot_roll = PID_Compute(&pid_roll, roll);
    PID_SetSP(&pid_dot_roll, target_dot_roll);

    // 计算滚转角速度的PID输出
    float ddot_roll_ref = PID_Compute(&pid_dot_roll, dot_roll);

    // 逆解算
    float ddot_y_ref_rollloop = (Lp*ddot_roll_ref-g_in_NTU*qsin(roll))/qcos(roll)/Rw;

    // 积分环节
    if(deltaT > 5e-3f) // 只有deltaT大于5ms时才进行积分。第一次计算时不需要积分
    {
        omega_ref += ddot_y_ref_rollloop * deltaT; // 0.005s为采样周期
    }

    // 设置轮子角速度的目标值
    App_Motor_SetSP_L(omega_ref);
    App_Motor_SetSP_R(omega_ref);

    // ### 滚转角控制器 结束 #####

    // 更新时间
    last_time = current_time;
}

/**
 * @brief 重置控制模块
 */
void App_Control_Reset(void){
    // 复位暂存的值
    last_time = GetUs();
    omega_ref = 0.0f;

    // 重置PID控制器
    PID_Reset(&pid_dot_y);
    PID_Reset(&pid_roll);
    PID_Reset(&pid_dot_roll);
}