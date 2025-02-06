/*
 * ramn_ecucontrol.c
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 TOYOTA MOTOR CORPORATION.
  * ALL RIGHTS RESERVED.</center></h2>
  *
  * This software component is licensed by TOYOTA MOTOR CORPORATION under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
 */

#include "ramn_ecucontrol.h"

#if defined(TARGET_ECUA)

void RAMN_ECU_SetEnable(char ecuName, uint8_t state)
{
	if 		(ecuName == 'B') HAL_GPIO_WritePin(ECUB_EN_GPIO_Port, ECUB_EN_Pin,state);
	else if (ecuName == 'C') HAL_GPIO_WritePin(ECUC_EN_GPIO_Port, ECUC_EN_Pin,state);
	else if (ecuName == 'D') HAL_GPIO_WritePin(ECUD_EN_GPIO_Port, ECUD_EN_Pin,state);
}

void RAMN_ECU_SetBoot0(char ecuName, uint8_t state)
{
	if 		(ecuName == 'B') HAL_GPIO_WritePin(ECUB_BOOT0_GPIO_Port, ECUB_BOOT0_Pin, state);
	else if (ecuName == 'C') HAL_GPIO_WritePin(ECUC_BOOT0_GPIO_Port, ECUC_BOOT0_Pin, state);
	else if (ecuName == 'D') HAL_GPIO_WritePin(ECUD_BOOT0_GPIO_Port, ECUD_BOOT0_Pin, state);
}

void RAMN_ECU_SetEnableAll(uint8_t state)
{
	RAMN_ECU_SetEnable('B',state);
	RAMN_ECU_SetEnable('C',state);
	RAMN_ECU_SetEnable('D',state);
}

void RAMN_ECU_SetBoot0All(uint8_t state)
{
	RAMN_ECU_SetBoot0('B',state);
	RAMN_ECU_SetBoot0('C',state);
	RAMN_ECU_SetBoot0('D',state);
}

void RAMN_ECU_SetDefaultState(void)
{
	RAMN_ECU_SetBoot0All(GPIO_PIN_RESET);
	osDelay(10); // Add small delay to make sure ECUs boot with BOOT0 pin set low.
	RAMN_ECU_SetEnableAll(GPIO_PIN_SET);
}

#endif
