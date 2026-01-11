#include "Uart3.h"

/*************  本地变量声明    **************/

u8  TX3_Cnt;    //发送计数
u8  RX3_Cnt;    //接收计数
u8  RX3_TimeOut;//接收超时计时器

bit	B_RX3_OK;	//接收数据标志，需手动清0
bit B_TX3_Busy; //发送忙标志

u8  xdata RX3_Buffer[RX3_Length]; //接收缓冲

/*************  本地函数声明    **************/

void UART3_config(u8 pin);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
void PrintString3(u8 *puts);
void UART3_send(u8 *p, u8 len);

//========================================================================
// 函数: void UART3_config(u8 brt)
// 描述: UART3初始化函数。
// 参数: pin：引脚选择，0: P0.0 P0.1,  1: P5.0 P5.1,    其他: 0
// 返回: none.
// 版本: VER1.0
// 日期: 2026-1-4
// 备注: 使用Timer3做波特率发生器
//========================================================================
void UART3_config(u8 pin)
{
    // Baudrate3 = (65536 - MAIN_Fosc / Baudrate3 / 4);
    
    if(pin > 1) pin = 0;
        T3R = 0;		//Timer stop
        S3CON = 0x50;   //8位数据, 使用Timer3做波特率发生器, 允许接收
        T3H = (u8)(Baudrate3 / 256);
        T3L = (u8)(Baudrate3 % 256);
        T3_CT = 0;	    //Timer3 set As Timer
        T3x12 = 1;	    //Timer3 set as 1T mode
        T3R = 1;		//Timer run enable

    ES3  = 1;           //允许UART3中断
    S3_S = (pin & 0x01);  //UART3 switch bit1 to: 0: P0.0 P0.1,  1: P5.0 P5.1

    B_TX3_Busy = 0;
    TX3_Cnt = 0;
    RX3_Cnt = 0;
}

//========================================================================
// 函数: void PrintString3(u8 *puts)
// 描述: 串口3发送字符串函数。
// 参数: puts:  字符串指针.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void PrintString3(u8 *puts)
{
    for (; *puts != 0;  puts++)     //遇到停止符0结束
    {
        while(B_TX3_Busy);
        B_TX3_Busy = 1;
        S3BUF = *puts;
    }
}

//========================================================================
// 函数: void UART3_send(u8 *p, u8 len)
// 描述: 串口3数据发送函数。
// 参数: p:  数据指针.  len:  数据长度.
// 返回: none.
// 版本: VER1.0
// 日期: 2026-1-4
// 备注: 
//========================================================================
void UART3_send(u8 *p, u8 len)
{
    u8 i;
    for (i = 0; i < len; i++)
    {
        while (B_TX3_Busy);
        B_TX3_Busy = 1;
        S3BUF = *p;
        p++;
    }
}


void UART3_RX_buffer_reset(void)
{
    B_RX3_OK = 0;
    RX3_Cnt = 0;
}

uint8_t get_RX3_buffer_length(void)
{
    return RX3_Cnt;
}

//========================================================================
// 函数: uint8_t xdata *get_RX3_buffer_address(void)
// 描述: 获取UART3接收缓冲区地址.
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2026-1-4
// 备注: 
//========================================================================
uint8_t xdata *get_RX3_buffer_address(void)
{
    return RX3_Buffer;
}

//========================================================================
// 函数: void UART3_int (void) interrupt UART3_VECTOR
// 描述: UART3中断函数。
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2026-1-4
// 备注: 注意当前版本仅单缓冲区，新的数据会覆盖掉旧数据
//========================================================================
void UART3_int (void) interrupt 17
{
    if(S3RI)
    {
        S3RI = 0;    //Clear Rx flag
        RX3_Buffer[RX3_Cnt] = S3BUF;
        if(++RX3_Cnt >= RX3_Length)   RX3_Cnt = 0;
        
        RX3_TimeOut = 2;    //timeout in 1ms
    }

    if(S3TI)
    {
        S3TI = 0;   //Clear Tx flag
        B_TX3_Busy = 0;
    }
}

