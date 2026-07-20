#include "bat_test.h"
#include "app_bat.h"
#include "app_usart2.h"
#include "usart.h"

/**
 * @brief 电池测试函数，向usart2发送电池电压值
 */
void Bat_Test(void)
{
    while(1){
        //每隔100ms发送一次电压值
        float volt = App_Bat_Get();
        My_USART_Printf(&huart2, "%.3f\n", volt);
        HAL_Delay(100);
    }
}
