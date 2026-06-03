#include "Kalman.h"
#include <stdlib.h>
#include <math.h>
#include "MPU6050.h"

#define RAD2DEG   57.29577951308232f
#define DEG2RAD   0.017453292519943295f

typedef struct {
    float Q_angle, Q_bias, R_measure;
    float angle, bias;
    float P[2][2];
    float rate;
} Kalman_t;

typedef struct {
    Kalman_t kx, ky;
    float compRoll, compPitch;
    float yaw;
    float ax, ay, az;
} IMU_t;

static IMU_t imu;

// Kalman 单轴更新
static float Kalman_Update(Kalman_t *k, float newAngle, float newRate, float dt) {
    k->rate  = newRate - k->bias;
    k->angle += dt * k->rate;
    k->P[0][0] += dt * (dt*k->P[1][1] - k->P[0][1] - k->P[1][0] + k->Q_angle);
    k->P[0][1] -= dt * k->P[1][1];
    k->P[1][0] -= dt * k->P[1][1];
    k->P[1][1] += k->Q_bias * dt;

    float S  = k->P[0][0] + k->R_measure;
    float K0 = k->P[0][0] / S;
    float K1 = k->P[1][0] / S;

    float y = newAngle - k->angle;
    k->angle += K0 * y;
    k->bias  += K1 * y;

    float P00 = k->P[0][0], P01 = k->P[0][1];
    k->P[0][0] -= K0 * P00;
    k->P[0][1] -= K0 * P01;
    k->P[1][0] -= K1 * P00;
    k->P[1][1] -= K1 * P01;

    return k->angle;
}

// 加速度计算 Roll/Pitch
static void Accel_To_RP(float ax, float ay, float az, float *pRoll, float *pPitch) {
    *pPitch = atan2f(-ax, sqrtf(ay*ay + az*az)) * RAD2DEG;
    *pRoll  = atan2f(ay, az) * RAD2DEG;
}

void IMU_Init(float roll0_deg, float pitch0_deg) {
    imu.kx.Q_angle = 0.001f; imu.kx.Q_bias = 0.003f; imu.kx.R_measure = 0.03f;
    imu.ky.Q_angle = 0.001f; imu.ky.Q_bias = 0.003f; imu.ky.R_measure = 0.03f;

    imu.kx.angle = roll0_deg; imu.kx.bias = 0;
    imu.ky.angle = pitch0_deg; imu.ky.bias = 0;
    imu.kx.P[0][0]=imu.kx.P[0][1]=imu.kx.P[1][0]=imu.kx.P[1][1]=0;
    imu.ky.P[0][0]=imu.ky.P[0][1]=imu.ky.P[1][0]=imu.ky.P[1][1]=0;

    imu.compRoll = roll0_deg;
    imu.compPitch = pitch0_deg;
    imu.yaw = 0.0f;

    imu.ax = imu.ay = imu.az = 0.0f;
}

void IMU_Update(float dt_s) {
    int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
    MPU6050_GetData(&ax_raw, &ay_raw, &az_raw, &gx_raw, &gy_raw, &gz_raw);

    const float Accel_Scale = 1.0f/16384.0f;
    const float Gyro_Scale  = DEG2RAD/131.0f;

    float ax = ax_raw * Accel_Scale;
    float ay = ay_raw * Accel_Scale;
    float az = az_raw * Accel_Scale;
    float gx_dps = gx_raw * Gyro_Scale * RAD2DEG;
    float gy_dps = gy_raw * Gyro_Scale * RAD2DEG;
    float gz_dps = gz_raw * Gyro_Scale * RAD2DEG;

    imu.ax = ax;
    imu.ay = ay;
    imu.az = az;

    float roll_acc, pitch_acc;
    Accel_To_RP(ax, ay, az, &roll_acc, &pitch_acc);

    Kalman_Update(&imu.kx, roll_acc, gx_dps, dt_s);
    Kalman_Update(&imu.ky, pitch_acc, gy_dps, dt_s);

    const float alpha = 0.98f;
    imu.compRoll  = alpha * (imu.compRoll  + gx_dps * dt_s) + (1.0f-alpha) * roll_acc;
    imu.compPitch = alpha * (imu.compPitch + gy_dps * dt_s) + (1.0f-alpha) * pitch_acc;

    imu.yaw += gz_dps * dt_s;
}

void IMU_GetEuler(float *roll_deg, float *pitch_deg, float *yaw_deg) {
    *roll_deg  = imu.kx.angle;
    *pitch_deg = imu.ky.angle;
    *yaw_deg   = imu.yaw;
}

void IMU_GetAccel(float *ax_g, float *ay_g, float *az_g) {
    *ax_g = imu.ax;
    *ay_g = imu.ay;
    *az_g = imu.az;
}
