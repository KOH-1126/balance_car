#ifndef APP_USART2_H
#define APP_USART2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "usart.h"

/**
 * @brief Format data and send it through USART2.
 *
 * The formatted output is limited to APP_USART2_PRINTF_BUFFER_SIZE - 1 bytes.
 * This function must be called after MX_USART2_UART_Init().
 */
void My_USART_Printf(UART_HandleTypeDef *huart, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* APP_USART2_H */
