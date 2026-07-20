#ifndef APP_BAT_H
#define APP_BAT_H

#ifdef __cplusplus
extern "C" {
#endif

void App_Bat_Init(void);
void App_Bat_Proc(void);
float App_Bat_Get(void);

#ifdef __cplusplus
}
#endif

#endif
