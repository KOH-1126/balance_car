#include "encoder_test.h"
#include "app_encoder.h"
#include "app_usart2.h"

/**
 * @brief 编码器测试函数，向usart2发送左右编码器计数值
 */
void Encoder_Test(void)
{
    while(1){
        My_USART_Printf(&huart2, "%f, %f\n", Get_Position_L(), Get_Position_R());
        HAL_Delay(300); // 延时1秒
    }
}

/**
 * @brief T法测速测试函数，向usart2发送左右轮速度值
 */
void T_Measure_Test(void)
{
    while(1){
        My_USART_Printf(&huart2, "%f, %f, %f, %f\n", Get_Position_L(), Get_Position_R(), omega_L, omega_R);
        HAL_Delay(300); // 延时1秒
    }
}
