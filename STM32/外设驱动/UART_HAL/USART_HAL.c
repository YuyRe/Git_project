#include "USART_HAL.h"
#include <stdarg.h>
#include <stio.h>


void USART1_Transmit(uint8_t *pData, uint16_t Size)
{
  if(pData == NULL || Size == 0)
    return;

  HAL_UART_Transmit_DMA(&huart1, pData, Size);

  while(huart1.gState != HAL_UART_STATE_READY);
}

void USART1_Printf(const char *fmt, ...)
{
  va_list ap;
  static char buf[128];
  // 先等上一次发送完成，否则 vsnprintf 会覆盖正在被 DMA 发送的 buf
  while(huart1.gState != HAL_UART_STATE_READY);

  va_start(ap, fmt);
  int len = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  if(len <= 0)
      return;

  // 防止超长截断
  if(len >= (int)sizeof(buf))
      len = (int)sizeof(buf) - 1;

  USART1_Transmit((uint8_t*)buf, (uint16_t)len);
}

static uint8_t rx_bufA[RX_BUF_LEN];
static uint8_t rx_bufB[RX_BUF_LEN];

uint8_t *rx_dma_buf;            // DMA正在写入的缓冲区
uint8_t *rx_proc_buf;           // 主循环处理的缓冲区
volatile uint16_t rx_data_len = 0;  // 本次收到的数据长度
volatile uint8_t  rx_flag = 0;      // 接收完成标志 1=有待处理帧

void USART1_Rx_PingPong_Init(void)
{
  rx_dma_buf = rx_bufA;
  rx_proc_buf = rx_bufB;
  rx_flag = 0;
  rx_data_len = 0;

  /* 先清 IDLE 标志，避免上电后空闲总线立刻触发一次虚假中断 */
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);

  /* 先启动 DMA 接收 */
  HAL_UART_Receive_DMA(&huart1, rx_dma_buf, RX_BUF_LEN);

  /* 最后再使能空闲中断 */
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
}

uint16_t USART1_GetFrame(uint8_t **dst)
{
  uint16_t len = 0;

  if(rx_flag)
  {
    rx_flag = 0;

    len = rx_data_len;
    if(len > RX_BUF_LEN) len = RX_BUF_LEN;
    *dst = rx_proc_buf;  // 直接返回指针，避免拷贝
  }

  return len;
}

void USART1_IDLECallback(void)
{
  // ===== 检测IDLE空闲中断 =====
  if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET)
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);  // 清除IDLE标志

    // 计算本次收到的数据长度
    rx_data_len = RX_BUF_LEN - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
    HAL_UART_AbortReceive(&huart1);      // 停止DMA接收
    // ===== 乒乓缓冲区指针交换 =====
    uint8_t *tmp = rx_dma_buf;
    rx_dma_buf = rx_proc_buf;
    rx_proc_buf = tmp;

    rx_flag = 1;  // 通知主循环有数据待处理

    // 在新缓冲区上重启DMA接收
    HAL_UART_Receive_DMA(&huart1, rx_dma_buf, RX_BUF_LEN);
  }
}
