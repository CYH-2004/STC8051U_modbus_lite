#ifndef __OLED12824_IIC_H_
#define __OLED12824_IIC_H_

#include "main.h"
// #include "AI8051U.h"

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

extern char WaveData[128];

#if     OLED_GRAM_Mode == 0             //0：8*128级缓存
    extern uint8_t OLED_GRAM[8][128];         // OLED全局缓存

#elif   OLED_GRAM_Mode == 1             //1:1024级缓存
    extern uint8_t OLED_GRAM[1024];           // OLED全局缓存

#else                                   //2：指令模式

#endif



//------------------------------引脚定义------------------------------






//------------------------------函数声明------------------------------

void OLED_Write_Byte(uint8_t dat, uint8_t cmd);               // 写入一个字节
void OLED_Init(void);                               // 初始化OLED
void OLED_Set_Pos(uint8_t x, uint8_t y);                      // OLED设置显示位置
void OLED_Display_On(void);                         // 开启OLED显示
void OLED_Display_Off(void);                        // 关闭OLED显示
void OLED_LightSet(uint8_t num);                         // 亮度设置
void OLED_Clear(uint8_t mode);                          // 清屏函数

#if     OLED_GRAM_Mode == 0 || OLED_GRAM_Mode == 1           //0：8*128级缓存//1:1024级缓存

// #elif   OLED_GRAM_Mode == 1             

    void OLED_GRAM_Clear(uint8_t mode);                                         // 通过缓冲数组，OLED屏幕全亮全灭
    void OLED_Refresh_Gram(void);                                           // OLED刷新显示
    void OLED_RefreshPart_Gram(uint8_t xstart, uint8_t ystart, uint8_t width, uint8_t height);  // OLED局部刷新显示

    void OLED_DrawPoint(uint8_t x, uint8_t y, uint8_t mode);                              // OLED画一个点，0：熄灭 1：点亮
    void OLED_FourPoints(uint8_t x0, uint8_t y0, uint8_t mode);                           // 4个像素点为一组，控制亮灭
    void OLED_Mask_Gray(uint8_t x0, uint8_t y0, uint8_t BMP[], uint8_t mode);                  // 蒙版函数，衍生于OLED_FourPoints函数
    // void OLED_DrawLineG(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode);              // OLED画一条线，0：熄灭 1：点亮
    void OLED_DrawLineG_Dot(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode, uint8_t dot);  // OLED画一条虚实线，0：熄灭 1：点亮
    // void OLED_DrawRectangleG(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t mode);         // OLED画一个矩形
    void OLED_DrawFrameG(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t mode);             // OLED画一个方框
    void OLED_DrawBlockG(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t mode);             // OLED画一个实心框
    void OLED_DrawBox(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t fill, uint8_t mode);       // OLED画空实心矩形
    void OLED_DrawCircleG(uint8_t x, uint8_t y, uint8_t r, uint8_t mode);                      // OLED画一个圆形
    void OLED_DrawEllipticG(uint8_t x0, uint8_t y0, uint8_t R1, uint8_t R2, uint8_t mode);          // OLED画一个椭圆形
    void OLED_DrawCircleG_Distance(uint8_t x0, uint8_t y0, uint8_t R, uint8_t mode);           // OLED画一个圆形（勾股定理法）
    void OLED_DrawBlockCircleG_Distance(uint8_t x0, uint8_t y0, uint8_t R, uint8_t mode);      // OLED画一个实心圆形（勾股定理法）
    void OLED_DrawCircleG_Bresenham(uint8_t x, uint8_t y, uint8_t r, uint8_t mode);            // OLED画一个圆形(Bresenham法)
    void OLED_DrawBlockCircleG_Bresenham(uint8_t x, uint8_t y, uint8_t r, uint8_t mode);       // OLED画一个实心圆形(Bresenham法)
    void OLED_DrawRoundRectangleG(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t R, uint8_t fill, uint8_t mode);       // OLED画一个圆角矩形(Bresenham法)
    void OLED_DrawWidthRoundRectangleG(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t R, uint8_t width, uint8_t mode); // OLED画一个宽边圆角矩形(Bresenham法)

    void OLED_DrawSin(void);                                                        // 画正弦函数
    // void ButtonWave(void);                                                          // 按键控制显示方波

    void OLED_ShowCharG(uint8_t x, uint8_t y, uint8_t chr, uint8_t charSize, uint8_t Is_Reverse);        // 缓存显示一个字符
    void OLED_ShowStringG(uint8_t x, uint8_t y, char *chr, uint8_t charSize, uint8_t Is_Reverse);   // 缓存显示一个字符号串
    void OLED_ShowNumG(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t Size, uint8_t Is_Reverse);    // 缓存显示一串数字
    void OLED_ShowChineseG(uint8_t x, uint8_t y, uint8_t no, uint8_t Is_Reverse);                   // 缓存显示汉字
    void OLED_ShowBMPG(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[], uint8_t Is_Reverse);    // 缓存显示BMP图片

#else                                   //2：指令模式
#endif

    void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t charSize, uint8_t Is_Reverse);         // 早指定位置显示一个字符
    void OLED_ShowString(uint8_t x, uint8_t y, char *chr, uint8_t charSize, uint8_t Is_Reverse);    // 显示一个字符号串
    uint32_t  OLED_Index(uint8_t m, uint8_t n);                                                // m^n函数
    void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len, uint8_t Size, uint8_t Is_Reverse);     // 显示一串数字
    void OLED_ShowChinese(uint8_t x, uint8_t y, uint8_t no, uint8_t Is_Reverse);                    // 显示汉字
    void OLED_ShowBMP(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t BMP[], uint8_t Is_Reverse);     // 显示BMP图片



#endif
