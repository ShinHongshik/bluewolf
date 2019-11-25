#ifndef _RSCMD_H_
#define _RSCMD_H_

/* outof ascii values */
#define CMD_END		0x80
#define CMD_ERROR	0xC0
#define CMD_CONT	0x8
#define CMD_CANCEL	0xa



extern uc08 dbgLevel;
//extern uc08 selectedPort[];
//extern uc08 selectedRange[];


//extern int repeat[];


extern char db_cmd_buf[];

void rs_rece_db(void);
void rs_rece_w107(void);
extern void put_char0(char c_data);
extern void put_char1(char c_data);
extern HAL_StatusTypeDef put_char2( char c_data);
extern void put_char3(char c_data);



//int calc_chksum(char* data);

#endif






