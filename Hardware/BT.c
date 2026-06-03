#include "stm32f10x.h"
#include "BT.h"
#include <stdio.h>
#include <stdarg.h>

char Serial_RxPacket[8];
uint8_t Serial_RxFlag;
char Serial_TxPacket[17];

void BT_Serial_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 打开 GPIOA 和 USART1 时钟（USART1 属于 APB2 外设）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 配置 TX (PA9) -> 复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 RX (PA10) -> 浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 USART 参数
    USART_InitStructure.USART_BaudRate = BT_USART_BaudRate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    // 使能接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 使能 USART1
    USART_Cmd(USART1, ENABLE);

    // NVIC 中断配置
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void BT_Send_Byte(uint8_t data)
{
    USART_SendData(USART1, data);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void BT_Send_String(uint8_t* ucstr)
{
    while (ucstr && *ucstr)
    {
        BT_Send_Byte(*ucstr++);
    }
}

void BT_SendArray(uint8_t* Array, uint16_t Length)
{
    for (uint16_t i = 0; i < Length; i++)
    {
        BT_Send_Byte(Array[i]);
    }
}

//==================== USART1 中断服务函数 ====================//
void USART1_IRQHandler(void)
{
    static uint8_t flag = 0;
    static uint8_t pRxPacket = 0;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        uint8_t RxData = USART_ReceiveData(USART1);

        if (flag == 0)
        {
            if (RxData == '0') // 数据帧起始符
            {
                flag = 1;
                pRxPacket = 0;
            }
        }
        else if (flag == 1)
        {
            if (RxData == '1') // 数据帧结束符
            {
                flag = 0;
                Serial_RxPacket[pRxPacket] = '\0';
                Serial_RxFlag = 1; // 接收完成
            }
            else
            {
                if (pRxPacket < sizeof(Serial_RxPacket) - 1)
                {
                    Serial_RxPacket[pRxPacket++] = RxData;
                }
            }
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
