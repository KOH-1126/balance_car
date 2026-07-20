#include "app_bat.h"
#include "adc.h"
#include "tim.h"

static volatile float vbat = 0.0f;
static uint32_t led_last_toggle_ms = 0U;
static uint8_t led_blink_stage = 0U;

/**
 * @brief 电池电压检测模块初始化
 */
void App_Bat_Init(void)
{
  /* ADC1 and TIM2 have already been configured by MX_ADC1_Init() and
     MX_TIM2_Init(). Calibrate the ADC before enabling conversions. */
  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /* Arm the injected group and its JEOC interrupt before TIM2 starts
     producing a TRGO update event every 10 ms. */
  if (HAL_ADCEx_InjectedStart_IT(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
}

float App_Bat_Get(void)
{
  return vbat;
}

/**
 * @brief Update the battery level LEDs.
 * @note  Call this function periodically from the main loop.
 */
void App_Bat_Proc(void)
{
  float voltage = vbat;

  if (voltage > 7.9f)
  {
    /* Full: three LEDs on. */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6,
                      GPIO_PIN_SET);
  }
  else if (voltage > 7.4f)
  {
    /* 75%: two LEDs on. */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
  }
  else if (voltage > 7.0f)
  {
    /* 50%: one LED on. */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_RESET);
  }
  else if (voltage > 6.5f)
  {
    /* 20%: all LEDs off. */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6,
                      GPIO_PIN_RESET);
  }
  else
  {
    uint32_t now = HAL_GetTick();

    if ((now - led_last_toggle_ms) >= 100U)
    {
      if (led_blink_stage == 0U)
      {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6,
                          GPIO_PIN_SET);
        led_blink_stage = 1U;
      }
      else
      {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6,
                          GPIO_PIN_RESET);
        led_blink_stage = 0U;
      }

      led_last_toggle_ms = now;
    }
  }
}

/**
 * @brief 注入序列转换完成回调函数
 */
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1)
  {
    uint16_t jdr1 = (uint16_t)HAL_ADCEx_InjectedGetValue(
        hadc, ADC_INJECTED_RANK_1);

    vbat = ((float)jdr1 / 4095.0f) * 3.3f * 8.4f / 3.3f;

    /* HAL disables JEOCIE before this callback for the current ADC settings.
       Re-arm the injected interrupt and wait for the next TIM2 TRGO event. */
    if (HAL_ADCEx_InjectedStart_IT(hadc) != HAL_OK)
    {
      Error_Handler();
    }
  }
}
