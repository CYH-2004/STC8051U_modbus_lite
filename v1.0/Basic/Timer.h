#ifndef __Timer_h
#define __Timer_h

#include "SysConfig.h"

#define T0MS 				(65536-MAIN_Fosc/12/1000)
#define T4Tick_SET 		(65536-MAIN_Fosc/40/1000)

extern void n_ms(uint16_t x);
// void delay_n_s(uint16_t x);//ÑÓÊ±N-S×Ó³ÌÐò
// void delay_n_100ms(uint16_t x);	
extern void Delay1000ms(void);
extern void n_ms_timer_0(void);

extern void timer4_init(void);
extern uint32_t get_systick(void);

#endif