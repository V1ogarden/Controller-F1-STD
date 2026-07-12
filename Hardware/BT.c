//#include "stm32f10x.h"
//#include "BT.h"
//#include <stdio.h>
//#include <stdarg.h>

//char Serial_RxPacket[8];
//uint8_t Serial_RxFlag;
//char Serial_TxPacket[17];

//void BT_Serial_Init(void)
//{
//    GPIO_InitTypeDef GPIO_InitStructure;
//    USART_InitTypeDef USART_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

//    // 打开 GPIOA 和 USART1 时钟（USART1 属于 APB2 外设）
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

//    // 配置 TX (PA9) -> 复用推挽输出
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // 配置 RX (PA10) -> 浮空输入
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    // 配置 USART 参数
//    USART_InitStructure.USART_BaudRate = BT_USART_BaudRate;
//    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//    USART_InitStructure.USART_StopBits = USART_StopBits_1;
//    USART_InitStructure.USART_Parity = USART_Parity_No;
//    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
//    USART_Init(USART1, &USART_InitStructure);

//    // 使能接收中断
//    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

//    // 使能 USART1
//    USART_Cmd(USART1, ENABLE);

//    // NVIC 中断配置
//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//}

//void BT_Send_Byte(uint8_t data)
//{
//    USART_SendData(USART1, data);
//    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//}

//void BT_Send_String(uint8_t* ucstr)
//{
//    while (ucstr && *ucstr)
//    {
//        BT_Send_Byte(*ucstr++);
//    }
//}

//void BT_SendArray(uint8_t* Array, uint16_t Length)
//{
//    for (uint16_t i = 0; i < Length; i++)
//    {
//        BT_Send_Byte(Array[i]);
//    }
//}

////==================== USART1 中断服务函数 ====================//
//void USART1_IRQHandler(void)
//{
//    static uint8_t flag = 0;
//    static uint8_t pRxPacket = 0;

//    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
//    {
//        uint8_t RxData = USART_ReceiveData(USART1);

//        if (flag == 0)
//        {
//            if (RxData == '0') // 数据帧起始符
//            {
//                flag = 1;
//                pRxPacket = 0;
//            }
//        }
//        else if (flag == 1)
//        {
//            if (RxData == '1') // 数据帧结束符
//            {
//                flag = 0;
//                Serial_RxPacket[pRxPacket] = '\0';
//                Serial_RxFlag = 1; // 接收完成
//            }
//            else
//            {
//                if (pRxPacket < sizeof(Serial_RxPacket) - 1)
//                {
//                    Serial_RxPacket[pRxPacket++] = RxData;
//                }
//            }
//        }

//        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//    }
//}


#include "stm32f10x.h"
#include "BT.h"
#include <stdio.h>
#include <string.h>

char Serial_RxPacket[8];
uint8_t Serial_RxFlag;
char Serial_TxPacket[17];

void BT_Serial_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    // 1. 开启时钟：GPIOA, USART1, 以及 DMA1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // 2. 配置 GPIO 
    // TX (PA9) -> 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RX (PA10) -> 浮空输入 (既然要接收，必须把RX引脚配回来)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. 配置 USART 参数 (Tx 和 Rx 都开启)
    USART_InitStructure.USART_BaudRate = 115200; // 宏定义 BT_USART_BaudRate 也可以
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; 
    USART_Init(USART1, &USART_InitStructure);

    // 4. 配置 DMA1_Channel4 (仅针对 USART1_TX)
    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)Serial_TxPacket; 
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                // 内存到外设
    DMA_InitStructure.DMA_BufferSize = 0;                             
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;           
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                     
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);

    // 5. 开启 USART1 的 DMA 发送请求 (注意：不需要开 Rx 的 DMA 请求)
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

    // 6. 使能接收中断 (RXNE)
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 7. 配置 NVIC (响应接收中断)
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 8. 使能 USART1
    USART_Cmd(USART1, ENABLE);
}

// ==================== DMA 零阻塞发送函数 ==================== //
void BT_SendArray_DMA(uint8_t *Array, uint16_t Length)
{
    // 等待上一次发送完成，防止高频调用时数据被覆盖
    while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET && DMA_GetCurrDataCounter(DMA1_Channel4) != 0);
    
    DMA_ClearFlag(DMA1_FLAG_TC4); 
    DMA_Cmd(DMA1_Channel4, DISABLE); 
    
    // 重新指向要发送的数组地址和长度
    DMA1_Channel4->CMAR = (uint32_t)Array; 
    DMA1_Channel4->CNDTR = Length;         
    
    DMA_Cmd(DMA1_Channel4, ENABLE); 
}

// ==================== USART1 中断服务函数 (状态机接收) ==================== //
void USART1_IRQHandler(void)
{
    static uint8_t flag = 0;
    static uint8_t pRxPacket = 0;

    // 检查是否是接收中断
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t RxData = USART_ReceiveData(USART1); // 读取数据会自动清除 RXNE 标志位

        if (flag == 0) // 等待帧头
        {
            if (RxData == '0') // 数据帧起始符
            {
                flag = 1;
                pRxPacket = 0;
            }
        }
        else if (flag == 1) // 接收载荷与等待帧尾
        {
            if (RxData == '1') // 数据帧结束符
            {
                flag = 0;
                Serial_RxPacket[pRxPacket] = '\0'; // 补上字符串结束符
                Serial_RxFlag = 1;                 // 接收完成标志置1
            }
            else
            {
                // 防越界保护机制
                if (pRxPacket < sizeof(Serial_RxPacket) - 1)
                {
                    Serial_RxPacket[pRxPacket++] = RxData;
                }
            }
        }
    }
}