#ifndef __UART3_h
#define __UART3_h

#include "SysConfig.h"

#define Baudrate3   (65536 - MAIN_Fosc / 115200 / 4)

#define	RX3_Length	128		/* 接收缓冲长度 */
#define	TX3_Length	128		/* 发送缓冲长度 */

extern u8  TX3_Cnt;     //发送计数
extern u8  RX3_Cnt;     //接收计数
extern u8  RX3_TimeOut;	//接收超时计时器

extern bit B_RX3_OK;    //接收数据标志
extern bit B_TX3_Busy;  //发送忙标志

extern u8  xdata RX3_Buffer[RX3_Length]; //接收缓冲

extern void UART3_config(u8 pin);
extern void PrintString3(u8 *puts);
extern void UART3_send(u8 *p, u8 len);

extern void UART3_RX_buffer_reset(void);
extern uint8_t get_RX3_buffer_length(void);
extern uint8_t xdata *get_RX3_buffer_address(void);

#endif
