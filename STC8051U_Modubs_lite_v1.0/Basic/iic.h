#ifndef __IIC_H_
#define __IIC_H_


#include "AI8051U.h"

#define IIC_MODE    1       //0：软件IIC  1：硬件IIC(仅4脚)
#define IIC_PIN     0       //0：四针IIC  1：七针IIC
#define SLAVE_24C02         0XA0

//------------------------------引脚定义------------------------------

// #if     IIC_PIN == 0

//     #define SCL P15
//     #define SDA P14

// #elif   IIC_PIN == 1

    #define SCL P15
    #define SDA P14
    #define RES P23
    #define DC  P34
   #define CS  P11

// #endif


//------------------------------变量声明------------------------------


//------------------------------函数声明------------------------------

void IIC_Delay2us(void);	    //@22.1184MHz
void IIC_Start(void);           //IIC启动
void IIC_Stop(void);            //IIC结束
void IIC_SendACK(void);         //发送ACK
void IIC_SendNoACK(void);       //发送NOACK
void IIC_WaitACK(void);         //等待从机ACK
void IIC_SendByte(u8 dat);      //发送一个字节
u8 IIC_ReadByte(void);          //读取一个字节
void IIC_Init(void);            //硬件IIC初始化

void IIC_Write_Byte(u8 slave, u8 addr, u8 *p, u8 number); // IIC写入连续的几个字节
void IIC_Read_Byte(u8 slave, u8 addr, u8 *p, u8 number);  // IIC读取连续的几个字节

#endif