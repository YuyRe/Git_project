#ifndef __KEY_HAL_H
#define __KEY_HAL_H

#include "main.h"

typedef struct
{
	GPIO_TypeDef *GPIOx;  
	uint16_t GPIO_Pin;
	GPIO_PinState Openlevel; //导通电平
	
	uint8_t Staut;  //状态标志 
//Bit0当前状态标志位，1为导通，0为断路
//Bit1导通触发标志位，导通及置1
//Bit2双击标志位，发生置1
//Bit3 空闲	
//Bit4~7 导通次数计数标志位
	uint8_t CycleNum; //周期计数
	
} Key_t;


#define KEY_TICK_NUM        5  //双击判断区间，实际时间区间 = Key_Update调用周期 * KEY_TICK_NUM的值

#define KEY_CURRENT_STATUS  0u
#define KEY_PRESS_STATUS    1u
#define KEY_DOUBLE_STATUS   2u

void Key_Init(Key_t *Key);
uint8_t Key_GetFlag(Key_t *Key, uint8_t Bit);
void Key_Update(Key_t *Key); //在定时中断中重复调用

#endif
