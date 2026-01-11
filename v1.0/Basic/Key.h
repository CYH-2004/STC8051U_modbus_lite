#ifndef __KEY_H
#define __KEY_H

#include "SysConfig.h"
#include "AI8051U.h"
#include "Timer.h"

// 按键引脚定义（STC8051U开发板）
sbit	KEY_OK=P3^2;
sbit	KEY_UP=P3^6;
sbit	KEY_DOWN=P3^7;


// 按键引脚定义（DisplayModule v1.0）
// sbit	KEY_OK=P3^4;
// sbit	KEY_UP=P3^5;
// sbit	KEY_DOWN=P3^2;

void key_scan(void);
void key_scan1(uint16_t dtime);//可修改等待的按键
uint8_t key_scan2(uint16_t dtime);//限时按键检测功能，使用对P2完整设定实现，建议开发对P2位检测的方法
uint16_t key_scan3(uint16_t dtime);//单键检测，返回相应时间（循环次数）
uint16_t key_scan4();//多键检测，不限时

#endif
