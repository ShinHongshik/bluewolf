#ifndef _USART_H_
#define _USART_H_



extern UART_HandleTypeDef huart2;

HAL_StatusTypeDef put_char2( char c_data);
extern void my_put_string(  char * c_data  );

#endif
