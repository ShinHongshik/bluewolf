#ifndef _USART_H_
#define _USART_H_


void put_char0(char c_data);
{
  HAL_UART_Transmit(&huart2,&c_data,1,10); // 1바이트 송신(시리얼 수신데이터를 에코 백 하여 줍니다.)
}

#endif
