#include "app_button.h"
#include "app.pwm.h"

// 通用按钮部分

#define BUTTON_SETTLING_TIME             10   // 按钮消抖延迟
#define BUTTON_CLICK_INTERVAL            200  // 按钮多击时每次点击的时间最大时间间隔，间隔短于此时间则视为连击
#define BUTTON_LONG_PRESS_THRESHOLD      1000 // 按钮长按最小时间，单位毫秒，按下时间短于此时间不计作长按
#define BUTTON_LONG_PRESS_TICK_INTERNVAL 100  // 长按后持续触发的时间间隔，每次触发的间隔时间，单位毫秒

static void OnButtonPressed(Button_t *Button);
static void OnButtonReleased(Button_t *Button);
static void OnButtonEveryPolled(Button_t *Button, uint32_t currentTime);

/**
  * @brief  Initialize the button application.
  * 一共有四个回调函数，分别是按下、松开、连击并计数和长按并计数
  */
void App_Button_Init(Button_t *Button, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    Button->GPIOx = GPIOx;
    Button->GPIO_Pin = GPIO_Pin;
    Button->button_pressed_cb = 0;
	Button->button_released_cb = 0;
	Button->button_clicked_cb = 0;
	Button->button_long_pressed_cb = 0;
	Button->LongPressThreshold = BUTTON_LONG_PRESS_THRESHOLD;
	Button->ClickInterval = BUTTON_CLICK_INTERVAL;
	Button->LongPressTickInterval = BUTTON_LONG_PRESS_TICK_INTERNVAL;

    Button->LastState = 1; // 初始状态下假设按钮是松开的
	Button->ChangePending = 0; 
	Button->PendingTime = 0;
	Button->LastPressedTime = 0;
	Button->LastReleasedTime = 0;
	Button->LongPressTicks = 0;
	Button->ClickCnt = 0;
}

void App_Button_Proc(Button_t *Button)
{
    uint8_t currentState;
	
	uint32_t currentTime = HAL_GetTick(); // 获取当前时间
	
	// 按键消抖
	if(Button->ChangePending) //如果按钮状态发生了改变
	{
		if (currentTime - Button->PendingTime >= BUTTON_SETTLING_TIME) // 已渡过按钮抖动时间
		{
            currentState = HAL_GPIO_ReadPin(Button->GPIOx, Button->GPIO_Pin) == GPIO_PIN_RESET ? 0 : 1; // 读取当前按钮状态，0 - 按下，1 - 松开
			
			if(currentState != Button->LastState)
			{
				if(currentState == 0) 
					OnButtonPressed(Button); // #1. 按钮按下
				else 
					OnButtonReleased(Button); // #2. 按钮松开
			}
			Button->LastState = currentState;
			Button->ChangePending = 0; // 状态改变已处理
		}
	}
	else // 按钮状态未发生改变，继续监测是否改变
	{
		currentState = HAL_GPIO_ReadPin(Button->GPIOx, Button->GPIO_Pin) == GPIO_PIN_RESET ? 0 : 1;
		
		if(currentState != Button->LastState)
		{
			Button->PendingTime = currentTime;
			Button->ChangePending = 1;
		}
	}

    OnButtonEveryPolled(Button, currentTime); // #3. 按钮状态被检测
}

uint8_t Button_GetState(Button_t *Button)
{
	return Button->LastState;
}

/**
 * @brief 设置按钮的单击间隔，短于此间隔视为连击
 * @param Interval - 单击间隔（单位毫秒） 
 */
void Button_ClickIntervalConfig(Button_t *Button, uint32_t Interval)
{
	Button->ClickInterval = Interval;
}

void Button_LongPressConfig(Button_t *Button, uint32_t Throshold, uint32_t TickInterval)
{
	Button->LongPressThreshold = Throshold;
	Button->LongPressTickInterval = TickInterval;
}

void Button_SetLongPressCb(Button_t *Button, void (*LongPressCb)(uint8_t ticks))
{
	Button->button_long_pressed_cb = LongPressCb;
}

void Button_SetPressCb(Button_t *Button, void (*PressCb)(void))
{
	Button->button_pressed_cb = PressCb;
}

void Button_SetReleaseCb(Button_t *Button, void (*ReleaseCb)(void))
{
	Button->button_released_cb = ReleaseCb;
}

void Button_SetClickCb(Button_t *Button, void (*ClickCb)(uint8_t clicks))
{
	Button->button_clicked_cb = ClickCb;
}

/**
 * @brief 处理按钮按下事件
 */
static void OnButtonPressed(Button_t *Button)
{
    Button->LastPressedTime = HAL_GetTick();
	
	if(Button->button_pressed_cb != 0)
	{
		Button->button_pressed_cb();
	}
}

/**
 * @brief 处理按钮松开事件
 */
static void OnButtonReleased(Button_t *Button)
{
    Button->LastReleasedTime = HAL_GetTick();
	
	// 调用按钮松开的回调函数
	if(Button->button_released_cb != 0)
	{
		Button->button_released_cb();
	}

    // 松开后长按计数清零
	Button->LongPressTicks = 0;
	
	if(Button->LastReleasedTime - Button->LastPressedTime < Button->LongPressThreshold) // 如果按下时间小于长按阈值，则视为一次点击
	{
		Button->ClickCnt++;
	}
	else // 如果按下时间大于长按阈值，则视为长按，清除连击计数
	{
		Button->ClickCnt = 0;
	}
}


static void OnButtonEveryPolled(Button_t *Button, uint32_t currentTime)
{
	/* 处理按钮长按的动作 */
	if(Button->LastState == 0) // 如果按钮当前是按下状态
	{
		if(Button->LongPressTicks == 0) // 如果长按未被触发
		{
			if(Button->LastPressedTime!= 0 // 按钮被按下过
				&& currentTime - Button->LastPressedTime > Button->LongPressThreshold) // 且已超过触发时间
			{
				Button->LongPressTicks = 1; //长按计数器启动，初始值为1
			
				if(Button->button_long_pressed_cb)
				{
					Button->button_long_pressed_cb(Button->LongPressTicks); // 触发长按回调函数
				}
				
				Button->LastLongPressTickTime = HAL_GetTick(); // 记录第一次长按计数器增加的时间
			}
		}
		else // 如果长按已被触发
		{
			if(currentTime - Button->LastLongPressTickTime > Button->LongPressTickInterval) // 超过长按计数间隔
			{
				Button->LastLongPressTickTime = HAL_GetTick(); // 更新长按计数增加的时间
				
				Button->LongPressTicks++;
				
				if(Button->button_long_pressed_cb)
				{
					Button->button_long_pressed_cb(Button->LongPressTicks); // 触发长按回调函数
				}
			}
		}
	}
	
	/* 处理按钮连击动作 */
	if(Button->ClickCnt > 0 //如果按下动作算作单次点击，ClikckCnt至少为1
		&& Button->LastState == 1 // 按钮已经松开
		&& (HAL_GetTick() - Button->LastReleasedTime) > Button->ClickInterval) // 如果松开时间超过了连击间隔，视为连击结束
	{
		if(Button->button_clicked_cb)
		{
			Button->button_clicked_cb(Button->ClickCnt);
		}
		
		Button->ClickCnt = 0; // 清除连击记录
	}
}

// 用户按钮部分

/**
 * @brief  User button instance, PA11
 */
static Button_t userKey;
static void OnUserKey_Clicked(uint8_t clicks);

void User_Key_Init(void)
{
	App_Button_Init(&userKey, GPIOA, GPIO_PIN_11);
	Button_SetClickCb(&userKey, OnUserKey_Clicked);
}

void User_Key_Proc(void)
{
	App_Button_Proc(&userKey);
}

static uint8_t pwm_on = 0;

/**
 * @brief 点击userkey的回调函数
 */
static void OnUserKey_Clicked(uint8_t clicks)
{
	if(clicks == 1){
		pwm_on = !pwm_on;
		App_PWM_Cmd(pwm_on);
	}
}