
#include "global_hs.h"



HAL_StatusTypeDef put_char2( char c_data)
{
  HAL_StatusTypeDef ret ;  
  uc08 buf[2]= {0};
  buf[0]  = c_data;
  
  ret  = HAL_UART_Transmit(&huart2,buf,1,1000); // 
  return ret;
}

void my_put_string(  char * c_data  )
{
  while(*c_data != 0)
  {
    if(HAL_UART_GetState(&huart2) == HAL_BUSY)
    {
      HAL_Delay (1);
    }
    else 
    {
      if(put_char2(*c_data++) != HAL_OK) break;
    }
  }
  
}



char my_Nput_string( UART_HandleTypeDef huart , uint8_t * c_data , int size)
{
		int send_size = size ;
		uint8_t * addr = c_data;

			while(--send_size)
			{
				put_char2(*addr);
				*addr =0;
				addr++;
			}
	return 0;
}

