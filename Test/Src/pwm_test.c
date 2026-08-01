#include "pwm_test.h"
#include "app.pwm.h"

/**
* @brief 测试pwm功能
* 先使能电机
* 正转2s 30%
* 正转2s 60%
* 正转2s 90%
* 关闭电机
* 反转
*/
void PWM_Test(void)
{
    App_PWM_Cmd(1); // 使能电机
    App_PWM_SetDuty_L(30.0f); // 设置左电机占空比为30%
    App_PWM_SetDuty_R(30.0f); // 设置右电机占空比为30%
    HAL_Delay(2000); // 延时2s
    App_PWM_SetDuty_L(60.0f); // 设置左电机占空比为60%
    App_PWM_SetDuty_R(60.0f); // 设置右电机占空比为60%
    HAL_Delay(2000); // 延时2s
    App_PWM_SetDuty_L(90.0f); // 设置左电机占空比为90%
    App_PWM_SetDuty_R(90.0f); // 设置右电机占空比为90%
    HAL_Delay(2000); // 延时2s
    App_PWM_Cmd(0); // 关闭电机

    App_PWM_Cmd(1); // 使能电机
    App_PWM_SetDuty_L(-30.0f); // 设置左电机占空比为-30%
    App_PWM_SetDuty_R(-30.0f); // 设置右电机占空比为-30%
    HAL_Delay(2000); // 延时2s
    App_PWM_SetDuty_L(-60.0f); // 设置左电机占空比为-60%
    App_PWM_SetDuty_R(-60.0f); // 设置右电机占空比为-60%
    HAL_Delay(2000); // 延时2s
    App_PWM_SetDuty_L(-90.0f); // 设置左电机占空比为-90%
    App_PWM_SetDuty_R(-90.0f); // 设置右电机占空比为-90%
    HAL_Delay(2000); // 延时2s
    App_PWM_Cmd(0); // 关闭电机

    while(1){

    }
}
