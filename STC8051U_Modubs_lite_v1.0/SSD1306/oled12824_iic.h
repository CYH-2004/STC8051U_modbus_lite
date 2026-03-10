#ifndef __OLED12824_IIC_H_
#define __OLED12824_IIC_H_


#include "AI8051U.h"

/*选择OLED的尺寸
    0：0.96寸4P/7P
    1：1.30寸4P 
*/
#define OLED_SIZE   0

#define OLED_GRAM_Mode 0            //0：8*128级缓存 1:1024级缓存 2：指令模式


//------------------------------变量声明------------------------------

#define SLAVE_OLED_12864    0X78    //128*64_OLED屏幕IIC地址
#define OLED_WriteCom_Addr	0x00	//从机写指令地址
#define OLED_WriteData_Addr	0x40	//从机写数据地址

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

#define Max_Column			128
#define Max_Row				64
#define	Brightness			0xFF    //亮度0~255

extern char xdata WaveData[128];

#if     OLED_GRAM_Mode == 0             //0：8*128级缓存
    extern u8 OLED_GRAM[8][128];         // OLED全局缓存

#elif   OLED_GRAM_Mode == 1             //1:1024级缓存
    extern u8 OLED_GRAM[1024];           // OLED全局缓存

#else                                   //2：指令模式

#endif



//------------------------------引脚定义------------------------------






//------------------------------函数声明------------------------------

void OLED_Write_Byte(u8 dat, u8 cmd);               // 写入一个字节
void OLED_Init(void);                               // 初始化OLED
void OLED_Set_Pos(u8 x, u8 y);                      // OLED设置显示位置
void OLED_Display_On(void);                         // 开启OLED显示
void OLED_Display_Off(void);                        // 关闭OLED显示
void OLED_LightSet(u8 num);                         // 亮度设置
void OLED_Clear(bit mode);                          // 清屏函数

#if     OLED_GRAM_Mode == 0 || OLED_GRAM_Mode == 1           //0：8*128级缓存//1:1024级缓存

// #elif   OLED_GRAM_Mode == 1             

    void OLED_GRAM_Clear(bit mode);                                         // 通过缓冲数组，OLED屏幕全亮全灭
    void OLED_Refresh_Gram(void);                                           // OLED刷新显示
    void OLED_RefreshPart_Gram(u8 xstart, u8 ystart, u8 width, u8 height);  // OLED局部刷新显示

    void OLED_DrawPoint(u8 x, u8 y, bit mode);                              // OLED画一个点，0：熄灭 1：点亮
    void OLED_FourPoints(u8 x0, u8 y0, bit mode);                           // 4个像素点为一组，控制亮灭
    void OLED_Mask_Gray(u8 x0, u8 y0, u8 BMP[], bit mode);                  // 蒙版函数，衍生于OLED_FourPoints函数
    // void OLED_DrawLineG(u8 x1, u8 y1, u8 x2, u8 y2, bit mode);              // OLED画一条线，0：熄灭 1：点亮
    void OLED_DrawLineG_Dot(u8 x1, u8 y1, u8 x2, u8 y2, bit mode, u8 dot);  // OLED画一条虚实线，0：熄灭 1：点亮
    // void OLED_DrawRectangleG(u8 x1, u8 y1, u8 x2, u8 y2, bit mode);         // OLED画一个矩形
    void OLED_DrawFrameG(u8 x0, u8 y0, u8 x1, u8 y1, bit mode);             // OLED画一个方框
    void OLED_DrawBlockG(u8 x0, u8 y0, u8 x1, u8 y1, bit mode);             // OLED画一个实心框
    void OLED_DrawBox(u8 x0, u8 y0, u8 x1, u8 y1, u8 fill, bit mode);       // OLED画空实心矩形
    void OLED_DrawCircleG(u8 x, u8 y, u8 r, bit mode);                      // OLED画一个圆形
    void OLED_DrawEllipticG(u8 x0, u8 y0, u8 R1, u8 R2, bit mode);          // OLED画一个椭圆形
    void OLED_DrawCircleG_Distance(u8 x0, u8 y0, u8 R, bit mode);           // OLED画一个圆形（勾股定理法）
    void OLED_DrawBlockCircleG_Distance(u8 x0, u8 y0, u8 R, bit mode);      // OLED画一个实心圆形（勾股定理法）
    void OLED_DrawCircleG_Bresenham(u8 x, u8 y, u8 r, bit mode);            // OLED画一个圆形(Bresenham法)
    void OLED_DrawBlockCircleG_Bresenham(u8 x, u8 y, u8 r, bit mode);       // OLED画一个实心圆形(Bresenham法)
    void OLED_DrawRoundRectangleG(u8 x0, u8 y0, u8 x1, u8 y1, u8 R, u8 fill, bit mode);       // OLED画一个圆角矩形(Bresenham法)
    void OLED_DrawWidthRoundRectangleG(u8 x0, u8 y0, u8 x1, u8 y1, u8 R, u8 width, bit mode); // OLED画一个宽边圆角矩形(Bresenham法)

    void OLED_DrawSin();                                                        // 画正弦函数
    void ButtonWave();                                                          // 按键控制显示方波

    void OLED_ShowCharG(u8 x, u8 y, u8 chr, u8 charSize, u8 Is_Reverse);        // 缓存显示一个字符
    void OLED_ShowStringG(u8 x, u8 y, char *chr, u8 charSize, u8 Is_Reverse);   // 缓存显示一个字符号串
    void OLED_ShowNumG(u8 x, u8 y, u32 num, u8 len, u8 Size, u8 Is_Reverse);    // 缓存显示一串数字
    void OLED_ShowChineseG(u8 x, u8 y, u8 no, u8 Is_Reverse);                   // 缓存显示汉字
    void OLED_ShowBMPG(u8 x0, u8 y0, u8 x1, u8 y1, u8 BMP[], u8 Is_Reverse);    // 缓存显示BMP图片

#else                                   //2：指令模式
#endif

    void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 charSize, u8 Is_Reverse);         // 早指定位置显示一个字符
    void OLED_ShowString(u8 x, u8 y, char *chr, u8 charSize, u8 Is_Reverse);    // 显示一个字符号串
    u32  OLED_Index(u8 m, u8 n);                                                // m^n函数
    void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 Size, u8 Is_Reverse);     // 显示一串数字
    void OLED_ShowChinese(u8 x, u8 y, u8 no, u8 Is_Reverse);                    // 显示汉字
    void OLED_ShowBMP(u8 x0, u8 y0, u8 x1, u8 y1, u8 BMP[], u8 Is_Reverse);     // 显示BMP图片



#endif