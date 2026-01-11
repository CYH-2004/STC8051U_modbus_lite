#ifndef __UART4_h
#define __UART4_h

#include "SysConfig.h"

#define Baudrate4   (65536 - MAIN_Fosc / 115200 / 4)

#define	RX4_Length	128		/* 接收缓冲长度 */
#define	TX4_Length	128		/* 发送缓冲长度 */

extern u8  TX4_Cnt;     //发送计数
extern u8  RX4_Cnt;     //接收计数
extern u8  RX4_TimeOut;	//接收超时计时器

extern bit B_RX4_OK;    //接收数据标志
extern bit B_TX4_Busy;  //发送忙标志

extern u8  xdata RX4_Buffer[RX4_Length]; //接收缓冲

extern void UART4_config(u8 brt);
extern void PrintString4(u8 *puts);
extern void UART4_send(u8 *p, u8 len);

extern void UART4_RX_buffer_reset(void);
extern uint8_t get_RX4_buffer_length(void);
extern uint8_t xdata *get_RX4_buffer_address(void);

#endif
