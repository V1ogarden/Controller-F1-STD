#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "stm32f10x.h" 
#include <stdint.h>

extern char Serial_RxPacket[];

extern char Serial_TxPacket[];

//-------------------- USART1 ÅäÖÃ --------------------
#define BT_USART_RCC                RCC_APB2Periph_USART1
#define BT_USART_GPIO_RCC           RCC_APB2Periph_GPIOA

#define BT_USARTx                   USART1
#define BT_USART_TX_PORT            GPIOA
#define BT_USART_TX_PIN             GPIO_Pin_9      // TX = PA9
#define BT_USART_RX_PORT            GPIOA
#define BT_USART_RX_PIN             GPIO_Pin_10     // RX = PA10
#define BT_USART_BaudRate           115200

//-------------------- º¯ÊýÉùÃ÷ --------------------
void BT_Serial_Init(void);
void BT_Send_Byte(uint8_t data);
void BT_Send_String(uint8_t *ucstr);
void BT_SendArray(uint8_t *Array, uint16_t Length);

#endif /* __SERIAL_H__ */
