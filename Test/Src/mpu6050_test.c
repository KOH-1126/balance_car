#include "mpu6050_test.h"
#include "usart.h"
#include "app_usart2.h"
#include "app_mpu6050.h"
#include "common.h"
#include "qmath.h"

void MPU6050_Test()
{
    App_MPU6050_Init();
    while(1){
        App_MPU6050_Update();
        float ax = App_MPU6050_GetAx();
        float ay = App_MPU6050_GetAy();
        float az = App_MPU6050_GetAz();
        float gx = App_MPU6050_GetGx();
        float gy = App_MPU6050_GetGy();
        float gz = App_MPU6050_GetGz();
        float temp = App_MPU6050_GetTemp();

        My_USART_Printf(&huart2, "%f, %f, %f, %f, %f, %f, %f\r\n", ax, ay, az, gx, gy, gz, temp);
        HAL_Delay(10);
    }
}

static void USART2_Proc()
{
    PERIODIC(10)
    My_USART_Printf(&huart2, "%f, %f, %f, %f\r\n", App_MPU6050_GetRoll(), App_MPU6050_GetPitch(), App_MPU6050_GetYaw(), App_MPU6050_GetTemp());
} 

void MPU6050_EularAngleTest(void)
{
    App_MPU6050_Init();
    while(1)
    {
        App_MPU6050_Proc();
        USART2_Proc();
    }
}