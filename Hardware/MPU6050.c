#include "stm32f10x.h"
#include "MyI2C.h"
#include "MPU6050.h"


// 写寄存器
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS);
    MyI2C_ReceiveAck();
    MyI2C_SendByte(RegAddress);
    MyI2C_ReceiveAck();
    MyI2C_SendByte(Data);
    MyI2C_ReceiveAck();
    MyI2C_Stop();
}

// 读寄存器
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;

    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS);
    MyI2C_ReceiveAck();
    MyI2C_SendByte(RegAddress);
    MyI2C_ReceiveAck();

    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
    MyI2C_ReceiveAck();
    Data = MyI2C_ReceiveByte();
    MyI2C_SendAck(1);
    MyI2C_Stop();

    return Data;
}

// 初始化
void MPU6050_Init(void)
{
    MyI2C_Init();
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
    MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);
    MPU6050_WriteReg(MPU6050_CONFIG, 0x06);
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);   // ±2000°/s
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);  // ±16g
}

// 获取芯片ID
uint8_t MPU6050_GetID(void)
{
    return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

// 连续读N字节
void MPU6050_ReadBytes(uint8_t start_reg, uint8_t *buf, uint8_t len)
{
    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS);
    MyI2C_ReceiveAck();
    MyI2C_SendByte(start_reg);
    MyI2C_ReceiveAck();

    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
    MyI2C_ReceiveAck();

    for (uint8_t i = 0; i < len; i++)
    {
        buf[i] = MyI2C_ReceiveByte();
        if (i != len - 1)
            MyI2C_SendAck(0); // ACK
        else
            MyI2C_SendAck(1); // NACK
    }
    MyI2C_Stop();
}

// 获取加速度&陀螺仪数据（连续读取14字节）
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
    uint8_t buf[14];
    MPU6050_ReadBytes(MPU6050_ACCEL_XOUT_H, buf, 14);

    *AccX  = (int16_t)(buf[0]  << 8 | buf[1]);
    *AccY  = (int16_t)(buf[2]  << 8 | buf[3]);
    *AccZ  = (int16_t)(buf[4]  << 8 | buf[5]);
    *GyroX = (int16_t)(buf[8]  << 8 | buf[9]);
    *GyroY = (int16_t)(buf[10] << 8 | buf[11]);
    *GyroZ = (int16_t)(buf[12] << 8 | buf[13]);
}
