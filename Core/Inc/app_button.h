#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#include "stm32f1xx_hal.h"

// 通用按钮部分
typedef struct
{
    /* 初始化参数 */
    GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
	void (*button_pressed_cb)(void);
	void (*button_released_cb)(void);
	void (*button_clicked_cb)(uint8_t clicks);
	void (*button_long_pressed_cb)(uint8_t ticks);
	uint32_t LongPressThreshold;
	uint32_t LongPressTickInterval;
	uint32_t ClickInterval; 
	
	uint8_t  LastState;     // 按钮上次的状态，1 - 松开，0 - 按下
	uint8_t  ChangePending; // 按钮的状态是否正在发生改变
	uint32_t PendingTime;   // 按钮状态开始变化的时间
	
	uint32_t LastPressedTime;  // 按钮上次按下的时间
	uint32_t LastReleasedTime; // 按钮上次松开的时间
	
	uint8_t LongPressTicks; 	// 长按计数
	uint32_t LastLongPressTickTime; // 上次长按计数的时间
	
	uint8_t ClickCnt; // 连击计数
} Button_t;

void App_Button_Init(Button_t *Button, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void App_Button_Proc(Button_t *Button);
uint8_t Button_GetState(Button_t *Button);
void Button_ClickIntervalConfig(Button_t *Button, uint32_t Interval);
void Button_LongPressConfig(Button_t *Button, uint32_t Throshold, uint32_t TickInterval);
void Button_SetLongPressCb(Button_t *Button, void (*LongPressCb)(uint8_t ticks));
void Button_SetPressCb(Button_t *Button, void (*PressCb)(void));
void Button_SetReleaseCb(Button_t *Button, void (*ReleaseCb)(void));
void Button_SetClickCb(Button_t *Button, void (*ClickCb)(uint8_t clicks));

//用户按钮部分
void User_Key_Init(void);
void User_Key_Proc(void);


#endif