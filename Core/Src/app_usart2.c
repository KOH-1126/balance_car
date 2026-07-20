#include "app_usart2.h"

#include <stdarg.h>
#include <stdio.h>

#define APP_USART2_PRINTF_BUFFER_SIZE 128U

void My_USART_Printf(UART_HandleTypeDef *huart, const char *format, ...)
{
  char buffer[APP_USART2_PRINTF_BUFFER_SIZE];
  va_list args;
  int length;

  if ((huart == NULL) || (huart->Instance != USART2) || (format == NULL))
  {
    return;
  }

  va_start(args, format);
  length = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  if (length <= 0)
  {
    return;
  }

  if ((size_t)length >= sizeof(buffer))
  {
    length = (int)(sizeof(buffer) - 1U);
  }

  (void)HAL_UART_Transmit(huart, (const uint8_t *)buffer,
                          (uint16_t)length, HAL_MAX_DELAY);
}
