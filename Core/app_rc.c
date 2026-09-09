#include "app_rc.h"
#include "usart.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include "app_control.h"

#define MaxCMDLength (64)

static char intBuffer[MaxCMDLength]; // 接收中断使用的缓冲区
static char transBuffer[MaxCMDLength]; //中断程序和进程函数间转运数据的缓冲区
static char procBuffer[MaxCMDLength]; // 进程函数使用的缓冲区
static uint8_t receivedByte;

static volatile uint8_t lineReceivedFlag = 0; // 一行字符串接收完成的标志位，0表示未完成，1表示完成

/**
  * @brief  RC initialization
  * @retval None
  */
void App_RC_Init(void)
{
    // 串口的初始化已经由MX完成

    // 打开串口接收中断
    HAL_UART_Receive_IT(&huart3, &receivedByte, 1);
}

void App_RC_Proc(void)
{
    if (lineReceivedFlag)
    {
        // 将接收到的数据从transBuffer复制到procBuffer，以便在主循环中处理
        strncpy(procBuffer, transBuffer, MaxCMDLength);
        lineReceivedFlag = 0; // 清除标志位
        // 解析字符串
        if(strncasecmp(procBuffer, "move ", 5) == 0){ // 如果是MOVE指令
            int8_t turnSpeed, moveSpeed;
            if(sscanf(procBuffer + 5, "%d%d", &turnSpeed, &moveSpeed) == 2){
                // 成功解析出两个整数
                // 在这里执行相应的操作，例如设置电机速度
                App_Control_SetMoveSpeed(-moveSpeed / 100.0f * 0.7f); // 将移动速度设置为百分比
            }
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if (huart->Instance == USART3)
    {
        static uint8_t index = 0; // 指向intBuffer的下一个空白位置
        /* 在这里处理 usart3_rx_byte */
        if(receivedByte != '\n' )
        {
            intBuffer[index++] = receivedByte;
        }
        else // 接收到换行符，表示一行数据接收完成
        {
            intBuffer[index] = '\0'; // 添加字符串结束符
            lineReceivedFlag = 1; // 设置标志位，表示一行数据接收完成
            index = 0; // 重置索引，为下一行数据做准备
            // 将接收到的数据从intBuffer复制到transBuffer，以便在主循环中处理
            strncpy(transBuffer, intBuffer, MaxCMDLength);
        }        


        /* 必须重新开启下一字节接收 */
        HAL_UART_Receive_IT(&huart3, &receivedByte, 1);
    }
}