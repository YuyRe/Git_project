#include "Key_HAL.h"

void Key_Init(Key_t *Key)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	if(Key->GPIOx == GPIOA){__HAL_RCC_GPIOA_CLK_ENABLE();}
	else if(Key->GPIOx == GPIOB){__HAL_RCC_GPIOB_CLK_ENABLE();}
	else if(Key->GPIOx == GPIOC){__HAL_RCC_GPIOC_CLK_ENABLE();}
	else if(Key->GPIOx == GPIOD){__HAL_RCC_GPIOD_CLK_ENABLE();}
	else if(Key->GPIOx == GPIOE){__HAL_RCC_GPIOE_CLK_ENABLE();}
	
	GPIO_InitStruct.Pin = Key->GPIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = (Key->Openlevel == GPIO_PIN_RESET)? GPIO_PULLUP:GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Key->GPIOx, &GPIO_InitStruct);
	
	Key->Staut = 0;
	Key->CycleNum = 0;
}

uint8_t Key_GetFlag(Key_t *Key, uint8_t Bit)
{
	if(((Key->Staut >> Bit) & 1u)  == 1u){
		Key->Staut &= ~(1u << Bit);
		return 1;
	}else
		return 0;
}

void Key_Update(Key_t *Key)
{
	GPIO_PinState level = HAL_GPIO_ReadPin(Key->GPIOx, Key->GPIO_Pin);
	
	if(level == Key->Openlevel) Key->Staut |= 0x01;
	if((Key->Staut & 1u) == 1u && level != Key->Openlevel){
		Key->Staut &= 0xFE;
	
		Key->Staut |= 0x02;
		Key->Staut += 0x10;
		if((Key->Staut >> 4) > 1) Key->Staut |= 0x04;
	}
	 
	if(Key->CycleNum > KEY_TICK_NUM)
	{
		Key->CycleNum = 0;
		Key->Staut &= 0x0F;
	}
	Key->CycleNum ++;
}
