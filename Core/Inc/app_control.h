#ifndef APP_CONTROL_C
#define APP_CONTROL_C

#include "stm32f1xx_hal.h"

void App_Control_Init(void);
void App_Control_Proc(void);
void App_Control_Reset(void);
void App_Control_SetMoveSpeed(float moveSpeed);

#endif
