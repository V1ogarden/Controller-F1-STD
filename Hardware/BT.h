//#ifndef __SERIAL_H__
//#define __SERIAL_H__

//#include "stm32f10x.h" 
//#include <stdint.h>

//extern char Serial_RxPacket[];

//extern char Serial_TxPacket[];

////-------------------- USART1 配置 --------------------
//#define BT_USART_RCC                RCC_APB2Periph_USART1
//#define BT_USART_GPIO_RCC           RCC_APB2Periph_GPIOA

//#define BT_USARTx                   USART1
//#define BT_USART_TX_PORT            GPIOA
//#define BT_USART_TX_PIN             GPIO_Pin_9      // TX = PA9
//#define BT_USART_RX_PORT            GPIOA
//#define BT_USART_RX_PIN             GPIO_Pin_10     // RX = PA10
//#define BT_USART_BaudRate           115200

////-------------------- 函数声明 --------------------
//void BT_Serial_Init(void);
//void BT_Send_Byte(uint8_t data);
//void BT_Send_String(uint8_t *ucstr);
//void BT_SendArray(uint8_t *Array, uint16_t Length);

//#endif /* __SERIAL_H__ */

#ifndef __BT_H__
#define __BT_H__

#include "stm32f10x.h" 
#include <stdint.h>

//-------------------- 暴露给外部(main.c)使用的变量 --------------------
extern char Serial_RxPacket[8];
extern uint8_t Serial_RxFlag;    // 新增：接收完成标志位，极其重要
extern char Serial_TxPacket[17];

//-------------------- USART1 & DMA 配置宏 --------------------
#define BT_USART_RCC            RCC_APB2Periph_USART1
#define BT_USART_GPIO_RCC       RCC_APB2Periph_GPIOA
#define BT_DMA_RCC              RCC_AHBPeriph_DMA1   // 新增：DMA1 时钟宏

#define BT_USARTx               USART1
#define BT_USART_TX_PORT        GPIOA
#define BT_USART_TX_PIN         GPIO_Pin_9      // TX = PA9
#define BT_USART_RX_PORT        GPIOA
#define BT_USART_RX_PIN         GPIO_Pin_10     // RX = PA10
#define BT_USART_BaudRate       115200

// 新增：DMA 通道宏定义 (USART1_TX 固定对应 DMA1_Channel4)
#define BT_TX_DMA_CHANNEL       DMA1_Channel4
#define BT_TX_DMA_FLAG_TC       DMA1_FLAG_TC4

//-------------------- 函数声明 --------------------
void BT_Serial_Init(void);

// 核心升级：替换掉以前所有的死等发送函数，统一使用零阻塞的 DMA 发送
void BT_SendArray_DMA(uint8_t *Array, uint16_t Length);

#endif
