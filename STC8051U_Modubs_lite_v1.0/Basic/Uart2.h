#ifndef __UART2_h
#define __UART2_h

#include "SysConfig.h"

#define Baudrate2   (65536 - MAIN_Fosc / 115200 / 4)
#define UART2_BUF_LENGTH    128

extern void PrintString2(u8 *puts);
extern void Uart2_PrintRaw_8(uint8_t rawData);
extern void UART2_config(u8 brt);


#endif