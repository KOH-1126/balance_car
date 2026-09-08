#ifndef APP_TELEMETRY_H
#define APP_TELEMETRY_H

#include "stm32f1xx_hal.h"

void App_Telemetry_Init(void);
void App_Telemetry_Proc(float roll_deg,
                        float roll_ref_deg,
                        float gx_dps,
                        float gx_ref_dps,
                        float omega_ref,
                        float omega_l,
                        float omega_r,
                        float speed_fb_mps,
                        float speed_corr_deg);

#endif
