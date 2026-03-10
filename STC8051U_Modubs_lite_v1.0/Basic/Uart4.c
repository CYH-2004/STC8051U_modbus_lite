#include "Uart4.h"

/*************  本地变量声明    **************/

u8  TX4_Cnt;    //发送计数
u8  RX4_Cnt;    //接收计数
u8  RX4_TimeOut;//接收超时计时器

bit	B_RX4_OK;	//接收数据标志，需手动清0
bit B_TX4_Busy; //发送忙标志

u8  xdata RX4_Buffer[RX4_Length]; //接收缓冲

/*************  本地函数声明    **************/

void UART4_config(u8 brt);   // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer3做波特率.
void PrintString4(u8 *puts);
void UART4_send(u8 *p, u8 len);

//========================================================================
// 函数: void PrintString4(u8 *puts)
// 描述: 串口4发送字符串函数。
// 参数: puts:  字符串指针.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void PrintString4(u8 *puts)
{
    for (; *puts != 0;  puts++)     //遇到停止符0结束
    {
        S4BUF = *puts;
        B_TX4_Busy = 1;
        while(B_TX4_Busy);
    }
}

//========================================================================
// 函数: void UART4_send(u8 *p, u8 len)
// 描述: 串口3数据发送函数。
// 参数: p:  数据指针.  len:  数据长度.
// 返回: none.
// 版本: VER1.0
// 日期: 2026-1-4
// 备注: 
//========================================================================
void UART4_send(u8 *p, u8 len)
{
    u8 i;
    for (i = 0; i < len; i++)
    {
        while (B_TX4_Busy);
        B_TX4_Busy = 1;
        S4BUF = *p;
        p++;
    }
}

void UART4_RX_buffer_reset(void)
{
    B_RX4_OK = 0;
    RX4_Cnt = 0;
}

uint8_t get_RX4_buffer_length(void)
{
    return RX4_Cnt;
}

//========================================================================
// 函数: uint8_t xdata *get_RX4_buffer_address(void)
// 描述: 获取UART3接收缓冲区地址.
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2026-1-4
// 备注: 
//========================================================================
uint8_t xdata *get_RX4_buffer_address(void)
{
    return RX4_Buffer;
}

//========================================================================
// 函数: SetTimer2Baudraye_for_uart4(u32 dat)
// 描述: 设置Timer2做波特率发生器。
// 参数: dat: Timer2的重装值.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void SetTimer2Baudraye_for_uart4(u32 dat)  // 使用Timer2做波特率.
{
    T2R = 0;		//Timer stop
    T2_CT = 0;	//Timer2 set As Timer
    T2x12 = 1;	//Timer2 set as 1T mode
    T2H = (u8)(dat / 256);
    T2L = (u8)(dat % 256);
    ET2 = 0;    //禁止中断
    T2R = 1;		//Timer run enable
}

//========================================================================
// 函数: void UART4_config(u8 brt)
// 描述: UART4初始化函数。
// 参数: brt: 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer4做波特率.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART4_config(u8 brt)    // 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer4做波特率.
{
    if(brt == 2)
    {
        SetTimer2Baudraye_for_uart4(Baudrate4);
        S4CON = 0x10;       //8位数据, 使用Timer2做波特率发生器, 允许接收
    }
    else
    {
        T4R = 0;	//Timer stop
        S4CON = 0x50;       //8位数据, 使用Timer4做波特率发生器, 允许接收
        T4H = (u8)(Baudrate4 / 256);
        T4L = (u8)(Baudrate4 % 256);
        T4_CT = 0;	//Timer3 set As Timer
        T4x12 = 1;	//Timer3 set as 1T mode
        T4R = 1;	//Timer run enable
    }
    ES4  = 1;       //允许UART4中断
    S4_S = 0;       //UART4 switch bit2 to: 0: P0.2 P0.3, 1: P5.2 P5.3

    B_TX4_Busy = 0;
    TX4_Cnt = 0;
    RX4_Cnt = 0;
}


//========================================================================
// 函数: void UART4_int (void) interrupt UART4_VECTOR
// 描述: UART4中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART4_int (void) interrupt 18
{
    if(S4RI)
    {
        S4RI = 0;    //Clear Rx flag
        RX4_Buffer[RX4_Cnt] = S4BUF;
        if(++RX4_Cnt >= RX4_Length)   RX4_Cnt = 0;
        RX4_TimeOut = 2;    //timeout in 1ms
    }

    if(S4TI)
    {
        S4TI = 0;    //Clear Tx flag
        B_TX4_Busy = 0;
    }
}

