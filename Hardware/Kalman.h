#pragma once
#include <stdint.h>

/**
 * 用户需实现：从 MPU6050 读取原始数据
 * AccX/Y/Z: 加速度计原始值 (LSB)
 * GyroX/Y/Z: 陀螺仪原始值 (LSB)
 */
//void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
//                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);

/**
 * 初始化 IMU 姿态解算
 * @param roll0_deg  初始横滚角 (°)
 * @param pitch0_deg 初始俯仰角 (°)
 */
void IMU_Init(float roll0_deg, float pitch0_deg);

/**
 * 更新 IMU 姿态解算，必须周期调用
 * @param dt_s  采样周期 (秒)
 */
void IMU_Update(float dt_s);

/**
 * 获取当前欧拉角 (°)
 * @param roll_deg   输出 Roll
 * @param pitch_deg  输出 Pitch
 * @param yaw_deg    输出 Yaw
 */
void IMU_GetEuler(float *roll_deg, float *pitch_deg, float *yaw_deg);

/**
 * 获取当前三轴加速度 (g)
 * @param ax_g  输出 X 轴加速度
 * @param ay_g  输出 Y 轴加速度
 * @param az_g  输出 Z 轴加速度
 */
void IMU_GetAccel(float *ax_g, float *ay_g, float *az_g);
