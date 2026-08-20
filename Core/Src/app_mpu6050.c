#include "app_mpu6050.h"
#include "i2c.h"
#include "common.h"
#include "qmath.h"

#define MPU6050_ADDR (0xd0)
#define ALPHA (0.95238f)
#define DELTA_T (0.005f)
#define R2D (57.2958f) // 弧度转角度的系数
#define D2R (0.0174533f) // 角度转弧度的系数
// #define g_in_NTU (9.78f) // 1g在NTU的加速度计输出值，单位为g

static float ax, ay, az; // 加速度计数据，单位为g
static float gx, gy, gz; // 陀螺仪数据，单位为°/s
static float temp; // 温度数据，单位为°C
static float roll=0.0f, pitch=0.0f, yaw=0.0f; // 欧拉角，单位为角度
static float roll_acc=0.0f,pitch_acc=0.0f; // 由加速度计计算的欧拉角
// static float dot_roll=0.0f,dot_pitch=0.0f,dot_yaw=0.0f; // 由陀螺仪计算的欧拉角变化率

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

/**
 * @brief MPU6050进程函数
 */
void App_MPU6050_Proc(void)
{
    PERIODIC(5); // 每5ms执行一次
    App_MPU6050_Update();
}


void App_MPU6050_Init(void){
    reg_write(0x6B, 0x80); //复位
    HAL_Delay(100); // 等待复位完成

    reg_write(0x6B, 0x00); // 唤醒MPU6050
    reg_write(0x1B, 0x18); // 设置陀螺仪量程为±2000°/s
    reg_write(0x1C, 0x00); // 设置加速度计量程为±2g

    roll_acc = qatan2(ay, az) * R2D; // 转换为角度
    pitch_acc = -qatan2(ax, sqrtf(ay * ay + az * az)) * R2D; // 转换为角度

    roll  = roll_acc;
    pitch = pitch_acc;
}

void App_MPU6050_Update(void){
    int16_t ax_raw = (int16_t)((reg_read(0x3B) << 8) + reg_read(0x3C)); // 读取加速度计X轴数据
    int16_t ay_raw = (int16_t)((reg_read(0x3D) << 8) + reg_read(0x3E)); // 读取加速度计Y轴数据
    int16_t az_raw = (int16_t)((reg_read(0x3F) << 8) + reg_read(0x40)); // 读取加速度计Z轴数据
    int16_t gx_raw = (int16_t)((reg_read(0x43) << 8) + reg_read(0x44)); // 读取陀螺仪X轴数据
    int16_t gy_raw = (int16_t)((reg_read(0x45) << 8) + reg_read(0x46)); // 读取陀螺仪Y轴数据
    int16_t gz_raw = (int16_t)((reg_read(0x47) << 8) + reg_read(0x48)); // 读取陀螺仪Z轴数据
    int16_t temp_raw = (int16_t)((reg_read(0x41) << 8) + reg_read(0x42)); // 读取温度数据

    // 将原始数据转换为实际物理值 ax ay az
    ax = ax_raw * 6.1035e-5f;
    ay = ay_raw * 6.1035e-5f;
    az = az_raw * 6.1035e-5f;
    gx = gx_raw * 6.1035e-2f;
    gy = gy_raw * 6.1035e-2f;
    gz = gz_raw * 6.1035e-2f;
    temp = temp_raw/333.87f + 21.0f; // 温度转换公式

    // 1. 单纯由陀螺仪数据计算欧拉角
    // 不化简的版本
    // dot_roll = gx + (gz*qcos(roll*D2R) + gy*qsin(roll*D2R))*qtan(pitch*D2R);
    // dot_pitch = gy*qcos(roll*D2R) - gz*qsin(roll*D2R);
    // dot_yaw = (gz*qcos(roll*D2R) + gy*qsin(roll*D2R))/qcos(pitch*D2R);
    // roll += dot_roll * DELTA_T;
    // pitch += dot_pitch * DELTA_T;
    // yaw += dot_yaw * DELTA_T;

    // 2. 由加速度计数据计算欧拉角
    
    roll_acc = qatan2(ay, az) * R2D; // 转换为角度

    //俯仰角在实际运行过程中应该一直为0
    pitch_acc = -qatan2(ax, sqrtf(ay * ay + az * az)) * R2D; // 转换为角度

    // 3. 互补融合滤波
    roll = ALPHA * (roll + gx * DELTA_T) + (1 - ALPHA) * roll_acc; // 这一行的效果最好，只要theta=0就可以，正常运行过程中就是这样的
    pitch = ALPHA * (pitch + gy * DELTA_T) + (1 - ALPHA) * pitch_acc; // 这一行的效果一般，要求phi很小接近0
    yaw += gz * DELTA_T; // 这一行的效果一般，要求phi很小接近0；而且陀螺仪有漂移，加速度计又不能测偏航角。因此无法通过互补滤波来修正。
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

float App_MPU6050_GetRoll(void){
    return roll;
}

float App_MPU6050_GetPitch(void){
    return pitch;
}

float App_MPU6050_GetYaw(void){
    return yaw;
}