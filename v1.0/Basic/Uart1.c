//串口通信程序移植自官方例程
#include "Uart1.h"

static u8  TX1_Cnt;    //发送计数
static u8  RX1_Cnt;    //接收计数
static bit B_TX1_Busy; //发送忙标志

static u8  RX1_Buffer[UART1_BUF_LENGTH]; //接收缓冲

//========================================================================
// 函数: void UART1_config(u8 brt)
// 描述: UART1初始化函数。
// 参数: brt: 选择波特率, 2: 使用Timer2做波特率, 其它值: 使用Timer1做波特率.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================

void UART1_config(u8 pin)
{
    /*********** 波特率使用定时器1 *****************/
        TR1 = 0;
        S1BRT = 0;		//S1 BRT Use Timer1;
        T1_CT = 0;		//Timer1 set As Timer
        T1x12 = 1;		//Timer1 set as 1T mode
        TMOD &= ~0x30;//Timer1_16bitAutoReload;
        TH1 = (u8)(Baudrate1 / 256);
        TL1 = (u8)(Baudrate1 % 256);
        ET1 = 0;    //禁止中断
        TR1 = 1;
    /*************************************************/

    SCON = (SCON & 0x3f) | 0x40;    //UART1模式, 0x00: 同步移位输出, 0x40: 8位数据,可变波特率, 0x80: 9位数据,固定波特率, 0xc0: 9位数据,可变波特率
//  PS  = 1;    //高优先级中断
    ES  = 1;    //允许中断
    REN = 1;    //允许接收
    P_SW1 &= 0x3f;
    P_SW1 |= 0x00;      //UART1 switch to, 0x00: P3.0 P3.1, 0x40: P3.6 P3.7, 0x80: P1.6 P1.7, 0xC0: P4.3 P4.4

    B_TX1_Busy = 0;
    TX1_Cnt = 0;
    RX1_Cnt = 0;
}

//========================================================================
// 函数: void UART1_int (void) interrupt UART1_VECTOR
// 描述: UART1中断函数。
// 参数: none.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART1_isr (void) interrupt 4
{
    if(RI)
    {
        RI = 0;
        RX1_Buffer[RX1_Cnt] = SBUF;
        if(++RX1_Cnt >= UART1_BUF_LENGTH)   RX1_Cnt = 0;
    }

    if(TI)
    {
        TI = 0;
        B_TX1_Busy = 0;
    }
}

//========================================================================
// 函数: void PrintString1(u8 *puts)
// 描述: 串口1发送字符串函数。
// 参数: puts:  字符串指针.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void PrintString1(u8 *puts)
{
    for (; *puts != 0;  puts++)     //遇到停止符0结束
    {
        while(B_TX1_Busy);
        B_TX1_Busy = 1;
        SBUF = *puts;
    }
}

// 发送单个无符号32位数字
void PrintNum1(uint32_t num)
{
    do
    {
        while(B_TX1_Busy);
        B_TX1_Busy = 1;
        SBUF = (uint8_t) (num % 10 + '0');
        num = (uint32_t)(num / 10);
    }while(num != 0);

    while(B_TX1_Busy);
    B_TX1_Busy = 1;
    SBUF = 0x0A;//发送回车
}

void PrintRaw_8(uint16_t rawData)
{
    while(B_TX1_Busy);
    B_TX1_Busy = 1;
    SBUF = rawData;
}

void PrintRaw_16(uint16_t rawData)
{
    while(B_TX1_Busy);
    B_TX1_Busy = 1;
    SBUF = rawData >> 8;

    while(B_TX1_Busy);
    B_TX1_Busy = 1;
    SBUF = rawData;
}

void PrintRaw(uint32_t rawData)
{
    while(B_TX1_Busy);
    B_TX1_Busy = 1;
    SBUF = rawData >> 24;

    while(B_TX1_Busy);
    B_TX1_Busy = 1;
    SBUF = rawData >> 16;
    
    while(B_TX1_Busy);
    B_TX1_Busy = 1;
    SBUF = rawData >> 8;

    while(B_TX1_Busy);
    B_TX1_Busy = 1;
    SBUF = rawData;

    while(B_TX1_Busy);
    B_TX1_Busy = 1;
    SBUF = 0x0A;//发送回车 
}



