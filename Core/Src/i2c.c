/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c1;

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspInit 0 */

  /* USER CODE END I2C1_MspInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = MPUSCL_Pin|MPUSDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_AFIO_REMAP_I2C1_ENABLE();

    /* I2C1 clock enable */
    __HAL_RCC_I2C1_CLK_ENABLE();
  /* USER CODE BEGIN I2C1_MspInit 1 */

  /* USER CODE END I2C1_MspInit 1 */
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef* i2cHandle)
{

  if(i2cHandle->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(MPUSCL_GPIO_Port, MPUSCL_Pin);

    HAL_GPIO_DeInit(MPUSDA_GPIO_Port, MPUSDA_Pin);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
  * @brief  Send bytes to an I2C slave in blocking mode.
  * @param  hi2c    Pointer to the HAL I2C handle.
  * @param  Addr    Left-aligned 7-bit slave address (A6..A0 followed by 0).
  * @param  pData   Pointer to the data to send.
  * @param  Size    Number of bytes to send.
  * @retval 0 on success, -1 on address/data NACK, -2 on another HAL error.
  */
__weak int8_t My_I2C_SendBytes(I2C_HandleTypeDef *hi2c, uint8_t Addr,
                               const uint8_t *pData, uint16_t Size)
{
  HAL_StatusTypeDef status;

  if ((hi2c == NULL) || ((pData == NULL) && (Size != 0U)))
  {
    return -2;
  }

  status = HAL_I2C_Master_Transmit(hi2c, (uint16_t)(Addr & 0xFEU),
                                   (uint8_t *)pData, Size, HAL_MAX_DELAY);
  if (status == HAL_OK)
  {
    return 0;
  }

  return ((HAL_I2C_GetError(hi2c) & HAL_I2C_ERROR_AF) != 0U) ? -1 : -2;
}

/**
  * @brief  Receive bytes from an I2C slave in blocking mode.
  * @param  hi2c     Pointer to the HAL I2C handle.
  * @param  Addr     Left-aligned 7-bit slave address (A6..A0 followed by 0).
  * @param  pBuffer  Pointer to the receive buffer.
  * @param  Size     Number of bytes to receive.
  * @retval 0 on success, -1 on address NACK, -2 on another HAL error.
  */
__weak int8_t My_I2C_ReceiveBytes(I2C_HandleTypeDef *hi2c, uint8_t Addr,
                                  uint8_t *pBuffer, uint16_t Size)
{
  HAL_StatusTypeDef status;

  if (Size == 0U)
  {
    return 0;
  }

  if ((hi2c == NULL) || (pBuffer == NULL))
  {
    return -2;
  }

  status = HAL_I2C_Master_Receive(hi2c, (uint16_t)(Addr & 0xFEU),
                                  pBuffer, Size, HAL_MAX_DELAY);
  if (status == HAL_OK)
  {
    return 0;
  }

  return ((HAL_I2C_GetError(hi2c) & HAL_I2C_ERROR_AF) != 0U) ? -1 : -2;
}

/* USER CODE END 1 */

