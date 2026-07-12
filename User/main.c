#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "Kalman.h"
#include "BT.h"
#include "Inputs.h"
#include "Eula_TX.h"
#include <string.h>
#include <stdio.h> // 需要用到 sprintf

float roll, pitch, yaw;
float ax, ay, az;

char tx_start='b';
char tx_div='e';

uint8_t current_mode=0;
uint8_t target_mode=0;

char arm_stop[]="b0e";
char arm_mode_1[]="bTe";
char arm_mode_0[]="bte";

// 新增：全局变量，记录姿态数据帧的长度，供 TIM3 中断调用
uint16_t eula_tx_len = 0; 

int main(void)
{
    BT_Serial_Init();
    OLED_Init();
    MPU6050_Init();
    IMU_Init(0.0f, 0.0f);
    inputs_init();
    TIM3_Init();
    
    //ID = MPU6050_GetID();
    //OLED_ShowHexNum(1, 4, ID, 2);
    
    while (1)
    {
        IMU_Update(0.1f);   
        // 采样周期 10 ms 
        IMU_GetEuler(&roll, &pitch, &yaw);
        IMU_GetAccel(&ax, &ay, &az);
        
        if(keys_rd[MODE_PIN].read==1)
        {
            target_mode=0;
        }
        else if(keys_rd[MODE_PIN].read==0)
        {
            target_mode=1;
        }
        
        if (target_mode == 1)
        {
            int t_roll = (int)(roll + 0.5f);    // 四舍五入
            int t_pitch = (int)(pitch + 0.5f);
            int t_yaw = (int)(yaw + 0.5f);
            
            // 确保 DMA 发送空闲时再更新数组，防止内存踩踏
            if (DMA_GetFlagStatus(BT_TX_DMA_FLAG_TC) != RESET || DMA_GetCurrDataCounter(BT_TX_DMA_CHANNEL) == 0)
            {
                eula_tx_len = sprintf((char*)eula_tx, "i,%d,%d,%d,d", t_roll, t_pitch, t_yaw);
            }           
        }
        
        OLED_ShowString(1,1,(char*)eula_tx); 
        OLED_ShowString(2,1,"Controlling:");
        OLED_ShowString(3,1,"ArmBot");
        OLED_ShowString(4,1,"BatteryLevel:42%");
    
        for(PinName pin=0; pin<PIN_NUM; pin++)
        {               
            if(keys_rd[pin].tap_flag==1)
            {
                
                BT_SendArray_DMA((uint8_t*)pinInfo[pin].arm_cmd, strlen(pinInfo[pin].arm_cmd));
                keys_rd[pin].tap_flag=0;
            }
            
            else if(keys_rd[pin].rls_flag==1)
            {
                BT_SendArray_DMA((uint8_t*)arm_stop, strlen(arm_stop));
                keys_rd[pin].rls_flag=0;
            }
        }
        
        if (target_mode != current_mode)
        {                       
            if (target_mode == 1)
            {
                BT_SendArray_DMA((uint8_t*)arm_mode_1, strlen(arm_mode_1));  // 发送模式1通知
                TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
                TIM_Cmd(TIM3, ENABLE);  // 开启TIM3，触发欧拉角发送中断
            }
            else if (target_mode == 0)
            {
                TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
                TIM_Cmd(TIM3, DISABLE); // 关闭TIM3，停止欧拉角发送
                
                Delay_ms(10);
                BT_SendArray_DMA((uint8_t*)arm_mode_0, strlen(arm_mode_0));  // 发送模式0通知
            }
            
            current_mode = target_mode;  // 更新当前模式
        }
    }           
}
