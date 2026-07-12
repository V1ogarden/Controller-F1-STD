#ifndef __INPUTS_H
#define __INPUTS_H

#include "stm32f10x.h"

//// 网络名 -> GPIO端口 映射
//#define MODE_PIN_PORT          GPIOB
//#define CHOOSER_PIN_PORT       GPIOB
//#define RIGHT_PIN_PORT         GPIOA
//#define TURN_RIGHT_PIN_PORT    GPIOA
//#define BALANCE_PIN_PORT       GPIOB
//#define BACK_PIN_PORT          GPIOB
//#define LEFT_PIN_PORT          GPIOB
//#define FRONT_PIN_PORT         GPIOB  
//#define TURN_LEFT_PIN_PORT     GPIOB
//#define UP_PIN_PORT            GPIOB
//#define CAT_UP_PIN_PORT        GPIOA
//#define OPEN_PIN_PORT          GPIOA
//#define CAT_DOWN_PIN_PORT      GPIOA
//#define CLOSE_PIN_PORT         GPIOA
//#define DOWN_PIN_PORT          GPIOA
//#define MID_PIN_PORT           GPIOA

//// 网络名 -> 引脚号 映射（对应GPIO_Pin_x）
//#define MODE_PIN           GPIO_Pin_12
//#define CHOOSER_PIN        GPIO_Pin_13
//#define RIGHT_PIN          GPIO_Pin_11
//#define TURN_RIGHT_PIN     GPIO_Pin_12
//#define BALANCE_PIN        GPIO_Pin_3
//#define BACK_PIN           GPIO_Pin_4
//#define LEFT_PIN           GPIO_Pin_5
//#define FRONT_PIN          GPIO_Pin_8
//#define TURN_LEFT_PIN      GPIO_Pin_9
//#define UP_PIN             GPIO_Pin_0
//#define CAT_UP_PIN         GPIO_Pin_7
//#define OPEN_PIN           GPIO_Pin_6
//#define CAT_DOWN_PIN       GPIO_Pin_5
//#define CLOSE_PIN          GPIO_Pin_4
//#define DOWN_PIN           GPIO_Pin_3
//#define MID_PIN            GPIO_Pin_1

// 枚举定义所有引脚（作为数组索引，顺序与引脚信息数组对应）
typedef enum {
    MODE_PIN,
    RIGHT_PIN,
    TURN_RIGHT_PIN,
    BALANCE_PIN,
    BACK_PIN,
    LEFT_PIN,
    FRONT_PIN,
    TURN_LEFT_PIN,
    UP_PIN,
    CAT_UP_PIN,
    OPEN_PIN,
    CAT_DOWN_PIN,
    CLOSE_PIN,
    DOWN_PIN,
    MID_PIN,
    PIN_NUM  // 引脚总数（用于数组大小）
} PinName;

// 结构体定义引脚硬件信息（端口+引脚号）
typedef struct 
{
    GPIO_TypeDef* port;  // GPIO端口（如GPIOA、GPIOB）
    uint16_t pin;        // 引脚号（如GPIO_Pin_12）
	char arm_cmd[5];		 //指令
} PinInfo;

//引脚状态机结构体
typedef struct 
{
	uint8_t read;
	uint8_t rd_step;
	uint8_t tap_flag;
	uint8_t rls_flag;
}pinRdInfo;

extern const PinInfo pinInfo[PIN_NUM];

extern volatile pinRdInfo keys_rd[PIN_NUM];

void inputs_init(void);

void PINs_Config(void); 

void TIM2_10ms_Init(void);



#endif
