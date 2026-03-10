#include "Timer.h"

#include "Uart3.h"
#include "Uart4.h"
extern u8  TX3_number;

uint32_t systick_count;

void Delay1000ms(void);
void n_ms(uint16_t x);
void n_ms_timer_0(void);
void timer4_init(void);
uint32_t get_systick(void);

//软件延时
// void Delay1000ms(void)	//@44.2368MHz
// {
// 	unsigned long edata i;

// 	_nop_();
// 	_nop_();
// 	i = 11059198UL;
// 	while (i) i--;
// }

void Delay1000ms(void)	//@40MHz
{
	unsigned long edata i;

	_nop_();
	_nop_();
	i = 9999998UL;
	while (i) i--;
}

//延时N-MS子程序********************************************************************
void n_ms(uint16_t x)
{
	uint16_t i;
    for(i=0;i<x;i++)
    {
		n_ms_timer_0();
    }
}

//定时器0的阻塞式1ms延时***********************************************************
void n_ms_timer_0(void)
{  
    TMOD &= 0xF0; // 清除定时器0的控制位  
    TMOD |= 0x01; // 设置定时器0为模式1（16位定时器/计数器模式）  
  
    TL0 = T0MS & 0x00ff;            //reload timer1 low byte
    TH0 = T0MS >> 8;                //reload timer1 high byte

    TR0 = 1; // 启动定时器0  
  
    while(!TF0); // 等待定时器溢出
    
  
    TF0 = 0; // 清除溢出标志  
    TR0 = 0; // 停止定时器0  
}


// 配置使用T4作为systick生成器，时钟源选择系统时钟
// 当前配置为每1ms产生一次中断

void timer4_init(void)
{
    systick_count = 0;
    
    // T4做定时器，1T模式，关闭时钟输出；关闭T3
    T4T3M = 0x20;

    // 设定时钟分频
    TM4PS = 39;
    
    // 定时器预设
    T4L = T4Tick_SET ;
    T4H = T4Tick_SET >>8;
    
    //定时器4开始计数，允许中断
    IE2 |= 0x40;
    T4T3M |= 0x80;
}

//T4中断服务程序
void TM4_Isr() interrupt 20
{
    systick_count++;

    //检查UART3接收缓冲区
    if(RX3_TimeOut != 0)
	{
		if(--RX3_TimeOut == 0)	//超时
		{
			if(RX3_Cnt != 0)	//接收有数据
			{
				B_RX3_OK = 1;	//标志已收到数据块
			}
		}
	}
    //检查UART4接收缓冲区
    if(RX4_TimeOut != 0)
	{
		if(--RX4_TimeOut == 0)	//超时
		{
			if(RX4_Cnt != 0)	//接收有数据
			{
				B_RX4_OK = 1;	//标志已收到数据块
			}
		}
	}
}

//获取systick值
uint32_t get_systick(void) 
{
    return systick_count;
}
