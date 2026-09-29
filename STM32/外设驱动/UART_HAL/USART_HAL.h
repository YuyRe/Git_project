#ifndef __USART_HAL_
#define __USART_HAL_

#include "usart.h"

#define RX_BUF_LEN     128          // 单块缓冲区大小，按需修改（≥最大单帧长度）

void USART1_Transmit(uint8_t *pData, uint16_t Size);
void USART1_Printf(const char *fmt, ...);

void USART1_Rx_PingPong_Init(void);
uint16_t USART1_GetFrame(uint8_t **dst);

void USART1_IDLECallback(void); //函数需在USARTx_IRQHandler中断中调用

/* 示例
static uint8_t *buf;  // 用于指向rx_proc_buf的首地址

USART1_Rx_PingPong_Init();  // 串口接收乒乓缓冲区初始化

while(1) 
{
    //==========================读取接收到的帧数据===============================
    uint8_t len = USART1_GetFrame(&buf);  // 获取接收到的帧数据，返回长度
    if(len > 0)
    {
        __HAL_UART_DISABLE_IT(&huart1, UART_IT_IDLE);
        //=========================处理接收到的帧数据===============================
        // 这里可以根据实际应用对接收到的帧数据进行处理


        //=========================处理接收到的帧数据===============================
        __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
    }
    //==========================================================================
}
*/
#endif
