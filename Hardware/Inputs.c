#include "stm32f10x.h"
#include "Inputs.h"

// 在 Inputs.c 中修改为这种写法：
const PinInfo pinInfo[PIN_NUM] = {
    // 这样写，即使 Inputs.h 中的枚举顺序变了，也绝对不会串位！
    [MODE_PIN]       = {GPIOB, GPIO_Pin_12, "bTe"},
    [RIGHT_PIN]      = {GPIOA, GPIO_Pin_11, "bre"},
    [LEFT_PIN]       = {GPIOB, GPIO_Pin_5,  "ble"},
    [FRONT_PIN]      = {GPIOB, GPIO_Pin_8,  "bfe"},
    [BACK_PIN]       = {GPIOB, GPIO_Pin_4,  "bbe"},
    [UP_PIN]         = {GPIOB, GPIO_Pin_0,  "bue"},
    [DOWN_PIN]       = {GPIOA, GPIO_Pin_3,  "bde"},
    [TURN_RIGHT_PIN] = {GPIOA, GPIO_Pin_12, "bRe"},
    [TURN_LEFT_PIN]  = {GPIOB, GPIO_Pin_9,  "bLe"},
    [CAT_UP_PIN]     = {GPIOA, GPIO_Pin_7,  "bUe"},
    [CAT_DOWN_PIN]   = {GPIOA, GPIO_Pin_5,  "bDe"},
    [CLOSE_PIN]      = {GPIOA, GPIO_Pin_4,  "bCe"},
    [OPEN_PIN]       = {GPIOA, GPIO_Pin_6,  "bOe"},
    [MID_PIN]        = {GPIOA, GPIO_Pin_1,  "bme"},
};

// 引脚状态机数组（保存读取信息和消抖和松手过程）
volatile pinRdInfo keys_rd[PIN_NUM] = {1,0,0,0};  // 0=低电平，1=高电平

/**
 * 初始化所有引脚为上拉输入模式
 */

void inputs_init(void)
{
	PINs_Config();
	
	TIM2_10ms_Init();
}

void PINs_Config(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 1. 使能所需时钟（GPIOA、GPIOB、AFIO）
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE );
    
    // 2. 释放JTAG复用引脚（PB3、PB4）
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);    // 禁用PB4的JTRST
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // 禁用PB3的JTDO
    
    // 3. 通用配置：上拉输入
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
    // 4. 循环配置所有引脚（通过枚举和数组批量处理）
    for (PinName pin = 0; pin < PIN_NUM; pin++) 
		{
        GPIO_InitStructure.GPIO_Pin = pinInfo[pin].pin;
        GPIO_Init(pinInfo[pin].port, &GPIO_InitStructure);
    }
		
}

/**
 * 初始化TIM2，每10ms产生一次更新中断
 */
void TIM2_10ms_Init(void) 
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 使能TIM2时钟（APB1总线）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 时基配置（72MHz时钟 → 10ms中断）
    TIM_TimeBaseStructure.TIM_Prescaler = 7199;    // 分频后10kHz
    TIM_TimeBaseStructure.TIM_Period = 99;         // 10kHz×100=100Hz → 10ms
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // 使能更新中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    // 配置NVIC中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

/**
 * TIM2中断服务函数：每10ms触发一次，读取所有引脚电平并保存到变量
 */
void TIM2_IRQHandler(void) 
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) 
		{
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // 清除中断标志
        
        
        for (PinName pin = 0; pin < PIN_NUM; pin++) 
				{
						// 循环读取所有引脚电平，保存到状态数组
            keys_rd[pin].read = GPIO_ReadInputDataBit(pinInfo[pin].port,pinInfo[pin].pin);
					
					
            switch (keys_rd[pin].rd_step)
            {
                case 0: // 初始状态，等待按下
                    if (keys_rd[pin].read == 0) {
                        keys_rd[pin].rd_step = 1;
                    }
                    break;
                    
                case 1: // 按下消抖确认
                    if (keys_rd[pin].read == 0) {
                        keys_rd[pin].tap_flag = 1;
                        keys_rd[pin].rd_step = 2;
                    } else {
                        keys_rd[pin].rd_step = 0; // 抖动，回到初始状态
                    }
                    break;
                    
                case 2: // 等待松开
                    if (keys_rd[pin].read == 1) {
                        // 注意：这里其实也建议加一个松开消抖（参考下文第2点）
                        keys_rd[pin].rls_flag = 1;
                        keys_rd[pin].rd_step = 3;
                    }
                    break;
                    
                case 3: // 等待主程序清除松开标志
                    // 如果主程序还没处理完 rls_flag，保持在这里
                    // 如果处理完了（通常主循环会将 rls_flag 清 0），则回到 0
                    if (keys_rd[pin].rls_flag == 0) {
                        keys_rd[pin].rd_step = 0;
                    }
                    break;
            }
        }
		
				
    }
}
