#include "oled12824_iic.h"
#include "iic.h"
#include "oled_font.h"
#include <INTRINS.H>
#include <stdlib.h>
#include "math.h"
#include "SysConfig.h"
// #define  MAIN_Fosc      24000000UL  //ISP下载时需将工作频率设置为24MHz

//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127


char xdata WaveData[128];
#if     OLED_GRAM_Mode == 0             //0：8*128级缓存
    u8 xdata OLED_GRAM[8][128];         // OLED全局缓存

#elif   OLED_GRAM_Mode == 1             //1:1024级缓存
    u8 xdata OLED_GRAM[1024];           // OLED全局缓存

#else                                   //2：指令模式

#endif



void delay_oled_ms(u16 ms)
{
    u16 xdata i;
    do{
        i = MAIN_Fosc / 6000;
          while(--i);   //6T per loop
    }while(--ms);
}


/**
 * @name OLED_Write_Byte
 * @brief 向SSD1306写入一个字节
 * @param dat:要写入的命令/数据
 * @param cmd:命令/数据标志 0：表示命令 1：表示数据
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：
 */
void OLED_Write_Byte(u8 dat,u8 cmd)         //写入一个字节
{
    if (cmd)
        IIC_Write_Byte(SLAVE_OLED_12864, OLED_WriteData_Addr, &dat, 1);
    else
        IIC_Write_Byte(SLAVE_OLED_12864, OLED_WriteCom_Addr, &dat, 1);
}

/*
显存地址模式 指令（20h）高位
00 水平地址模式 （00h）低位 ：之字形排列，当数据地址超出右边界时，会自动下移一行并在最左边显示；而最后一行超出时会在第一行显示。
01 垂直地址模式 （01h）低位：
10 页地址模式   （02h）低位：与水平地址模式类似，但超出边界时，会在同行的最左边显示，而不是下一行。
11 无效模式     （03h）低位：
*/
/**
 * @name OLED_Init
 * @brief 初始化SSD1306
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-31
 * @author 作者：汐
 * @note 注释：
 */
void OLED_Init(void)        //初始化OLED
{
    IIC_Init();
    delay_oled_ms(100);     //稍作延时，等单片机系统稳定再进行配置，否则有的型号的屏幕芯片会初始化失败
    delay_oled_ms(200);

    OLED_Write_Byte(0xAE, OLED_CMD);//---ae:设置显示关 af:设置显示开启

    OLED_Write_Byte(0x20, OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02) 设置内存地址模式
    OLED_Write_Byte(0x02, OLED_CMD);//00：水平地址模式 01：垂直模式 02：页地址模式

#if     OLED_SIZE == 0      //0.96寸
    OLED_Write_Byte(0x00, OLED_CMD);//---设置列起始地址低位
#elif   OLED_SIZE == 1      //1.30寸
    OLED_Write_Byte(0x02, OLED_CMD);//---设置列起始地址低位
#endif
    OLED_Write_Byte(0x10, OLED_CMD);//---设置列起始地址高位

    OLED_Write_Byte(0x40, OLED_CMD);//--- 设置显示开始行(0x40~0x7F)：显存中的第几行作为屏幕的第一行，之前的内容补在屏幕下方
    OLED_Write_Byte(0xb0, OLED_CMD);//--设置目标显示位置页的起始地址：b0-b7

    OLED_Write_Byte(0x81, OLED_CMD);//---为 BANK0 设置对比度控制
    OLED_Write_Byte(Brightness, OLED_CMD);//--0~255;默认0X7F (亮度设置,越大越亮)

    OLED_Write_Byte(0xA4, OLED_CMD);//---A4h显示显存内容，A5h无视显存内容点亮全屏

    OLED_Write_Byte(0xA6, OLED_CMD);//---设置屏幕显示模式 a6:设置正常显示 a7:反白显示

    OLED_Write_Byte(0xa1, OLED_CMD);//---设置画面显示方向 a0左右反置 a1正常
    OLED_Write_Byte(0xC8, OLED_CMD);//---设置画面显示方向 c0上下反置 c8正常

    OLED_Write_Byte(0xA8, OLED_CMD);//---设置复用率（0-63）
    OLED_Write_Byte(0x3f, OLED_CMD);//--00h-3fh

    OLED_Write_Byte(0xD3, OLED_CMD);//--设置屏幕上下偏移
    OLED_Write_Byte(0x00/*±8的倍数*/, OLED_CMD);//--00h-ffh

    OLED_Write_Byte(0xd5, OLED_CMD);//---设置显示时钟分频率、振荡器频率（刷新率）
    OLED_Write_Byte(0xc0, OLED_CMD);//--10h-f0h,1-f,越大速度越快（适合调节手机拍摄）

    OLED_Write_Byte(0xD9, OLED_CMD);//---置重充电周期
    OLED_Write_Byte(0xF1, OLED_CMD);//--Set Pre-Charge as 15 Clocks & Discharge as 1 Clock

    OLED_Write_Byte(0xDA, OLED_CMD);//---设置 COM 引脚硬件配置
    OLED_Write_Byte(0x12, OLED_CMD);//--12864:0x12，12832:0x02

    OLED_Write_Byte(0xDB, OLED_CMD);//---设置 Vcomh 反压值
    OLED_Write_Byte(0x30, OLED_CMD);//--Set VCOM Deselect Level

    
    OLED_Write_Byte(0x8D, OLED_CMD);//--- 设置电荷泵开启，升压允许
    OLED_Write_Byte(0x14, OLED_CMD);//--set(0x10) disable set(0x14) enable
    
    // OLED_Write_Byte(0xD8,OLED_CMD);//set area color mode off
    // OLED_Write_Byte(0x05,OLED_CMD);//
    
    OLED_Write_Byte(0xAF, OLED_CMD);//---设置显示开（进入正常工作模式）

    OLED_GRAM_Clear(0);
    OLED_Set_Pos(0, 0);
    OLED_Display_On();
}

/**
 * @name OLED_Set_Pos
 * @brief 指定要操作的像素点的坐标（地址）
 * @param x: 0~127
 * @param y: 0~7
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：根据手册指令，地址的低4位指令为00h~0fh，地址的高4位指令为10h~1fh。
 *  假设此时x=100 -> x=0x64 -> 0x64&0xf0=0x60 -> 0x60>>4=0x06 -> 0x06|0x10=0x16 ，即高4位为6
 *  而0x64&0x0f=0x04 ，即低4位为4。结合起来即64h。
 */
void OLED_Set_Pos(u8 x,u8 y)                //OLED设置显示位置
{
    OLED_Write_Byte((u8)(0XB0 + y), OLED_CMD);  //设置Y坐标，查看手册输入0Xb0就是起始地址
#if     OLED_SIZE == 0      //0.96寸
    OLED_Write_Byte(((x & 0xf0) >> 4) | 0x10, OLED_CMD);        //发送坐标的高4位的值 
    OLED_Write_Byte((x & 0x0f), OLED_CMD);                      //发送坐标的低4位的值 

#elif   OLED_SIZE == 1      //1.30寸    
    OLED_Write_Byte((((x + 2) & 0xf0) >> 4) | 0x10, OLED_CMD);  //发送坐标的高4位的值
    OLED_Write_Byte(((x + 2) & 0x0f), OLED_CMD);                //发送坐标的低4位的值

#endif
}

/**
 * @name OLED_Display_On
 * @brief 开启OLED显示
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：
 */
void OLED_Display_On(void)              //开启OLED显示
{
    OLED_Write_Byte(0x8d, OLED_CMD);    //设置 DCDC命令
    OLED_Write_Byte(0x14, OLED_CMD);    //DCDC ON
    OLED_Write_Byte(0xaf, OLED_CMD);    //显示开启
}

/**
 * @name OLED_Display_Off
 * @brief 关闭OLED显示
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：
 */
void OLED_Display_Off(void)             //关闭OLED显示
{
    OLED_Write_Byte(0x8d, OLED_CMD);    //设置 DCDC命令
    OLED_Write_Byte(0x10, OLED_CMD);    //DCDC OFF
    OLED_Write_Byte(0xae, OLED_CMD);    //显示关闭
}

/**
 * @name OLED_LightSet
 * @brief OLED亮度设置
 * @param num：0~255
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：
 */
void OLED_LightSet(u8 num)     //亮度设置
{
    OLED_Write_Byte(0x81,OLED_CMD);     //
    OLED_Write_Byte(num,OLED_CMD);      //  
    OLED_Write_Byte(0xDB,OLED_CMD);     //--set vcomh
    OLED_Write_Byte(0x20,OLED_CMD);     //Set VCOM Deselect Level   
}

/**
 * @name OLED_Clear
 * @brief 清屏函数
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：清完屏,0：整个屏幕是黑色的和没点亮一样 1：整个屏幕是全亮的
 */
void OLED_Clear(bit mode)       //清屏函数
{
    u8 xdata i, n;
    for (i = 0; i<8; i++)       //循环8行
    {
        OLED_Write_Byte((u8)(0xb0 + i), OLED_CMD);    //设置页地址（0~7）
        #if     OLED_SIZE == 0      //0.96寸
        OLED_Write_Byte(0x00, OLED_CMD);//---set low column address	设置低列地址
        #elif   OLED_SIZE == 1      //1.30寸
        OLED_Write_Byte(0x02, OLED_CMD);//---set low column address	设置低列地址
        #endif
        OLED_Write_Byte(0x10, OLED_CMD);        //设置显示位置—列高地址
        for (n = 0; n < Max_Column; n++)        // 循环128列
            if(mode)
            OLED_Write_Byte(0xff, OLED_DATA);      //点亮一个灯组里德8个LED
            else
            OLED_Write_Byte(0x00, OLED_DATA);      //熄灭一个灯组里德8个LED
    }//更新显示
}

/**
 * @name OLED_Index
 * @brief m^n函数
 * @param m^n
 * @return result
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：
 */
u32 OLED_Index(u8 m, u8 n)  //m^n函数
{
    u32 result = 1;
    while (n--)result *= m;
    return result;
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#if     OLED_GRAM_Mode == 0             //0：8*128级缓存

/**
 * @name OLED_GRAM_Clear
 * @brief 通过缓冲数组，让OLED屏幕 
 * @param mode:0：全灭 1：全亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-3
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_GRAM_Clear(bit mode)       //通过缓冲数组，让OLED屏幕全亮全灭
{
    u8 xdata x, y;

    for ( y = 0; y < 8; y++)
    {
        for ( x = 0; x < 128; x++)
        {
            if (mode)
                OLED_GRAM[y][x] = 0xff;
            else
                OLED_GRAM[y][x] = 0x00;
        }
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_Refresh_Gram
 * @brief OLED刷新显示
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-3
 * @author 作者：汐
 * @note 注释：
 */
void OLED_Refresh_Gram(void)            //OLED刷新显示
{
    u8 xdata x,y;
    for(y=0;y<8;y++)
    {
        OLED_Set_Pos(0, y);
        for (x = 0; x < 128; x++)
            OLED_Write_Byte(OLED_GRAM[y][x], OLED_DATA);
    }
}

/**
 * @name OLED_RefreshPart_Gram
 * @brief OLED局部刷新显示
 * @param xstart:需要刷新区域的起始（左上角）坐标值
 * @param ystart:需要刷新区域的起始（左上角）坐标值
 * @param width:需要刷新的区域宽度有多少列，0-127
 * @param height:需要刷新的区域高度有多少行，0-7
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-3
 * @author 作者：汐
 * @note 注释：
 */
void OLED_RefreshPart_Gram(u8 xstart, u8 ystart, u8 width, u8 height)            //OLED局部刷新显示
{
    u8 x, y;
    for ( y = ystart; y < ( ystart + height ); y++)
    {
        OLED_Set_Pos(xstart, y);
        for ( x = xstart; x <  (xstart + width ); x++)
        {
            OLED_Write_Byte(OLED_GRAM[y][x], OLED_DATA);
        }
    }
}

/**
 * @name OLED_DrawPoint
 * @brief OLED画一个点，0：熄灭 1：点亮
 * @param x:横向坐标 0-127
 * @param y:竖向坐标 0-63
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-3
 * @author 作者：
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawPoint(u8 x,u8 y,bit mode)     //OLED画一个点，0：熄灭 1：点亮
{
    if((x<0)||(x>127)||(y<0)||(y>63))
        return;     //防止超出屏幕
    if( mode )
        OLED_GRAM[y / 8][x] |= 0x01 << (y % 8);
    else
        OLED_GRAM[y / 8][x] &= ~(0x01 << (y % 8));
    // OLED_Refresh_Gram();                    //启用这个后续画线等，将会逐点绘制，但耗时长
}

/**
 * @name OLED_FourPoints
 * @brief 4个像素点为一组，控制一组中的一个灯亮灭
 * @param x0:起始位置0-1
 * @param y0:起始位置0-1
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-13
 * @author 作者：
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_FourPoints(u8 x0,u8 y0,bit mode)          //4个像素点为一组，控制亮灭
{
    u8 xdata x, y;
    for ( y = y0; y < 64; y+=2)
    {
        for ( x = x0; x < 128; x+=2)
        {
            if(mode)
                OLED_GRAM[y / 8][128 + x] |= 0x01 << (y % 8);
            else
                OLED_GRAM[y / 8][128 + x] &= ~(0x01 << (y % 8));
        }
    }
}

/**
 * @name OLED_Mask_Gray
 * @brief 灰阶蒙版显示一个图片，衍生于OLED_FourPoints函数
 * @param x0:起始位置
 * @param y0:起始位置
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-13
 * @author 作者：
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_Mask_Gray(u8 x0, u8 y0, u8 BMP[], bit mode)  //蒙版函数，衍生于OLED_FourPoints函数
{
    u8 xdata x, y;
    bit z;
    for ( y = y0; y < 64; y+=2)
    {
        for ( x = x0; x < 128; x+=3)    //6阶灰度
        {
            z = BMP[128 * (y / 8) + x] & (0x01 << (y % 8)); // 将蒙版上的像素点坐标，对应BMP数组里相同位置的那个点的数值，赋值给变量z
            if(z==1)
            {
                if(mode)
                OLED_DrawPoint(x, y, 1);
                else
                OLED_DrawPoint(x, y, 0);
            }
        }
    }
}

/**
 * @name OLED_DrawLineG_Dot
 * @brief OLED画一条虚实线(微分法)
 * @param x1y1：起点坐标
 * @param x2y2：终点坐标
 * @param mode:0：熄灭 1：点亮
 * @param dot:虚实密度,1：实线
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawLineG_Dot( u8 x1, u8 y1, u8 x2, u8 y2 , bit mode , u8 dot)      //OLED画一条虚实线，0：熄灭 1：点亮
{
    //先计算增量Δy和Δx
    u8 DeltaX = abs(x2 - x1);
    u8 DeltaY = abs(y2 - y1);
    u8 dm, i;           //定义分割基准，循环变量
    float dx, dy;       //定义变化量
    float x, y;         //定义需要显示的坐标值

    if(DeltaX >= DeltaY)  //选择较长的边为基准
        dm = DeltaX;
    else
        dm = DeltaY;

    dx = (float)(x2 - x1) / dm;
    dy = (float)(y2 - y1) / dm;
    x = (float)x1 + 0.5;    //四舍五入
    y = (float)y1 + 0.5;    //四舍五入
    for ( i = 0; i <= dm; i++)
    {
        if(!(i%dot) || i==1)
        OLED_DrawPoint(x, y, mode);
        x += dx;
        y += dy;
    }
}

/**
 * @name OLED_DrawFrameG
 * @brief OLED画一个方框
 * @param x0y0：起点坐标
 * @param x1y1：终点坐标
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawFrameG(u8 x0, u8 y0, u8 x1, u8 y1, bit mode)  //OLED画一个方框
{
    OLED_DrawLineG_Dot(x0, y0, x1, y0, mode, 1);
    OLED_DrawLineG_Dot(x1, y0, x1, y1, mode, 1);
    OLED_DrawLineG_Dot(x1, y1, x0, y1, mode, 1);
    OLED_DrawLineG_Dot(x0, y1, x0, y0, mode, 1);
}

/**
 * @name OLED_DrawBlockG
 * @brief OLED画一个实心框
 * @param x0y0：起点坐标
 * @param x1y1：终点坐标
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawBlockG(u8 x0, u8 y0, u8 x1, u8 y1, bit mode)  //OLED画一个实心框
{
    char i, d;

    if(y1-y0 >= 0)
        d = 1;
    else
        d = -1;
    
    for ( i = 0; i <= abs(y1-y0); i++)
    {
        OLED_DrawLineG_Dot(x0, (u8)(y0 + i * d), x1, (u8)(y0 + i * d), mode, 1);
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawBox
 * @brief 画空实心矩形
 * @param x0y0：起点坐标
 * @param x1y1：终点坐标
 * @param fill:0：空心 1：实心
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawBox(u8 x0, u8 y0, u8 x1, u8 y1, u8 fill, bit mode)        //画空实心矩形
{
    int xstart, ystart, xend, yend, i;

    if(x0 < x1) { xstart = x0; xend = x1;}
    else        { xstart = x1; xend = x0;}
    if(y0 < y1) { ystart = y0; yend = y1;}
    else        { ystart = y1; yend = y0;}

    if(fill == 0)  //空心
    {
        OLED_DrawLineG_Dot((u8)xstart, (u8)ystart, (u8)xend, (u8)ystart, mode, 1);
        OLED_DrawLineG_Dot((u8)xstart, (u8)yend, (u8)xend, (u8)yend, mode, 1);
        OLED_DrawLineG_Dot((u8)xstart, (u8)ystart, (u8)xstart, (u8)yend, mode, 1);
        OLED_DrawLineG_Dot((u8)xend, (u8)ystart, (u8)xend, (u8)yend, mode, 1);
    }
    else if (fill == 1) //实心
    {
        for ( i = ystart; i <= yend; i++)
        {
            OLED_DrawLineG_Dot((u8)xstart, (u8)i, (u8)xend, (u8)i, mode, 1);
        }
    }
}

/**
 * @name OLED_DrawCircleG
 * @brief OLED画一个圆形
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawCircleG(u8 x0, u8 y0, u8 R, bit mode)      //OLED画一个圆形
{
    float Rx, Ry, angle, Rad;
    // OLED_DrawPoint(x0, y0, 1);

    for ( angle = 0; angle < 360; angle+=1)
    {
        Rad = angle * 3.14 / 180;

        Rx = R * cos(Rad);
        Ry = R * sin(Rad);

        OLED_DrawPoint(x0 + Rx, /*64 -*/ (y0 + Ry), mode);
        // OLED_Refresh_Gram();
    }
    OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawEllipticG
 * @brief OLED画一个椭圆形
 * @param xy：起点坐标
 * @param R1：长轴
 * @param R2：短轴
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawEllipticG(u8 x0, u8 y0, u8 R1, u8 R2, bit mode)      //OLED画一个椭圆形
{
    float Rx, Ry, angle, Rad;
    // OLED_DrawPoint(x0, y0, 1);

    for ( angle = 0; angle < 360; angle+=1)
    {
        Rad = angle * 3.14 / 180;

        Rx = R1 * cos(Rad);
        Ry = R2 * sin(Rad);

        OLED_DrawPoint(x0 + Rx, /*64 -*/ (y0 + Ry), mode);
        // OLED_Refresh_Gram();
    }
    OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawCircleG_Distance
 * @brief OLED画一个圆形（勾股定理法）
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawCircleG_Distance(u8 x0,u8 y0, u8 R, bit mode) //OLED画一个圆形（勾股定理法）
{
    int x = 0;
    int y = R;

    while (x<=y)
    {
        OLED_DrawPoint((u8)(x0 + x), (u8)(y0 + y), mode);   //右下1
        OLED_DrawPoint((u8)(x0 - x), (u8)(y0 + y), mode);   //左下1
        OLED_DrawPoint((u8)(x0 - x), (u8)(y0 - y), mode);   //左上1
        OLED_DrawPoint((u8)(x0 + x), (u8)(y0 - y), mode);   //右上1
        OLED_DrawPoint((u8)(x0 + y), (u8)(y0 + x), mode);   //右下2
        OLED_DrawPoint((u8)(x0 - y), (u8)(y0 + x), mode);   //左下2
        OLED_DrawPoint((u8)(x0 - y), (u8)(y0 - x), mode);   //左上2
        OLED_DrawPoint((u8)(x0 + y), (u8)(y0 - x), mode);   //右上2

        x++;
        if( (x*x+y*y)>(R*R))
            y--;
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawBlockCircleG_Distance
 * @brief OLED画一个实心圆形（勾股定理法）
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawBlockCircleG_Distance(u8 x0,u8 y0, u8 R, bit mode) //OLED画一个实心圆形（勾股定理法）
{
    int x = 0;
    int y = R;

    while (x<=y)
    {
        // OLED_DrawPoint((u8)(x0 + x), (u8)(y0 + y), mode);
        // OLED_DrawPoint((u8)(x0 - x), (u8)(y0 + y), mode);
        OLED_DrawLineG_Dot((u8)(x0 + x), (u8)(y0 + y), (u8)(x0 - x), (u8)(y0 + y), mode, 1);
        // OLED_DrawPoint((u8)(x0 - x), (u8)(y0 - y), mode);
        // OLED_DrawPoint((u8)(x0 + x), (u8)(y0 - y), mode);
        OLED_DrawLineG_Dot((u8)(x0 - x), (u8)(y0 - y), (u8)(x0 + x), (u8)(y0 - y), mode, 1);
        // OLED_DrawPoint((u8)(x0 + y), (u8)(y0 + x), mode);
        // OLED_DrawPoint((u8)(x0 - y), (u8)(y0 + x), mode);
        OLED_DrawLineG_Dot((u8)(x0 + y), (u8)(y0 + x), (u8)(x0 - y), (u8)(y0 + x), mode, 1);
        // OLED_DrawPoint((u8)(x0 - y), (u8)(y0 - x), mode);
        // OLED_DrawPoint((u8)(x0 + y), (u8)(y0 - x), mode);
        OLED_DrawLineG_Dot((u8)(x0 - y), (u8)(y0 - x), (u8)(x0 + y), (u8)(y0 - x), mode, 1);
        x++;
        if( (x*x+y*y)>(R*R))
            y--;
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawCircleG_Bresenham
 * @brief OLED画一个圆形(Bresenham法)
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawCircleG_Bresenham(u8 x0, u8 y0, u8 R, bit mode)      //OLED画一个圆形(Bresenham法)
{
    int x, y;
    int di;
    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    while (x <= y)
    {
        OLED_DrawPoint((u8)(x0 + x), (u8)(y0 + y), mode);   //右下1
        OLED_DrawPoint((u8)(x0 - x), (u8)(y0 + y), mode);   //左下1
        OLED_DrawPoint((u8)(x0 - x), (u8)(y0 - y), mode);   //左上1
        OLED_DrawPoint((u8)(x0 + x), (u8)(y0 - y), mode);   //右上1
        OLED_DrawPoint((u8)(x0 + y), (u8)(y0 + x), mode);   //右下2
        OLED_DrawPoint((u8)(x0 - y), (u8)(y0 + x), mode);   //左下2
        OLED_DrawPoint((u8)(x0 - y), (u8)(y0 - x), mode);   //左上2
        OLED_DrawPoint((u8)(x0 + y), (u8)(y0 - x), mode);   //右上2

        x++;
        //使用Bresenham算法画圆
        if (di < 0)
        di += 4 * x + 6;
        else
        {
            di += 10 + 4 * (x - y);
            y--;
        }
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawBlockCircleG_Bresenham
 * @brief OLED画一个实心圆形(Bresenham法)
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawBlockCircleG_Bresenham(u8 x0, u8 y0, u8 R, bit mode)      //OLED画一个实心圆形(Bresenham法)
{
    int x, y;
    int di;
    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    while (x <= y)
    {
        // OLED_DrawPoint((u8)(x0 + x), (u8)(y0 + y), mode);        //6
        // OLED_DrawPoint((u8)(x0 - x), (u8)(y0 + y), mode);        //1
        OLED_DrawLineG_Dot((u8)(x0 + x), (u8)(y0 + y), (u8)(x0 - x), (u8)(y0 + y), mode, 1);
        // OLED_DrawPoint((u8)(x0 - x), (u8)(y0 - y), mode);        //2
        // OLED_DrawPoint((u8)(x0 + x), (u8)(y0 - y), mode);        //5
        OLED_DrawLineG_Dot((u8)(x0 - x), (u8)(y0 - y), (u8)(x0 + x), (u8)(y0 - y), mode, 1);
        // OLED_DrawPoint((u8)(x0 + y), (u8)(y0 + x), mode);        //4
        // OLED_DrawPoint((u8)(x0 - y), (u8)(y0 + x), mode);        //3
        OLED_DrawLineG_Dot((u8)(x0 + y), (u8)(y0 + x), (u8)(x0 - y), (u8)(y0 + x), mode, 1);
        // OLED_DrawPoint((u8)(x0 - y), (u8)(y0 - x), mode);        //7
        // OLED_DrawPoint((u8)(x0 + y), (u8)(y0 - x), mode);        //0
        OLED_DrawLineG_Dot((u8)(x0 - y), (u8)(y0 - x), (u8)(x0 + y), (u8)(y0 - x), mode, 1);

        x++;
        //使用Bresenham算法画圆
        if (di < 0)
        di += 4 * x + 6;
        else
        {
            di += 10 + 4 * (x - y);
            y--;
        }
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}



/**
 * @name OLED_DrawRoundRectangleG
 * @brief OLED画一个圆角矩形(Bresenham法)
 * @param xy：矩形起点坐标
 * @param x1y1:矩形终点坐标
 * @param R：圆角半径
 * @param fill:0：空心 1：实心
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawRoundRectangleG(u8 x0, u8 y0, u8 x1, u8 y1, u8 R, u8 fill, bit mode)      //OLED画一个圆角矩形(Bresenham法)
{
    int x, y, di, i;
    int xstart, ystart, xend, yend;

    if(x0 < x1) { xstart = x0; xend = x1;}
    else        { xstart = x1; xend = x0;}
    if(y0 < y1) { ystart = y0; yend = y1;}
    else        { ystart = y1; yend = y0;}

    if (R > (u8)(xend - xstart) / 2 || R > (u8)(yend - ystart) / 2)
        R = ((u8)(xend - xstart) < (u8)(yend - ystart)) ? ((u8)(xend - xstart) / 2) : ((u8)(yend - ystart) / 2);

    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    
    if( fill == 0)
    {   //空心
        while (x <= y)
        {
            OLED_DrawPoint((u8)(xstart + R - x), (u8)(ystart + R - y), mode);   //左上1
            OLED_DrawPoint((u8)(xstart + R - y), (u8)(ystart + R - x), mode);   //左上2
            OLED_DrawPoint((u8)(xend - R + x), (u8)(ystart + R - y), mode);   //右上1
            OLED_DrawPoint((u8)(xend - R + y), (u8)(ystart + R - x), mode);   //右上2
            OLED_DrawPoint((u8)(xstart + R - x), (u8)(yend - R + y), mode);   //左下1
            OLED_DrawPoint((u8)(xstart + R - y), (u8)(yend - R + x), mode);   //左下2
            OLED_DrawPoint((u8)(xend - R + x), (u8)(yend - R + y), mode);   //右下1
            OLED_DrawPoint((u8)(xend - R + y), (u8)(yend - R + x), mode);   //右下2

            
            //使用Bresenham算法画圆
            if (di < 0)
            di += 4 * x + 6;
            else
            {
                di += 10 + 4 * (x - y);
                y--;
            }
            x++;
            // OLED_Refresh_Gram();
        }

        OLED_DrawLineG_Dot((u8)(xstart + R + 1), (u8)ystart, (u8)(xend - R - 1), (u8)ystart, mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R + 1), (u8)yend, (u8)(xend - R - 1), (u8)yend, mode, 1);
        OLED_DrawLineG_Dot((u8)xstart, (u8)(ystart + R + 1), (u8)xstart, (u8)(yend - R - 1), mode, 1);
        OLED_DrawLineG_Dot((u8)xend, (u8)(ystart + R + 1), (u8)xend, (u8)(yend - R - 1), mode, 1);
    }
    else if( fill == 1)
    {   //实心
        while (x <= y)
        {
            OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(ystart + R - y), (u8)(xend - R + x), (u8)(ystart + R - y), mode, 1);
            OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(ystart + R - x), (u8)(xend - R + y), (u8)(ystart + R - x), mode, 1);
            OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(yend - R + y), (u8)(xend - R + x), (u8)(yend - R + y), mode, 1);
            OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(yend - R + x), (u8)(xend - R + y), (u8)(yend - R + x), mode, 1);
            
            //使用Bresenham算法画圆
            if (di < 0)
            di += 4 * x + 6;
            else
            {
                di += 10 + 4 * (x - y);
                y--;
            }
            x++;
            // OLED_Refresh_Gram();
        }

        for ( i = ystart + R + 1; i <= yend - R -1; i++)
        {
            OLED_DrawLineG_Dot((u8)xstart, (u8)i, (u8)xend, (u8)i, mode, 1);
        }
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawWideRoundRectangleG
 * @brief OLED画一个宽边圆角矩形(Bresenham法)
 * @param xy：矩形起点坐标
 * @param x1y1:矩形终点坐标
 * @param R：圆角半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawWidthRoundRectangleG(u8 x0, u8 y0, u8 x1, u8 y1, u8 R, u8 width, bit mode)      //OLED画一个宽边圆角矩形(Bresenham法)
{
    int x, y, di;
    int xstart, ystart, xend, yend;

    if (width <= 1)
    {
        OLED_DrawRoundRectangleG(x0, y0, x1, y1, R, 0, 1);
        return;
    }
    

    //画大扇形
    if(x0 < x1) { xstart=x0; xend = x1;}
    else        { xstart=x1; xend = x0;}
    if(y0 < y1) { ystart=y0; yend = y1;}
    else        { ystart=y1; yend = y0;}

    if (R > (u8)(xend - xstart) / 2 || R > (u8)(yend - ystart) / 2)
        R = ((u8)(xend - xstart) < (u8)(yend - ystart)) ? ((u8)(xend - xstart) / 2) : ((u8)(yend - ystart) / 2);

    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    

    while (x <= y)
    {
        OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(ystart + R - y), (u8)(xstart + R - x), (u8)(ystart + R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(ystart + R - x), (u8)(xstart + R - y), (u8)(ystart + R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + x), (u8)(ystart + R - y), (u8)(xend - R + x), (u8)(ystart + R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + y), (u8)(ystart + R - x), (u8)(xend - R + y), (u8)(ystart + R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(yend - R + y), (u8)(xstart + R - x), (u8)(yend - R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(yend - R + x), (u8)(xstart + R - y), (u8)(yend - R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + x), (u8)(yend - R + y), (u8)(xend - R + x), (u8)(yend - R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + y), (u8)(yend - R + x), (u8)(xend - R + y), (u8)(yend - R), mode, 1);
        
        //使用Bresenham算法画圆
        if (di < 0)
        di += 4 * x + 6;
        else
        {
            di += 10 + 4 * (x - y);
            y--;
        }
        x++;
    }

    OLED_DrawBox((u8)(xstart + R + 1), (u8)(ystart), (u8)(xend - R - 1), (u8)(ystart + width - 1), 1, mode);
    OLED_DrawBox((u8)(xstart + R + 1), (u8)(yend - width + 1), (u8)(xend - R - 1), (u8)(yend), 1, mode);
    OLED_DrawBox((u8)(xstart), (u8)(ystart + R + 1), (u8)(xstart + width - 1), (u8)(yend - R - 1), 1, mode);
    OLED_DrawBox((u8)(xend - width + 1), (u8)(ystart + R + 1), (u8)(xend), (u8)(yend - R - 1), 1, mode);

    //画擦除小扇形
    if(x0 < x1) { xstart = x0 + width; xend = x1 - width;}
    else        { xstart = x1 + width; xend = x0 - width;}
    if(y0 < y1) { ystart = y0 + width; yend = y1 - width;}
    else        { ystart = y1 + width; yend = y0 - width;}
    R -= width;
    if (R > (u8)(xend - xstart) / 2 || R > (u8)(yend - ystart) / 2)
        R = ((u8)(xend - xstart) < (u8)(yend - ystart)) ? ((u8)(xend - xstart) / 2) : ((u8)(yend - ystart) / 2);

    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    

    while (x <= y)
    {
        OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(ystart + R - y), (u8)(xstart + R - x), (u8)(ystart + R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(ystart + R - x), (u8)(xstart + R - y), (u8)(ystart + R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + x), (u8)(ystart + R - y), (u8)(xend - R + x), (u8)(ystart + R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + y), (u8)(ystart + R - x), (u8)(xend - R + y), (u8)(ystart + R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(yend - R + y), (u8)(xstart + R - x), (u8)(yend - R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(yend - R + x), (u8)(xstart + R - y), (u8)(yend - R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + x), (u8)(yend - R + y), (u8)(xend - R + x), (u8)(yend - R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + y), (u8)(yend - R + x), (u8)(xend - R + y), (u8)(yend - R), ~mode, 1);
        
        //使用Bresenham算法画圆
        if (di < 0)
        di += 4 * x + 6;
        else
        {
            di += 10 + 4 * (x - y);
            y--;
        }
        x++;
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawSin
 * @brief 画正弦函数
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024
 * @author 作者：
 * @note 注释：
 */
void OLED_DrawSin()     //画正弦函数
{
    u8 a = 8;   //X轴系数
    u8 b = 20;  //Y轴系数
    u8 x0 = 40, y0 = 32;    //起始点，结合后文，最好控制在中心点
    //x0 = 63 - 180/a

    float Si, Sx, Sy, Rad;

    for ( Si = 0; Si <= 360; Si+= 1)
    {
        Rad = Si * 3.14 / 180;
        Sx = Si / /*系数*/ a;               //最左端：x0+0*a，最右端：x0+360/a
        Sy = sin(Rad) * /*系数*/ b;     //最高点：y0-1*b，最低点：y0+1*b
        OLED_DrawPoint(Sx + x0, 64 - (Sy + y0), 1);
        OLED_RefreshPart_Gram(30, 1, 60, 6);

        OLED_ShowNum(35, 7, Si, 3, 6, 0);
    }
}

/**
 * @name ButtonWave
 * @brief 按键控制显示方波
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：
 * @note 注释：
 */
void ButtonWave()   //按键控制显示方波
{
    u8 Xtime;

    char Range01 = 50;
    char Range02 = 10;
    for ( Xtime = 127; Xtime > 0; Xtime--)
        WaveData[Xtime] = WaveData[Xtime - 1];

    if(P54)
        WaveData[0] = Range02;
    else
        WaveData[0] = Range01;

    OLED_GRAM_Clear(0);

    for ( Xtime = 0; Xtime < 127; Xtime++)
        OLED_DrawLineG_Dot(Xtime, (u8)(63 - WaveData[Xtime]), (u8)(Xtime + 1), (u8)(63 - WaveData[Xtime + 1]), 1, 1);

    OLED_Refresh_Gram();
    delay_oled_ms(50);
}

/**
 * @name OLED_ShowCharG
 * @brief 在指定位置显示一个字符,包括部分字符
 * @param x:0~127
 * @param y:0~7
 * @param chr:
 * @param charSize:选择字体16(高度占2行)/6(高度占1行)
 * @param Is_Reverse:模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowCharG(u8 x,u8 y,u8 chr, u8 charSize, u8 Is_Reverse)     // 缓存显示一个字符
{
    u8 xdata i = 0;
    chr -= ' ';	//得到偏移后的值
    if (x > Max_Column - 1)     //如果起点横坐标超过了屏幕最右侧
    {
        x = 0;                  //列坐标x置零，回到屏幕最左侧
        y = y + 2;              //行坐标y下移2行
    }                           //将字符显示在屏幕指定行的下一行起点位置
    if (charSize == 16)                 //判断字号大小，读取F16X8字库，占2行
    {   
        for (i = 0; i<8; i++)           //两行高度的上行
            OLED_GRAM[y][(x + i)] = (Is_Reverse == 0 ? F16X8[chr][i] : ~F16X8[chr][i]);;
        for (i = 0; i<8; i++)           //两行高度的下行
            OLED_GRAM[y+1][(x + i)] = (Is_Reverse == 0?F16X8[chr][i + 8]:~F16X8[chr][i + 8]);
    }
    else if(charSize == 6)              //判断字号大小，读取F8X6字库，占1行
    {
        for (i = 0; i<6; i++)
            OLED_GRAM[y][(x + i)] = (Is_Reverse == 0?F8X6[chr][i]:~F8X6[chr][i]);
    }
}

/**
 * @name OLED_ShowStringG
 * @brief 显示一个字符号串
 * @param xy：起点坐标
 * @param chr：需要显示的字符串
 * @param charSize：字体大小16/6
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowStringG(u8 x, u8 y, char *chr, u8 charSize, u8 Is_Reverse) //显示一个字符号串
{
    u8 xdata j = 0;
    while (chr[j] != '\0')      //读取到字符串结束符之前，一直循环
    {
        OLED_ShowCharG(x, y, chr[j], charSize, Is_Reverse);  //从第一个字符开始显示
        x += 8;         //显示完一个字符后，起点x坐标右移8格，准备显示下一个字符
        if (x>120)      //如果起点x位置大于120，为保证字符显示完整，在下一行最左侧位置显示
        {
            x = 0;
            y += 2;
        }
        j++;            //指针移到下一个字符
    }
}

/**
 * @name OLED_ShowNumG
 * @brief 缓存显示一串数字
 * @param xy：起点坐标
 * @param num：数值(0~4294967295)
 * @param len：数字的位数，即显示几位有效数字,max=10
 * @param Size：字体大小16/6
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowNumG(u8 x, u8 y, u32 num, u8 len, u8 Size, u8 Is_Reverse)  //缓存显示一串数字
{
    u8 xdata t, temp;
    u8 xdata enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / OLED_Index(10, (u8)(len - t - 1))) % 10;      //取第t位数字
        if (enshow == 0 && t < (len - 1))       //如果数串长度小于指定长度
        {
            if (temp == 0)                      //多出来的位用空格补齐
            {
                OLED_ShowCharG((u8)(x + 8 * t), y, ' ', Size, Is_Reverse);
                continue;                       //继续循环判断
            }
            else
                enshow = 1;                     //直到有数字为止
        }
        OLED_ShowCharG((u8)(x + 8 * t), y, (u8)(temp + '0'), Size, Is_Reverse);  //从高到低逐位显示数字
    }
}

/**
 * @name OLED_ShowChineseG
 * @brief 显示汉字
 * @param xy：起点坐标
 * @param no：需要显示的字符串
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowChineseG(u8 x, u8 y, u8 no, u8 Is_Reverse) //缓存显示汉字
{
    u8 xdata i;
    for (i = 0; i < 16; i++)    //汉字上半部分坐标（第x列，第y行）
    {
        OLED_GRAM[y][(x + i)] = (Is_Reverse == 0?Hzk[2 * no][i]:~Hzk[2 * no][i]);
    }
    for (i = 0; i<16; i++)      //汉字下半部分坐标（第x列，第y+1行）
    {
        OLED_GRAM[y][(x + i)] = (Is_Reverse == 0?Hzk[2 * no + 1][i]:~Hzk[2 * no + 1][i]);
    }
}

/**
 * @name OLED_ShowBMPG
 * @brief 缓存显示BMP图片
 * @param x0y0：起始点坐标(x,y),x的范围0-127，y为页的范围0-7
 * @param x1y1：图片的像素（与实际取模的图片像素一致）
 * @param BMP[]：需要显示BMP
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-31
 * @author 作者：汐
 * @note 注释：x1-x0=图片的实际像素长度 y1-y0=图片的实际像素宽度
 *           通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowBMPG(u8 x0, u8 y0, u8 x1, u8 y1, u8 BMP[], u8 Is_Reverse)  //缓存显示BMP图片
{
    u16 xdata j = 0;
    u8 xdata x, y;
    if (y1 % 8 == 0) 
		y1 = y1 / 8;
    else 
		y1 = y1 / 8 + 1;
    for (y = y0; y<y1; y++)
    {
        for (x = x0; x<x1; x++)
        {
            OLED_GRAM[y0][x] = (Is_Reverse == 0 ? BMP[j++] : ~BMP[j++], OLED_DATA);
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#elif   OLED_GRAM_Mode == 1             //1:1024级缓存

/**
 * @name OLED_GRAM_Clear
 * @brief 通过缓冲数组，让OLED屏幕 
 * @param mode:0：全灭 1：全亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-3
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_GRAM_Clear(bit mode)       //通过缓冲数组，让OLED屏幕全亮全灭
{
    u16 xdata t;

        for ( t = 0; t < 1024; t++)
        {
            if (mode)
                OLED_GRAM[t] = 0xff;
            else
                OLED_GRAM[t] = 0x00;
        }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_Refresh_Gram
 * @brief OLED刷新显示
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-3
 * @author 作者：汐
 * @note 注释：
 */
void OLED_Refresh_Gram(void)            //OLED刷新显示
{
    u8 xdata x, y;
    u16 xdata j = 0;
    for ( y = 0; y < 8; y++)
    {
        OLED_Set_Pos(0, y);
        for ( x = 0; x < 128; x++)
        {
            OLED_Write_Byte(OLED_GRAM[j++], OLED_DATA);
        }
    }
}

/**
 * @name OLED_RefreshPart_Gram
 * @brief OLED局部刷新显示
 * @param xstart:需要刷新区域的起始（左上角）坐标值
 * @param ystart:需要刷新区域的起始（左上角）坐标值
 * @param width:需要刷新的区域宽度有多少列，0-127
 * @param height:需要刷新的区域高度有多少行，0-7
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-3
 * @author 作者：汐
 * @note 注释：
 */
void OLED_RefreshPart_Gram(u8 xstart, u8 ystart, u8 width, u8 height)            //OLED局部刷新显示
{
    u8 x, y;
    for ( y = ystart; y < ( ystart + height ); y++)
    {
        OLED_Set_Pos(xstart, y);
        for ( x = xstart; x <  (xstart + width ); x++)
        {
            OLED_Write_Byte(OLED_GRAM[y*128+x], OLED_DATA);
        }
    }
}

/**
 * @name OLED_DrawPoint
 * @brief OLED画一个点
 * @param x:横向坐标 0-127
 * @param y:竖向坐标 0-63
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-3
 * @author 作者：
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawPoint(u8 x,u8 y,bit mode)     //OLED画一个点，0：熄灭 1：点亮
{
    if((x<0)||(x>127)||(y<0)||(y>63))
        return;     //防止超出屏幕
    if( mode )
        OLED_GRAM[(y / 8) * 128 + x] |= 0x01 << (y % 8);
    else
        OLED_GRAM[(y / 8) * 128 + x] &= ~(0x01 << (y % 8));
    // OLED_Refresh_Gram();                    //启用这个后续画线等，将会逐点绘制，但耗时长
}

/**
 * @name OLED_FourPoints
 * @brief 4个像素点为一组，控制一组中的一个灯亮灭
 * @param x0:起始位置0-1
 * @param y0:起始位置0-1
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_FourPoints(u8 x0,u8 y0,bit mode)          //4个像素点为一组，控制亮灭
{
    u8 xdata x, y;
    for ( y = y0; y < 64; y+=2)
    {
        for ( x = x0; x < 128; x+=2)
        {
            if(mode)
                OLED_GRAM[(y / 8) * 128 + x] |= 0x01 << (y % 8);
            else
                OLED_GRAM[(y / 8) * 128 + x] &= ~(0x01 << (y % 8));
        }
    }
}

/**
 * @name OLED_Mask_Gray
 * @brief 灰阶蒙版显示一个图片，衍生于OLED_FourPoints函数
 * @param x0:起始位置
 * @param y0:起始位置
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024
 * @author 作者：
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_Mask_Gray(u8 x0, u8 y0, u8 BMP[], bit mode)  //蒙版函数，衍生于OLED_FourPoints函数
{
    u8 xdata x, y;
    bit z;
    for ( y = y0; y < 64; y+=2)
    {
        for ( x = x0; x < 128; x+=3)    //6阶灰度
        {
            z = BMP[128 * (y / 8) + x] & (0x01 << (y % 8)); // 将蒙版上的像素点坐标，对应BMP数组里相同位置的那个点的数值，赋值给变量z
            if(z==1)
            {
                if(mode)
                OLED_DrawPoint(x, y, 1);
                else
                OLED_DrawPoint(x, y, 0);
            }
        }
    }
}


// /**
//  * @name OLED_DrawLineG
//  * @brief OLED画一条线
//  * @param x1y1：起点坐标
//  * @param x2y2：终点坐标
//  * @param mode:0：熄灭 1：点亮
//  * @return 无
//  * @version 版本：v1.0
//  * @date 日期：2024-1-4
//  * @author 作者：汐
//  * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
//  */
// void OLED_DrawLineG( u8 x1, u8 y1, u8 x2, u8 y2 , bit mode)      //OLED画一条线，0：熄灭 1：点亮
// {
//     u8 i;
//     //先计算增量Δy和Δx
// 	char DeltaY = 0,DeltaX = 0;
// 	float k = 0,b = 0;          //考虑到斜率有小数的情况，所以b也写成浮点型
// 	if(x1>x2)					//保持Δx为正，方便后面使用
// 	{
// 		i = x2;x2 = x1;x1 = i;
// 		i = y2;y2 = y1;y1 = i;
// 		i = 0;
// 	}
// 	DeltaY = y2 - y1;
// 	DeltaX = x2 - x1;
//     if (DeltaX == 0)            //斜率k不存在时的画法
//     {
//         if(y1 <= y2)
//         {
//             for (y1; y1 <= y2; y1++)
//             {
//                 OLED_DrawPoint(x1, y1, mode);
//             }
//         }
//         else if(y1>y2)
//         {
//             for (y2; y2 <= y1; y2++)
//             {
//                 OLED_DrawPoint(x1, y2, mode);
//             }
//         }
//     }
//     else if(DeltaY == 0)        //斜率K=0时的画法
//     {
//         for (x1; x1 <= x2; x1++)
//         {
//             OLED_DrawPoint(x1, y1, mode);
//         }
//     }
//     else                        //斜率正常存在时的画法
//     {
//         k = ((float)DeltaY) / ((float)DeltaX);      // 计算斜率
//         b = y2 - k * x2;                            // 计算截距
//         if((k>-1) & (k<1))
//         {
// 			for(x1;x1<=x2;x1++)
// 			{
// 				OLED_DrawPoint(x1,(int)(k * x1 + b),mode);
// 			}
// 		}
//         else if((k>=1)|(k<=-1))
// 		{
// 			if(y1<=y2)
// 			{
// 				for(y1;y1<=y2;y1++)
// 				{
// 					OLED_DrawPoint((int)((y1 - b) / k),y1,mode);
// 				}
// 			}
//             else if(y1>y2)
// 			{
// 				for(y2;y2<=y1;y2++)
// 				{
// 					OLED_DrawPoint((int)((y2 - b) / k),y2,mode);
// 				}
// 			}
// 		}
// 	}
// }

// /**
//  * @name OLED_DrawLineG
//  * @brief OLED画一条线
//  * @param x1y1：起点坐标
//  * @param x2y2：终点坐标
//  * @param mode:0：熄灭 1：点亮
//  * @return 无
//  * @version 版本：v1.0
//  * @date 日期：2024-1-4
//  * @author 作者：汐
//  * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
//  */
// void OLED_DrawLineG( u8 x1, u8 y1, u8 x2, u8 y2 , bit mode)      //OLED画一条线，0：熄灭 1：点亮
// {
//     u8 x,y;
//     if( x1>x2 )             //如果起始坐标大于终点坐标
//     {
//         x=x1;x1=x2;x2=x;    //将起始坐标的值与终点坐标的值互换
//         y=y1;y1=y2;y2=y;
//     }
//     if(x1!=x2)              //如果是画一条斜线
//     {
//         for( x = x1; x <= x2; x++ )
//         {
//             if( y2>y1 )
//                 OLED_DrawPoint(x, (u8)(y1 + (u16)(y2 - y1) * (u16)x / (u16)(x2 - x1)), mode);
//             else
//                 OLED_DrawPoint(x, (u8)(y1 - (u16)(y1 - y2) * (u16)x / (u16)(x2 - x1)), mode);
//         }        
//     }
//     else
//     {
//         if( y1>y2 )
//         {
//             for( y = y2; y <= y1; y++ )
//                 OLED_DrawPoint(x1, y, mode);
//         }
//         else
//         {
//             for( y = y1; y <= y2; y++ )
//                 OLED_DrawPoint(x1, y, mode);
//         }
//     }
// }

/**
 * @name OLED_DrawLineG_Dot
 * @brief OLED画一条虚实线(微分法)
 * @param x1y1：起点坐标
 * @param x2y2：终点坐标
 * @param mode:0：熄灭 1：点亮
 * @param dot:虚实密度,1：实线
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawLineG_Dot( u8 x1, u8 y1, u8 x2, u8 y2 , bit mode , u8 dot)      //OLED画一条虚实线，0：熄灭 1：点亮
{
    //先计算增量Δy和Δx
    u8 DeltaX = abs(x2 - x1);
    u8 DeltaY = abs(y2 - y1);
    u8 dm, i;           //定义分割基准，循环变量
    float dx, dy;       //定义变化量
    float x, y;         //定义需要显示的坐标值

    if(DeltaX >= DeltaY)  //选择较长的边为基准
        dm = DeltaX;
    else
        dm = DeltaY;

    dx = (float)(x2 - x1) / dm;
    dy = (float)(y2 - y1) / dm;
    x = (float)x1 + 0.5;    //四舍五入
    y = (float)y1 + 0.5;    //四舍五入
    for ( i = 0; i <= dm; i++)
    {
        if(!(i%dot) || i==1)
        OLED_DrawPoint(x, y, mode);
        x += dx;
        y += dy;
    }
}

// /**
//  * @name OLED_DrawRectangleG
//  * @brief OLED画一个矩形
//  * @param x1y1：起点坐标
//  * @param x2y2：终点坐标
//  * @param mode:0：熄灭 1：点亮
//  * @return 无
//  * @version 版本：v1.0
//  * @date 日期：2024-1-4
//  * @author 作者：汐
//  * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
//  */
// void OLED_DrawRectangleG(u8 x1, u8 y1, u8 x2, u8 y2 , bit mode)    //OLED画一个矩形
// {
//     OLED_DrawLineG(x1, y1, x2, y1, mode);
//     OLED_DrawLineG(x2, y1, x2, y2, mode);
//     OLED_DrawLineG(x2, y2, x1, y2, mode);
//     OLED_DrawLineG(x1, y2, x1, y1, mode);
// }

/**
 * @name OLED_DrawFrameG
 * @brief OLED画一个方框
 * @param x0y0：起点坐标
 * @param x1y1：终点坐标
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawFrameG(u8 x0, u8 y0, u8 x1, u8 y1, bit mode)  //OLED画一个方框
{
    OLED_DrawLineG_Dot(x0, y0, x1, y0, mode, 1);
    OLED_DrawLineG_Dot(x1, y0, x1, y1, mode, 1);
    OLED_DrawLineG_Dot(x1, y1, x0, y1, mode, 1);
    OLED_DrawLineG_Dot(x0, y1, x0, y0, mode, 1);
}

/**
 * @name OLED_DrawBlockG
 * @brief OLED画一个实心框
 * @param x0y0：起点坐标
 * @param x1y1：终点坐标
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawBlockG(u8 x0, u8 y0, u8 x1, u8 y1, bit mode)  //OLED画一个实心框
{
    char i, d;

    if(y1-y0 >= 0)
        d = 1;
    else
        d = -1;
    
    for ( i = 0; i <= abs(y1-y0); i++)
    {
        OLED_DrawLineG_Dot(x0, (u8)(y0 + i * d), x1, (u8)(y0 + i * d), mode, 1);
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawBox
 * @brief 画空实心矩形
 * @param x0y0：起点坐标
 * @param x1y1：终点坐标
 * @param fill:0：空心 1：实心
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawBox(u8 x0, u8 y0, u8 x1, u8 y1, u8 fill, bit mode)        //画空实心矩形
{
    int xstart, ystart, xend, yend, i;

    if(x0 < x1) { xstart = x0; xend = x1;}
    else        { xstart = x1; xend = x0;}
    if(y0 < y1) { ystart = y0; yend = y1;}
    else        { ystart = y1; yend = y0;}

    if(fill == 0)  //空心
    {
        OLED_DrawLineG_Dot((u8)xstart, (u8)ystart, (u8)xend, (u8)ystart, mode, 1);
        OLED_DrawLineG_Dot((u8)xstart, (u8)yend, (u8)xend, (u8)yend, mode, 1);
        OLED_DrawLineG_Dot((u8)xstart, (u8)ystart, (u8)xstart, (u8)yend, mode, 1);
        OLED_DrawLineG_Dot((u8)xend, (u8)ystart, (u8)xend, (u8)yend, mode, 1);
    }
    else if (fill == 1) //实心
    {
        for ( i = ystart; i <= yend; i++)
        {
            OLED_DrawLineG_Dot((u8)xstart, (u8)i, (u8)xend, (u8)i, mode, 1);
        }
    }
}

/**
 * @name OLED_DrawCircleG
 * @brief OLED画一个圆形
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawCircleG(u8 x0, u8 y0, u8 R, bit mode)      //OLED画一个圆形
{
    float Rx, Ry, angle, Rad;
    // OLED_DrawPoint(x0, y0, 1);

    for ( angle = 0; angle < 360; angle+=1)
    {
        Rad = angle * 3.14 / 180;

        Rx = R * cos(Rad);
        Ry = R * sin(Rad);

        OLED_DrawPoint(x0 + Rx, /*64 -*/ (y0 + Ry), mode);
        // OLED_Refresh_Gram();
    }
    OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawEllipticG
 * @brief OLED画一个椭圆形
 * @param xy：起点坐标
 * @param R1：长轴
 * @param R2：短轴
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawEllipticG(u8 x0, u8 y0, u8 R1, u8 R2, bit mode)      //OLED画一个椭圆形
{
    float Rx, Ry, angle, Rad;
    // OLED_DrawPoint(x0, y0, 1);

    for ( angle = 0; angle < 360; angle+=1)
    {
        Rad = angle * 3.14 / 180;

        Rx = R1 * cos(Rad);
        Ry = R2 * sin(Rad);

        OLED_DrawPoint(x0 + Rx, /*64 -*/ (y0 + Ry), mode);
        // OLED_Refresh_Gram();
    }
    OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawCircleG_Distance
 * @brief OLED画一个圆形（勾股定理法）
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawCircleG_Distance(u8 x0,u8 y0, u8 R, bit mode) //OLED画一个圆形（勾股定理法）
{
    int x = 0;
    int y = R;

    while (x<=y)
    {
        OLED_DrawPoint((u8)(x0 + x), (u8)(y0 + y), mode);   //右下1
        OLED_DrawPoint((u8)(x0 - x), (u8)(y0 + y), mode);   //左下1
        OLED_DrawPoint((u8)(x0 - x), (u8)(y0 - y), mode);   //左上1
        OLED_DrawPoint((u8)(x0 + x), (u8)(y0 - y), mode);   //右上1
        OLED_DrawPoint((u8)(x0 + y), (u8)(y0 + x), mode);   //右下2
        OLED_DrawPoint((u8)(x0 - y), (u8)(y0 + x), mode);   //左下2
        OLED_DrawPoint((u8)(x0 - y), (u8)(y0 - x), mode);   //左上2
        OLED_DrawPoint((u8)(x0 + y), (u8)(y0 - x), mode);   //右上2

        x++;
        if( (x*x+y*y)>(R*R))
            y--;
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawBlockCircleG_Distance
 * @brief OLED画一个实心圆形（勾股定理法）
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawBlockCircleG_Distance(u8 x0,u8 y0, u8 R, bit mode) //OLED画一个实心圆形（勾股定理法）
{
    int x = 0;
    int y = R;

    while (x<=y)
    {
        // OLED_DrawPoint((u8)(x0 + x), (u8)(y0 + y), mode);
        // OLED_DrawPoint((u8)(x0 - x), (u8)(y0 + y), mode);
        OLED_DrawLineG_Dot((u8)(x0 + x), (u8)(y0 + y), (u8)(x0 - x), (u8)(y0 + y), mode, 1);
        // OLED_DrawPoint((u8)(x0 - x), (u8)(y0 - y), mode);
        // OLED_DrawPoint((u8)(x0 + x), (u8)(y0 - y), mode);
        OLED_DrawLineG_Dot((u8)(x0 - x), (u8)(y0 - y), (u8)(x0 + x), (u8)(y0 - y), mode, 1);
        // OLED_DrawPoint((u8)(x0 + y), (u8)(y0 + x), mode);
        // OLED_DrawPoint((u8)(x0 - y), (u8)(y0 + x), mode);
        OLED_DrawLineG_Dot((u8)(x0 + y), (u8)(y0 + x), (u8)(x0 - y), (u8)(y0 + x), mode, 1);
        // OLED_DrawPoint((u8)(x0 - y), (u8)(y0 - x), mode);
        // OLED_DrawPoint((u8)(x0 + y), (u8)(y0 - x), mode);
        OLED_DrawLineG_Dot((u8)(x0 - y), (u8)(y0 - x), (u8)(x0 + y), (u8)(y0 - x), mode, 1);
        x++;
        if( (x*x+y*y)>(R*R))
            y--;
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawCircleG_Bresenham
 * @brief OLED画一个圆形(Bresenham法)
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawCircleG_Bresenham(u8 x0, u8 y0, u8 R, bit mode)      //OLED画一个圆形(Bresenham法)
{
    int x, y;
    int di;
    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    while (x <= y)
    {
        OLED_DrawPoint((u8)(x0 + x), (u8)(y0 + y), mode);   //右下1
        OLED_DrawPoint((u8)(x0 - x), (u8)(y0 + y), mode);   //左下1
        OLED_DrawPoint((u8)(x0 - x), (u8)(y0 - y), mode);   //左上1
        OLED_DrawPoint((u8)(x0 + x), (u8)(y0 - y), mode);   //右上1
        OLED_DrawPoint((u8)(x0 + y), (u8)(y0 + x), mode);   //右下2
        OLED_DrawPoint((u8)(x0 - y), (u8)(y0 + x), mode);   //左下2
        OLED_DrawPoint((u8)(x0 - y), (u8)(y0 - x), mode);   //左上2
        OLED_DrawPoint((u8)(x0 + y), (u8)(y0 - x), mode);   //右上2

        x++;
        //使用Bresenham算法画圆
        if (di < 0)
        di += 4 * x + 6;
        else
        {
            di += 10 + 4 * (x - y);
            y--;
        }
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawBlockCircleG_Bresenham
 * @brief OLED画一个实心圆形(Bresenham法)
 * @param xy：起点坐标
 * @param r：半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawBlockCircleG_Bresenham(u8 x0, u8 y0, u8 R, bit mode)      //OLED画一个实心圆形(Bresenham法)
{
    int x, y;
    int di;
    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    while (x <= y)
    {
        // OLED_DrawPoint((u8)(x0 + x), (u8)(y0 + y), mode);        //6
        // OLED_DrawPoint((u8)(x0 - x), (u8)(y0 + y), mode);        //1
        OLED_DrawLineG_Dot((u8)(x0 + x), (u8)(y0 + y), (u8)(x0 - x), (u8)(y0 + y), mode, 1);
        // OLED_DrawPoint((u8)(x0 - x), (u8)(y0 - y), mode);        //2
        // OLED_DrawPoint((u8)(x0 + x), (u8)(y0 - y), mode);        //5
        OLED_DrawLineG_Dot((u8)(x0 - x), (u8)(y0 - y), (u8)(x0 + x), (u8)(y0 - y), mode, 1);
        // OLED_DrawPoint((u8)(x0 + y), (u8)(y0 + x), mode);        //4
        // OLED_DrawPoint((u8)(x0 - y), (u8)(y0 + x), mode);        //3
        OLED_DrawLineG_Dot((u8)(x0 + y), (u8)(y0 + x), (u8)(x0 - y), (u8)(y0 + x), mode, 1);
        // OLED_DrawPoint((u8)(x0 - y), (u8)(y0 - x), mode);        //7
        // OLED_DrawPoint((u8)(x0 + y), (u8)(y0 - x), mode);        //0
        OLED_DrawLineG_Dot((u8)(x0 - y), (u8)(y0 - x), (u8)(x0 + y), (u8)(y0 - x), mode, 1);

        x++;
        //使用Bresenham算法画圆
        if (di < 0)
        di += 4 * x + 6;
        else
        {
            di += 10 + 4 * (x - y);
            y--;
        }
        OLED_Refresh_Gram();
    }
    // OLED_Refresh_Gram();
}



/**
 * @name OLED_DrawRoundRectangleG
 * @brief OLED画一个圆角矩形(Bresenham法)
 * @param xy：矩形起点坐标
 * @param x1y1:矩形终点坐标
 * @param R：圆角半径
 * @param fill:0：空心 1：实心
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawRoundRectangleG(u8 x0, u8 y0, u8 x1, u8 y1, u8 R, u8 fill, bit mode)      //OLED画一个圆角矩形(Bresenham法)
{
    int x, y, di, i;
    int xstart, ystart, xend, yend;

    if(x0 < x1) { xstart = x0; xend = x1;}
    else        { xstart = x1; xend = x0;}
    if(y0 < y1) { ystart = y0; yend = y1;}
    else        { ystart = y1; yend = y0;}

    if (R > (u8)(xend - xstart) / 2 || R > (u8)(yend - ystart) / 2)
        R = ((u8)(xend - xstart) < (u8)(yend - ystart)) ? ((u8)(xend - xstart) / 2) : ((u8)(yend - ystart) / 2);

    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    
    if( fill == 0)
    {   //空心
        while (x <= y)
        {
            OLED_DrawPoint((u8)(xstart + R - x), (u8)(ystart + R - y), mode);   //左上1
            OLED_DrawPoint((u8)(xstart + R - y), (u8)(ystart + R - x), mode);   //左上2
            OLED_DrawPoint((u8)(xend - R + x), (u8)(ystart + R - y), mode);   //右上1
            OLED_DrawPoint((u8)(xend - R + y), (u8)(ystart + R - x), mode);   //右上2
            OLED_DrawPoint((u8)(xstart + R - x), (u8)(yend - R + y), mode);   //左下1
            OLED_DrawPoint((u8)(xstart + R - y), (u8)(yend - R + x), mode);   //左下2
            OLED_DrawPoint((u8)(xend - R + x), (u8)(yend - R + y), mode);   //右下1
            OLED_DrawPoint((u8)(xend - R + y), (u8)(yend - R + x), mode);   //右下2

            
            //使用Bresenham算法画圆
            if (di < 0)
            di += 4 * x + 6;
            else
            {
                di += 10 + 4 * (x - y);
                y--;
            }
            x++;
            // OLED_Refresh_Gram();
        }

        OLED_DrawLineG_Dot((u8)(xstart + R + 1), (u8)ystart, (u8)(xend - R - 1), (u8)ystart, mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R + 1), (u8)yend, (u8)(xend - R - 1), (u8)yend, mode, 1);
        OLED_DrawLineG_Dot((u8)xstart, (u8)(ystart + R + 1), (u8)xstart, (u8)(yend - R - 1), mode, 1);
        OLED_DrawLineG_Dot((u8)xend, (u8)(ystart + R + 1), (u8)xend, (u8)(yend - R - 1), mode, 1);
    }
    else if( fill == 1)
    {   //实心
        while (x <= y)
        {
            OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(ystart + R - y), (u8)(xend - R + x), (u8)(ystart + R - y), mode, 1);
            OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(ystart + R - x), (u8)(xend - R + y), (u8)(ystart + R - x), mode, 1);
            OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(yend - R + y), (u8)(xend - R + x), (u8)(yend - R + y), mode, 1);
            OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(yend - R + x), (u8)(xend - R + y), (u8)(yend - R + x), mode, 1);
            
            //使用Bresenham算法画圆
            if (di < 0)
            di += 4 * x + 6;
            else
            {
                di += 10 + 4 * (x - y);
                y--;
            }
            x++;
            // OLED_Refresh_Gram();
        }

        for ( i = ystart + R + 1; i <= yend - R -1; i++)
        {
            OLED_DrawLineG_Dot((u8)xstart, (u8)i, (u8)xend, (u8)i, mode, 1);
        }
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawWideRoundRectangleG
 * @brief OLED画一个宽边圆角矩形(Bresenham法)
 * @param xy：矩形起点坐标
 * @param x1y1:矩形终点坐标
 * @param R：圆角半径
 * @param mode:0：熄灭 1：点亮
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_DrawWidthRoundRectangleG(u8 x0, u8 y0, u8 x1, u8 y1, u8 R, u8 width, bit mode)      //OLED画一个宽边圆角矩形(Bresenham法)
{
    int x, y, di;
    int xstart, ystart, xend, yend;

    if (width <= 1)
    {
        OLED_DrawRoundRectangleG(x0, y0, x1, y1, R, 0, 1);
        return;
    }
    

    //画大扇形
    if(x0 < x1) { xstart=x0; xend = x1;}
    else        { xstart=x1; xend = x0;}
    if(y0 < y1) { ystart=y0; yend = y1;}
    else        { ystart=y1; yend = y0;}

    if (R > (u8)(xend - xstart) / 2 || R > (u8)(yend - ystart) / 2)
        R = ((u8)(xend - xstart) < (u8)(yend - ystart)) ? ((u8)(xend - xstart) / 2) : ((u8)(yend - ystart) / 2);

    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    

    while (x <= y)
    {
        OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(ystart + R - y), (u8)(xstart + R - x), (u8)(ystart + R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(ystart + R - x), (u8)(xstart + R - y), (u8)(ystart + R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + x), (u8)(ystart + R - y), (u8)(xend - R + x), (u8)(ystart + R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + y), (u8)(ystart + R - x), (u8)(xend - R + y), (u8)(ystart + R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(yend - R + y), (u8)(xstart + R - x), (u8)(yend - R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(yend - R + x), (u8)(xstart + R - y), (u8)(yend - R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + x), (u8)(yend - R + y), (u8)(xend - R + x), (u8)(yend - R), mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + y), (u8)(yend - R + x), (u8)(xend - R + y), (u8)(yend - R), mode, 1);
        
        //使用Bresenham算法画圆
        if (di < 0)
        di += 4 * x + 6;
        else
        {
            di += 10 + 4 * (x - y);
            y--;
        }
        x++;
    }

    OLED_DrawBox((u8)(xstart + R + 1), (u8)(ystart), (u8)(xend - R - 1), (u8)(ystart + width - 1), 1, mode);
    OLED_DrawBox((u8)(xstart + R + 1), (u8)(yend - width + 1), (u8)(xend - R - 1), (u8)(yend), 1, mode);
    OLED_DrawBox((u8)(xstart), (u8)(ystart + R + 1), (u8)(xstart + width - 1), (u8)(yend - R - 1), 1, mode);
    OLED_DrawBox((u8)(xend - width + 1), (u8)(ystart + R + 1), (u8)(xend), (u8)(yend - R - 1), 1, mode);

    //画擦除小扇形
    if(x0 < x1) { xstart = x0 + width; xend = x1 - width;}
    else        { xstart = x1 + width; xend = x0 - width;}
    if(y0 < y1) { ystart = y0 + width; yend = y1 - width;}
    else        { ystart = y1 + width; yend = y0 - width;}
    R -= width;
    if (R > (u8)(xend - xstart) / 2 || R > (u8)(yend - ystart) / 2)
        R = ((u8)(xend - xstart) < (u8)(yend - ystart)) ? ((u8)(xend - xstart) / 2) : ((u8)(yend - ystart) / 2);

    x = 0;
    y = R;
    di = 3 - (R << 1);       //判断下个点位置的标志
    

    while (x <= y)
    {
        OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(ystart + R - y), (u8)(xstart + R - x), (u8)(ystart + R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(ystart + R - x), (u8)(xstart + R - y), (u8)(ystart + R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + x), (u8)(ystart + R - y), (u8)(xend - R + x), (u8)(ystart + R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + y), (u8)(ystart + R - x), (u8)(xend - R + y), (u8)(ystart + R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - x), (u8)(yend - R + y), (u8)(xstart + R - x), (u8)(yend - R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xstart + R - y), (u8)(yend - R + x), (u8)(xstart + R - y), (u8)(yend - R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + x), (u8)(yend - R + y), (u8)(xend - R + x), (u8)(yend - R), ~mode, 1);
        OLED_DrawLineG_Dot((u8)(xend - R + y), (u8)(yend - R + x), (u8)(xend - R + y), (u8)(yend - R), ~mode, 1);
        
        //使用Bresenham算法画圆
        if (di < 0)
        di += 4 * x + 6;
        else
        {
            di += 10 + 4 * (x - y);
            y--;
        }
        x++;
    }
    // OLED_Refresh_Gram();
}

/**
 * @name OLED_DrawSin
 * @brief 画正弦函数
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024
 * @author 作者：
 * @note 注释：
 */
void OLED_DrawSin()     //画正弦函数
{
    u8 a = 8;   //X轴系数
    u8 b = 20;  //Y轴系数
    u8 x0 = 40, y0 = 32;    //起始点，结合后文，最好控制在中心点
    //x0 = 63 - 180/a

    float Si, Sx, Sy, Rad;

    for ( Si = 0; Si <= 360; Si+= 1)
    {
        Rad = Si * 3.14 / 180;
        Sx = Si / /*系数*/ a;               //最左端：x0+0*a，最右端：x0+360/a
        Sy = sin(Rad) * /*系数*/ b;     //最高点：y0-1*b，最低点：y0+1*b
        OLED_DrawPoint(Sx + x0, 64 - (Sy + y0), 1);
        OLED_RefreshPart_Gram(30, 1, 60, 6);

        OLED_ShowNum(35, 7, Si, 3, 6, 0);
    }
}

/**
 * @name ButtonWave
 * @brief 按键控制显示方波
 * @param 无
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：
 * @note 注释：
 */
void ButtonWave()   //按键控制显示方波
{
    u8 Xtime;

    char Range01 = 50;
    char Range02 = 10;
    for ( Xtime = 127; Xtime > 0; Xtime--)
        WaveData[Xtime] = WaveData[Xtime - 1];

    if(P54)
        WaveData[0] = Range02;
    else
        WaveData[0] = Range01;

    OLED_GRAM_Clear(0);

    for ( Xtime = 0; Xtime < 127; Xtime++)
        OLED_DrawLineG_Dot(Xtime, (u8)(63 - WaveData[Xtime]), (u8)(Xtime + 1), (u8)(63 - WaveData[Xtime + 1]), 1, 1);

    OLED_Refresh_Gram();
    delay_oled_ms(50);
}



/**
 * @name OLED_ShowCharG
 * @brief 在指定位置显示一个字符,包括部分字符
 * @param x:0~127
 * @param y:0~7
 * @param chr:
 * @param charSize:选择字体16(高度占2行)/6(高度占1行)
 * @param Is_Reverse:模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowCharG(u8 x,u8 y,u8 chr, u8 charSize, u8 Is_Reverse)     // 缓存显示一个字符
{
    u8 xdata i = 0;
    chr -= ' ';	//得到偏移后的值
    if (x > Max_Column - 1)     //如果起点横坐标超过了屏幕最右侧
    {
        x = 0;                  //列坐标x置零，回到屏幕最左侧
        y = y + 2;              //行坐标y下移2行
    }                           //将字符显示在屏幕指定行的下一行起点位置
    if (charSize == 16)                 //判断字号大小，读取F16X8字库，占2行
    {   
        for (i = 0; i<8; i++)           //两行高度的上行
            OLED_GRAM[128*y+(x+i)] = (Is_Reverse == 0?F16X8[chr][i]:~F16X8[chr][i]);
        for (i = 0; i<8; i++)           //两行高度的下行
            OLED_GRAM[128*(y+1)+(x+i)] = (Is_Reverse == 0?F16X8[chr][i + 8]:~F16X8[chr][i + 8]);
    }
    else if(charSize == 6)              //判断字号大小，读取F8X6字库，占1行
    {
        for (i = 0; i<6; i++)
            OLED_GRAM[128*y+(x+i)] = (Is_Reverse == 0?F8X6[chr][i]:~F8X6[chr][i]);
    }
}

/**
 * @name OLED_ShowStringG
 * @brief 显示一个字符号串
 * @param xy：起点坐标
 * @param chr：需要显示的字符串
 * @param charSize：字体大小16/6
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowStringG(u8 x, u8 y, char *chr, u8 charSize, u8 Is_Reverse) //显示一个字符号串
{
    u8 xdata j = 0;
    while (chr[j] != '\0')      //读取到字符串结束符之前，一直循环
    {
        OLED_ShowCharG(x, y, chr[j], charSize, Is_Reverse);  //从第一个字符开始显示
        x += 8;         //显示完一个字符后，起点x坐标右移8格，准备显示下一个字符
        if (x>120)      //如果起点x位置大于120，为保证字符显示完整，在下一行最左侧位置显示
        {
            x = 0;
            y += 2;
        }
        j++;            //指针移到下一个字符
    }
}

/**
 * @name OLED_ShowNumG
 * @brief 缓存显示一串数字
 * @param xy：起点坐标
 * @param num：数值(0~4294967295)
 * @param len：数字的位数，即显示几位有效数字,max=10
 * @param Size：字体大小16/6
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowNumG(u8 x, u8 y, u32 num, u8 len, u8 Size, u8 Is_Reverse)  //缓存显示一串数字
{
    u8 xdata t, temp;
    u8 xdata enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / OLED_Index(10, (u8)(len - t - 1))) % 10;      //取第t位数字
        if (enshow == 0 && t < (len - 1))       //如果数串长度小于指定长度
        {
            if (temp == 0)                      //多出来的位用空格补齐
            {
                OLED_ShowCharG((u8)(x + 8 * t), y, ' ', Size, Is_Reverse);
                continue;                       //继续循环判断
            }
            else
                enshow = 1;                     //直到有数字为止
        }
        OLED_ShowCharG((u8)(x + 8 * t), y, (u8)(temp + '0'), Size, Is_Reverse);  //从高到低逐位显示数字
    }
}

/**
 * @name OLED_ShowChineseG
 * @brief 显示汉字
 * @param xy：起点坐标
 * @param no：需要显示的字符串
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2024-1-4
 * @author 作者：汐
 * @note 注释：通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowChineseG(u8 x, u8 y, u8 no, u8 Is_Reverse) //缓存显示汉字
{
    u8 xdata i;
    for (i = 0; i < 16; i++)    //汉字上半部分坐标（第x列，第y行）
    {
        OLED_GRAM[128*y+(x+i)] = (Is_Reverse == 0?Hzk[2 * no][i]:~Hzk[2 * no][i]);
    }
    for (i = 0; i<16; i++)      //汉字下半部分坐标（第x列，第y+1行）
    {
        OLED_GRAM[128*y+(x+i)] = (Is_Reverse == 0?Hzk[2 * no + 1][i]:~Hzk[2 * no + 1][i]);
    }
}

/**
 * @name OLED_ShowBMPG
 * @brief 缓存显示BMP图片
 * @param x0y0：起始点坐标(x,y),x的范围0-127，y为页的范围0-7
 * @param x1y1：图片的像素（与实际取模的图片像素一致）
 * @param BMP[]：需要显示BMP
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-31
 * @author 作者：汐
 * @note 注释：x1-x0=图片的实际像素长度 y1-y0=图片的实际像素宽度
 *           通过缓冲数组绘制，需要配合OLED_Refresh_Gram函数使用
 */
void OLED_ShowBMPG(u8 x0, u8 y0, u8 x1, u8 y1, u8 BMP[], u8 Is_Reverse)  //缓存显示BMP图片
{
    u16 xdata j = 0;
    u8 xdata x, y;
    if (y1 % 8 == 0) 
		y1 = y1 / 8;
    else 
		y1 = y1 / 8 + 1;
    for (y = y0; y<y1; y++)
    {
        for (x = x0; x<x1; x++)
        {
            OLED_GRAM[128*y0 + x] = (Is_Reverse == 0?BMP[j++]:~BMP[j++], OLED_DATA);
        }
    }
}



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#else                                   //2：指令模式

#endif





/**
 * @name OLED_ShowChar
 * @brief 在指定位置显示一个字符,包括部分字符
 * @param x:0~127
 * @param y:0~63
 * @param chr:
 * @param charSize:选择字体16(高度占2行)/6(高度占1行)
 * @param Is_Reverse:模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：
 */
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 charSize, u8 Is_Reverse)  //早指定位置显示一个字符
{
    u8 xdata i = 0;
    chr -= ' ';	//得到偏移后的值
    if (x > Max_Column - 1)     //如果起点横坐标超过了屏幕最右侧
    {
        x = 0;                  //列坐标x置零，回到屏幕最左侧
        y = y + 2;              //行坐标y下移2行
    }                           //将字符显示在屏幕指定行的下一行起点位置
    if (charSize == 16)                 //判断字号大小，读取F16X8字库，占2行
    {
        OLED_Set_Pos(x, y);             //两行高度的上行
        for (i = 0; i<8; i++)
            OLED_Write_Byte(Is_Reverse == 0?F16X8[chr][i]:~F16X8[chr][i], OLED_DATA);
        OLED_Set_Pos(x, (u8)(y + 1));   //两行高度的下行
        for (i = 0; i<8; i++)
            OLED_Write_Byte(Is_Reverse == 0?F16X8[chr][i + 8]:~F16X8[chr][i + 8], OLED_DATA);
    }
    else if(charSize == 6)              //判断字号大小，读取F8X6字库，占1行
    {
        OLED_Set_Pos(x, y);
        for (i = 0; i<6; i++)
            OLED_Write_Byte(Is_Reverse == 0?F8X6[chr][i]:~F8X6[chr][i], OLED_DATA);
    }
}

/**
 * @name OLED_ShowString
 * @brief 显示一个字符号串
 * @param xy：起点坐标
 * @param chr：需要显示的字符串
 * @param charSize：字体大小16/6
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：
 */
void OLED_ShowString(u8 x, u8 y, char *chr, u8 charSize, u8 Is_Reverse) //显示一个字符号串
{
    u8 xdata j = 0;
    while (chr[j] != '\0')      //读取到字符串结束符之前，一直循环
    {
        OLED_ShowChar(x, y, chr[j], charSize, Is_Reverse);  //从第一个字符开始显示
        x += 8;         //显示完一个字符后，起点x坐标右移8格，准备显示下一个字符
        if (x>120)      //如果起点x位置大于120，为保证字符显示完整，在下一行最左侧位置显示
        {
            x = 0;
            y += 2;
        }
        j++;            //指针移到下一个字符
    }
}

/**
 * @name OLED_ShowNum
 * @brief 显示一串数字
 * @param xy：起点坐标
 * @param num：数值(0~4294967295)
 * @param len：数字的位数，即显示几位有效数字,max=10
 * @param Size：字体大小16/6
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-30
 * @author 作者：汐
 * @note 注释：
 */
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 Size, u8 Is_Reverse)  //显示一串数字
{
    u8 xdata t, temp;
    u8 xdata enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / OLED_Index(10, (u8)(len - t - 1))) % 10;      //取第t位数字
        if (enshow == 0 && t < (len - 1))       //如果数串长度小于指定长度
        {
            if (temp == 0)                      //多出来的位用空格补齐
            {
                OLED_ShowChar((u8)(x + 8 * t), y, ' ', Size, Is_Reverse);
                continue;                       //继续循环判断
            }
            else
                enshow = 1;                     //直到有数字为止
        }
        OLED_ShowChar((u8)(x + 8 * t), y, (u8)(temp + '0'), Size, Is_Reverse);  //从高到低逐位显示数字
    }
}

/**
 * @name OLED_ShowChinese
 * @brief 显示汉字
 * @param xy：起点坐标
 * @param no：需要显示的字符串
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-31
 * @author 作者：汐
 * @note 注释：
 */
void OLED_ShowChinese(u8 x, u8 y, u8 no, u8 Is_Reverse) //显示汉字
{
    u8 xdata i;
    OLED_Set_Pos(x, y);             //汉字上半部分坐标（第x列，第y行）
    for (i = 0; i < 16; i++)
    {
        OLED_Write_Byte(Is_Reverse == 0?Hzk[2 * no][i]:~Hzk[2 * no][i], OLED_DATA);
    }
    OLED_Set_Pos(x, (u8)(y + 1));   //汉字下半部分坐标（第x列，第y+1行）
    for (i = 0; i<16; i++)
    {
        OLED_Write_Byte(Is_Reverse == 0?Hzk[2 * no + 1][i]:~Hzk[2 * no + 1][i], OLED_DATA);
    }
}


/**
 * @name OLED_ShowBMP
 * @brief 显示BMP图片
 * @param x0y0：起始点坐标(x,y),x的范围0-127，y为页的范围0-7
 * @param x1y1：图片的像素（与实际取模的图片像素一致）
 * @param BMP[]：需要显示BMP
 * @param Is_Reverse：模式 0：黑底白字 1：白底黑字
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023-12-31
 * @author 作者：汐
 * @note 注释：x1-x0=图片的实际像素长度 y1-y0=图片的实际像素宽度
 */
void OLED_ShowBMP(u8 x0, u8 y0, u8 x1, u8 y1, u8 BMP[], u8 Is_Reverse)  //显示BMP图片
{
    u16 xdata j = 0;
    u8 xdata x, y;
    if (y1 % 8 == 0) 
		y1 = y1 / 8;
    else 
		y1 = y1 / 8 + 1;
    for (y = y0; y<y1; y++)
    {
        OLED_Set_Pos(x0, y);
        for (x = x0; x<x1; x++)
        {
            OLED_Write_Byte(Is_Reverse == 0?BMP[j++]:~BMP[j++], OLED_DATA);
        }
    }
}



