#include "iic.h"
#include <INTRINS.H>




#if IIC_MODE == 0           //0：软件IIC
//------------------------------变量定义------------------------------

u8 ack = 0;



//------------------------------函数定义------------------------------


void IIC_Delay2us(void)	//@22.1184MHz
{
    unsigned long edata i;

    _nop_();
    _nop_();
    _nop_();
    i = 9UL;
    while (i) i--;
}

//---------------------驱动函数---------------------


void IIC_Init(void)            //IIC初始化
{
    #if     IIC_PIN == 0        //4P
    SCL = 1;
    SDA = 1;
    #elif   IIC_PIN == 1        //7P
    SCL = 1;
    SDA = 1;
    RES = 1;
    DC = 0;
    CS = 0;
    #endif
}


void IIC_Start(void)            //IIC启动
{
    SCL = 1;
    SDA = 1;
    IIC_Delay2us();
    SDA = 0;
    IIC_Delay2us();
    SCL = 0;
    IIC_Delay2us();
}

void IIC_Stop(void)             //IIC结束
{
    SCL = 0;
    SDA = 0;
    IIC_Delay2us();
    SCL = 1;
    IIC_Delay2us();
    SDA = 1; 
    IIC_Delay2us();
}


void IIC_SendACK(void)          //发送ACK
{
    SDA = 0;
    IIC_Delay2us();
    SCL = 1;
    IIC_Delay2us();
    SCL = 0;
    IIC_Delay2us();
}

void IIC_SendNoACK(void)        //发送NOACK
{
    SDA = 1;
    IIC_Delay2us();
    SCL = 1;
    IIC_Delay2us();
    SCL = 0;
    IIC_Delay2us();
}


void IIC_WaitACK(void)          //等待从机ACK
{
    SDA = 1;
    IIC_Delay2us();
    SCL = 1;
    IIC_Delay2us();
    ack = SDA;
    IIC_Delay2us();
    SCL = 0;
    IIC_Delay2us();
}

void IIC_SendByte(u8 dat)       //发送一个字节
{
    u8 i = 8;
    do
    {
        if (dat & 0x80)
            SDA = 1;
        else
            SDA = 0;
        IIC_Delay2us();
        dat <<= 1;
        SCL = 1;
        IIC_Delay2us();
        SCL = 0;
        IIC_Delay2us();
    } while (--i);
}

u8 IIC_ReadByte(void)           //读取一个字节
{
    u8 i = 8, dat = 0;
    SDA = 1;
    IIC_Delay2us();
    do
    {
        SCL = 1;
        IIC_Delay2us();
        dat <<= 1;
        if (SDA)
            dat |= 1;
        SCL = 0;
        IIC_Delay2us();
    } while (--i);

    return dat;
}

//---------------------功能函数---------------------

/**
 * @name IIC_Write_Byte
 * @brief IIC写入连续的几个字节
 * @param slave：选择哪一个从机的地址
 * @param addr：数据的地址
 * @param p：数据内容
 * @param number：写入几个数据
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023
 * @author 作者：汐
 * @note 注释：
 */
void IIC_Write_Byte(u8 slave,u8 addr,u8 *p,u8 number)       //IIC写入连续的几个字节
{
    IIC_Start();
    IIC_SendByte(slave);
    IIC_WaitACK();
    if (!ack)
    {
        IIC_SendByte(addr);
        IIC_WaitACK();
        if (!ack)
        {
            do
            {
                IIC_SendByte(*p);
                p++;
                IIC_WaitACK();
                if (ack)
                    break;
            } while (--number);
        }
    }
    IIC_Stop();
}

/**
 * @name IIC_Read_Byte
 * @brief IIC读取连续的几个字节
 * @param slave：选择哪一个从机的地址
 * @param addr：要读的数据的地址
 * @param p：数据
 * @param number：写入几个数据
 * @return 
 * @version 版本：v1.0
 * @date 日期：2023
 * @author 作者：汐
 * @note 注释：发送开始命令->发送器件地址（写）->发送数据地址->发送开始命令->发送器件地址（读）->读数据
 */
void IIC_Read_Byte(u8 slave,u8 addr,u8 *p,u8 number)       //IIC读取连续的几个字节
{
    IIC_Start();
    IIC_SendByte(slave);
    IIC_WaitACK();
    if (!ack)
    {
        IIC_SendByte(addr);
        IIC_WaitACK();
        if (!ack)
        {
            IIC_Start();
            IIC_SendByte((u8)(slave + 0x01));
            IIC_WaitACK();
            if (!ack)
            {
                do
                {
                    *p = IIC_ReadByte();
                    p++;
                    if(number != 1)
                        IIC_SendACK();
                } while (--number);
                IIC_SendNoACK();
            }
            
        }
    }
    IIC_Stop();
}



#elif IIC_MODE == 1             //1：硬件IIC

//------------------------------变量定义------------------------------

//------------------------------函数定义------------------------------

void IIC_Delay2us(void)	//@22.1184MHz
{
    while( !(I2CMSST & 0X40) );
    I2CMSST &= ~(0x40);
}

//---------------------驱动函数---------------------

void IIC_Init(void)            //硬件IIC初始化
{
    I2C_S1 = 0;                 //设置IIC的引脚,00:P15,P14; 01:P25,P24; 11:P32,P33
    I2C_S0 = 1;                 //SCL=P25 SDA=P24
    I2CCFG = 0XE5;              //设置速度和使能IIC
    I2CMSST = 0;                //清空主机状态
}


void IIC_Start(void)            //IIC启动
{
    I2CMSCR = 0X01;             //开始命令
    IIC_Delay2us();             //等待命令
}

void IIC_Stop(void)             //IIC结束
{
    I2CMSCR = 0X06;             //结束命令
    IIC_Delay2us();             //等待命令
}


void IIC_SendACK(void)          //发送ACK
{
    I2CMSST = 0X00;             //准备发送ACK
    I2CMSCR = 0X05;             //发送ACK
    IIC_Delay2us();
}

void IIC_SendNoACK(void)        //发送NOACK
{
    I2CMSST = 0X01;             //准备发送NOACK
    I2CMSCR = 0X05;             //发送NOACK
    IIC_Delay2us();
}


void IIC_WaitACK(void)          //等待从机ACK
{
    I2CMSCR = 0X03;             //接收ACK命令
    IIC_Delay2us();
}

void IIC_SendByte(u8 dat)       //发送一个字节
{
    I2CTXD = dat;               //发送的数据写入寄存器
    I2CMSCR = 0X02;             //发送数据指令
    IIC_Delay2us();
}

u8 IIC_ReadByte(void)           //读取一个字节
{
    I2CMSCR = 0X04;             //接收数据指令
    IIC_Delay2us();

    return I2CRXD;
}

//---------------------功能函数---------------------

/**
 * @name IIC_Write_Byte
 * @brief IIC写入连续的几个字节
 * @param slave：选择哪一个从机的地址
 * @param addr：数据的地址
 * @param p：数据内容
 * @param number：写入几个数据
 * @return 无
 * @version 版本：v1.0
 * @date 日期：2023
 * @author 作者：汐
 * @note 注释：
 */
void IIC_Write_Byte(u8 slave,u8 addr,u8 *p,u8 number)       //IIC写入连续的几个字节
{
    IIC_Start();
    IIC_SendByte(slave);
    IIC_WaitACK();
    
    IIC_SendByte(addr);
    IIC_WaitACK();

    do
    {
        IIC_SendByte(*p);
        p++;
        IIC_WaitACK();
    } while (--number);
    
    IIC_Stop();
}

/**
 * @name IIC_Read_Byte
 * @brief IIC读取连续的几个字节
 * @param slave：选择哪一个从机的地址
 * @param addr：要读的数据的地址
 * @param p：数据
 * @param number：写入几个数据
 * @return 
 * @version 版本：v1.0
 * @date 日期：2023
 * @author 作者：汐
 * @note 注释：发送开始命令->发送器件地址（写）->发送数据地址->发送开始命令->发送器件地址（读）->读数据
 */
void IIC_Read_Byte(u8 slave,u8 addr,u8 *p,u8 number)       //IIC读取连续的几个字节
{
    IIC_Start();
    IIC_SendByte(slave);
    IIC_WaitACK();

    IIC_SendByte(addr);
    IIC_WaitACK();

    IIC_Start();
    IIC_SendByte((u8)(slave + 0x01));
    IIC_WaitACK();

    do
    {
        *p = IIC_ReadByte();
        p++;
        if(number != 1)
            IIC_SendACK();
    } while (--number);
    IIC_SendNoACK();
    IIC_Stop();
}

#endif
