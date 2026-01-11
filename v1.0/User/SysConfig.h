#ifndef __SysConfig_H
#define __SysConfig_H

// ----------------------------公共头文件引用-----------------------------
#include "AI8051U.h"
#include <stdio.h>
// #include <string.h>
// ------------------------------*********-------------------------------


sbit back_light_ctrl = P2^7;
sbit LED_ctrl_1 = P2^2;
sbit LED_ctrl_2 = P0^7;
// ------------------------------时钟预设---------------------------------
//使用STC8051U时，建议主频使用40MHz以提高处理速度，时钟频率在STC-ISP软件中修改
//注意气温对内部时钟影响较大，目前已知冬天气温15度时44M会不稳定，需降至40M或更低

// #define MAIN_Fosc 44236800UL
#define MAIN_Fosc 40000000UL

// ------------------------------*********-------------------------------

#define     TRUE    1
#define     FALSE   0

// ----------------------------数据类型宏定义-----------------------------
#define     u8     unsigned char
#define     u16    unsigned int
#define     u32    unsigned long

#define     uint8_t     unsigned char
#define     uint16_t    unsigned int
#define     uint32_t    unsigned long

#define     int8_t      char
#define     int16_t     int
#define     int32_t     long

#define     uchar   unsigned char
#define     uint    unsigned short

// ------------------------------*********-------------------------------

// ----------------------------User Define ------------------------------

// ----------------------------Test Define ------------------------------
#define TEST_MODE 0

#endif
