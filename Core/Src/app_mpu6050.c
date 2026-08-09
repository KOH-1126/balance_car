#include "app_mpu6050.h"
#include "i2c.h"

#define MPU6050_ADDR (0xd0)

static float ax, ay, az; // 加速度计数据，单位为g
static float gx, gy, gz; // 陀螺仪数据，单位为°/s
static float temp; // 温度数据，单位为°C

static void reg_write(uint8_t reg, uint8_t data);
static uint8_t reg_read(uint8_t reg);

/**
 * @brief 向寄存器写值，一次写入一个字节
 * @param reg 寄存器地址
 * @param data 要写入的数据
 */
static void reg_write(uint8_t reg, uint8_t data)
{
    uint8_t byte2Send[2] = {reg, data};
    My_I2C_SendBytes(&hi2c1, MPU6050_ADDR, byte2Send, 2);
}

/**
 * @brief 从寄存器读值，一次读取一个字节
 * @param reg 寄存器地址
 * @return 寄存器的值
 */
static uint8_t reg_read(uint8_t reg)
{
    My_I2C_SendBytes(&hi2c1, MPU6050_ADDR, &reg, 1); // 发送寄存器地址
    uint8_t data;
    My_I2C_ReceiveBytes(&hi2c1, MPU6050_ADDR, &data, 1); // 接收寄存器值
    return data;
}

void App_MPU6050_Init(void){
    reg_write(0x6B, 0x80); //复位
    HAL_Delay(100); // 等待复位完成

    reg_write(0x6B, 0x00); // 唤醒MPU6050
    reg_write(0x1B, 0x18); // 设置陀螺仪量程为±2000°/s
    reg_write(0x1C, 0x00); // 设置加速度计量程为±2g
}

void App_MPU6050_Update(void){
    int16_t ax_raw = (int16_t)((reg_read(0x3B) << 8) + reg_read(0x3C)); // 读取加速度计X轴数据
    int16_t ay_raw = (int16_t)((reg_read(0x3D) << 8) + reg_read(0x3E)); // 读取加速度计Y轴数据
    int16_t az_raw = (int16_t)((reg_read(0x3F) << 8) + reg_read(0x40)); // 读取加速度计Z轴数据
    int16_t gx_raw = (int16_t)((reg_read(0x43) << 8) + reg_read(0x44)); // 读取陀螺仪X轴数据
    int16_t gy_raw = (int16_t)((reg_read(0x45) << 8) + reg_read(0x46)); // 读取陀螺仪Y轴数据
    int16_t gz_raw = (int16_t)((reg_read(0x47) << 8) + reg_read(0x48)); // 读取陀螺仪Z轴数据
    int16_t temp_raw = (int16_t)((reg_read(0x41) << 8) + reg_read(0x42)); // 读取温度数据

    // 将原始数据转换为实际物理值
    ax = ax_raw * 6.1035e-5f;
    ay = ay_raw * 6.1035e-5f;
    az = az_raw * 6.1035e-5f;
    gx = gx_raw * 6.1035e-2f;
    gy = gy_raw * 6.1035e-2f;
    gz = gz_raw * 6.1035e-2f;
    temp = temp_raw/333.87f + 21.0f; // 温度转换公式
}

float App_MPU6050_GetAx(void){
    return ax;

}
float App_MPU6050_GetAy(void){
    return ay;
}
float App_MPU6050_GetAz(void){
    return az;
}
float App_MPU6050_GetGx(void){
    return gx;
}
float App_MPU6050_GetGy(void){
    return gy;
}
float App_MPU6050_GetGz(void){
    return gz;
}
float App_MPU6050_GetTemp(void){
    return temp;
}