#ifndef __UART1_h
#define __UART1_h

#include "SysConfig.h"

#define Baudrate1           (65536 - MAIN_Fosc / 115200 / 4)
#define UART1_BUF_LENGTH    128

extern void UART1_config(u8 pin);

extern void PrintString1(u8 *puts);
extern void PrintNum1(uint32_t num);
extern void PrintRaw_8(uint16_t rawData);
extern void PrintRaw_16(uint16_t rawData);
extern void PrintRaw(uint32_t rawData);



#endif