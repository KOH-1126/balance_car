#include "app_mpu6050.h"
#include "i2c.h"
#include "common.h"
#include "qmath.h"

#define MPU6050_ADDR (0xd0)
#define ALPHA (0.95238f)
#define DELTA_T (0.005f)
#define GYRO_SCALE_DPS (1.0f / 16.4f)
#define GYRO_CALIBRATION_SAMPLES (500U)

static float ax, ay, az; // 加速度计数据，单位为g
static float gx, gy, gz; // 陀螺仪数据，单位为°/s
static float temp; // 温度数据，单位为°C
static float roll=0.0f, pitch=0.0f, yaw=0.0f; // 欧拉角，单位为角度
static float roll_acc=0.0f,pitch_acc=0.0f; // 由加速度计计算的欧拉角
static float gx_bias_raw=0.0f, gy_bias_raw=0.0f, gz_bias_raw=0.0f;
// static float dot_roll=0.0f,dot_pitch=0.0f,dot_yaw=0.0f; // 由陀螺仪计算的欧拉角变化率

static void reg_write(uint8_t reg, uint8_t data);
static int8_t read_motion_raw(int16_t *ax_raw, int16_t *ay_raw,
                              int16_t *az_raw, int16_t *temp_raw,
                              int16_t *gx_raw, int16_t *gy_raw,
                              int16_t *gz_raw);

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
 * @brief 从0x3B开始一次性读取完整的加速度、温度和陀螺仪原始数据
 * @return 0表示读取成功，-1表示I2C通信失败
 */
static int8_t read_motion_raw(int16_t *ax_raw, int16_t *ay_raw,
                              int16_t *az_raw, int16_t *temp_raw,
                              int16_t *gx_raw, int16_t *gy_raw,
                              int16_t *gz_raw)
{
    uint8_t start_reg = 0x3B;
    uint8_t data[14];

    if (My_I2C_SendBytes(&hi2c1, MPU6050_ADDR, &start_reg, 1) != 0) return -1;
    if (My_I2C_ReceiveBytes(&hi2c1, MPU6050_ADDR, data, sizeof(data)) != 0) return -1;

    *ax_raw = (int16_t)(((uint16_t)data[0] << 8) | data[1]);
    *ay_raw = (int16_t)(((uint16_t)data[2] << 8) | data[3]);
    *az_raw = (int16_t)(((uint16_t)data[4] << 8) | data[5]);
    *temp_raw = (int16_t)(((uint16_t)data[6] << 8) | data[7]);
    *gx_raw = (int16_t)(((uint16_t)data[8] << 8) | data[9]);
    *gy_raw = (int16_t)(((uint16_t)data[10] << 8) | data[11]);
    *gz_raw = (int16_t)(((uint16_t)data[12] << 8) | data[13]);
    return 0;
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
    reg_write(0x1A, 0x03); // DLPF：陀螺仪约44Hz，加速度计约42Hz

    HAL_Delay(300); // 等待传感器和数字滤波器初步稳定
    App_MPU6050_CalibrateGyro();

    /* 先取得一帧有效数据，再用加速度计角度初始化融合角。 */
    App_MPU6050_Update();

    roll  = roll_acc;
    pitch = pitch_acc;
    yaw = 0.0f;
}

/**
 * @brief 上电静止校准陀螺仪零偏
 * @note 调用期间必须保持车体完全静止，耗时约1秒。
 */
void App_MPU6050_CalibrateGyro(void)
{
    int64_t gx_sum = 0;
    int64_t gy_sum = 0;
    int64_t gz_sum = 0;
    uint32_t valid_samples = 0;

    for (uint32_t i = 0; i < GYRO_CALIBRATION_SAMPLES; ++i) {
        int16_t ax_raw, ay_raw, az_raw, temp_raw;
        int16_t gx_raw, gy_raw, gz_raw;

        if (read_motion_raw(&ax_raw, &ay_raw, &az_raw, &temp_raw,
                            &gx_raw, &gy_raw, &gz_raw) == 0) {
            gx_sum += gx_raw;
            gy_sum += gy_raw;
            gz_sum += gz_raw;
            ++valid_samples;
        }
        HAL_Delay(2);
    }

    if (valid_samples > 0U) {
        gx_bias_raw = (float)gx_sum / (float)valid_samples;
        gy_bias_raw = (float)gy_sum / (float)valid_samples;
        gz_bias_raw = (float)gz_sum / (float)valid_samples;
    }
}

void App_MPU6050_Update(void){
    int16_t ax_raw, ay_raw, az_raw, temp_raw;
    int16_t gx_raw, gy_raw, gz_raw;

    if (read_motion_raw(&ax_raw, &ay_raw, &az_raw, &temp_raw,
                        &gx_raw, &gy_raw, &gz_raw) != 0) {
        return; // 本帧通信失败时保留上一帧数据
    }

    // 将原始数据转换为实际物理值 ax ay az
    ax = ax_raw * 6.1035e-5f;
    ay = ay_raw * 6.1035e-5f;
    az = az_raw * 6.1035e-5f;
    gx = ((float)gx_raw - gx_bias_raw) * GYRO_SCALE_DPS;
    gy = ((float)gy_raw - gy_bias_raw) * GYRO_SCALE_DPS;
    gz = ((float)gz_raw - gz_bias_raw) * GYRO_SCALE_DPS;
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
