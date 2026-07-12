#include "stm32f10x.h"
#include "Eula_TX.h"
#include "BT.h"
/**
 * 初始化定时器3（TIM3），每20ms产生一次更新中断
 * TIM3挂载在APB1总线，时钟频率为72MHz（APB1分频为2时，TIM3时钟=APB1时钟×2=36MHz×2=72MHz）
 */
 
char eula_tx[20];
 
void TIM3_Init(void) 
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Prescaler = 7199;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;   
    TIM_TimeBaseStructure.TIM_Period = 999;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;       
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
    // 3. 使能TIM3更新中断（计数器溢出时触发）
    //TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
    // 4. 配置NVIC中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;              // TIM3中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;     // 抢占优先级（可调整）
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;            // 子优先级（可调整）
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 5. 启动定时器
    //TIM_Cmd(TIM3, ENABLE);
}

/**
 * TIM3中断服务函数：每10ms触发一次，读取所有引脚状态并消抖
 */
void TIM3_IRQHandler(void) 
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) 
    {        

        BT_SendArray_DMA((uint8_t*)eula_tx, strlen((char*)eula_tx));
        
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  // 清除中断标志，避免重复触发
    }
}
