
#ifndef _HS_TYPE
#define _HS_TYPE
//-----------------------type def
//typedef unsigned int ui16;
//typedef signed int si16;
//typedef unsigned long int ul32;
//typedef signed long int sl32;
typedef unsigned char BYTE;
typedef unsigned int  WORD;
typedef unsigned short DWORD;

typedef unsigned long  ul32;
typedef signed long    sl32;
typedef unsigned short ui16;
typedef signed short   si16;
typedef signed char    sc08;
typedef unsigned char  uc08;

// Alphanumeric LCD Module functions
#define sbi(x,y) (x|=(1<<y))
#define cbi(x,y) (x&=~(1<<y))
#define bv(bit)	 (1 << (bit))
#define ABS(x)		((x>0)?(x):(-x))
#define DEC(a) if((a)>0) a--
//#define ON 1
//#define OFF 0

#define __DEBUG_MODE_	 0
//#define LCD_P_GO(a,b) (lcd_gotoxy(a,b))
//#define LCD_CLR() (lcd_clear())

//----------------------------------
//#define PA PORTA
//#define PB PORTB
//#define PC PORTC
//#define PD PORTD
//#define PE PORTE
//#define PF PORTF
//#define PG PORTG
//#define PH PORTH
//#define PJ PORTJ
//#define PK PORTK
//#define PL PORTL

//-------------------------------------



#endif




