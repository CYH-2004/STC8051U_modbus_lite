#include "SysConfig.h"

#include "Timer.h"
#include "Uart1.h"
#include "Uart2.h"
#include "Uart3.h"
#include "Uart4.h"
#include "Key.h"

#include "iic.h"
#include "oled12824_iic.h"

#include "modbus.h"


void MCU_Init(void)
{
    P_SW2|=0x80;     //使能访问XFR，没有冲突不用关闭
	CKCON=0x00;     //设置外部数据总线速度为最快
	WTST=0x00;      //设置程序代码等待参数
                    //赋值为0可将CPU执行程序的速度设置为最快

	//IO口初始设置
    P0M0 = 0x00;    P0M1 = 0x00;
    P1M0 = 0x00;    P1M1 = 0x00;
    P2M0 = 0xff;    P2M1 = 0x00; 
    P3M0 = 0x00;    P3M1 = 0x00;
    P4M0 = 0x00;    P4M1 = 0x00;
    P5M0 = 0x00;    P5M1 = 0x00;

    EA=1;

    timer4_init();      //Systick初始化

    UART1_config(0);    //UART1初始化
    // UART2_config(2);    //UART2初始化
    
    //其他控制初始化
    LED_ctrl_1 = 0;
    LED_ctrl_2 = 0;
    
    Delay1000ms();
    // PrintString1("STC8051U UART initialization succeed !\r\n");
    LED_ctrl_2 = 1;
}

void modbus_connect_lost(void)
{
    OLED_GRAM_Clear(0);
    if(modbus_mode())   //主机模式
    {
        OLED_ShowStringG(0, 0, "Master mode", 16, 1);
    }
    else  //从机模式
    {
        OLED_ShowStringG(0, 0, "Slave mode", 16, 1);
    }
    OLED_ShowStringG(0, 3, "Reconnecting...", 16, 0);
    OLED_Refresh_Gram();
}

void main()
{
    uint8_t light_status;
    uint8_t light_status_temp;
    uint16_t counter;
    // uint16_t counter_temp;

    uint16_t slave_address;
    uint16_t slave_light_coil_addr;
    uint16_t slave_counter_reg_addr;

    volatile uint8_t slave_status;

    MCU_Init();


    OLED_Init();
    OLED_GRAM_Clear(0);
    OLED_Refresh_Gram();
    counter = 0;
    light_status = 0;
    

    //设置从站地址
    slave_address = 0x10;

    //设置从站数据存储地址，默认为可用的首地址
    slave_light_coil_addr = COILS_START_ADDRESS;
    slave_counter_reg_addr = HOLDING_REG_START_ADDRESS;

    if(key_scan2(200))  //从机模式
    {
        LED_ctrl_2 = 0;
        PrintString1("UART1: Enter modbus slave mode!\r\n");
        // PrintString2("UART2: Enter modbus slave mode!\r\n");
        modbus_RTU_init(115200, 0, slave_address);

        OLED_GRAM_Clear(0);
        OLED_ShowStringG(0, 0, "Slave", 16, 0);
        OLED_ShowStringG(0, 3, "Initializing...", 16, 0);
        OLED_Refresh_Gram();
        
        light_status_temp = light_status;
        modbus_write_bit_status(slave_light_coil_addr, &light_status);
        modbus_write_reg_value(slave_counter_reg_addr, &counter);

        OLED_GRAM_Clear(0);
        OLED_ShowStringG(0, 0, "Slave", 16, 1);
        if(light_status)
        {
            // LED_ctrl_1 = 1;
            OLED_ShowStringG(0, 4, "Light On", 16, 0);
            OLED_DrawRoundRectangleG(85, 20, 124, 59, 8, 1, 1);
            OLED_Refresh_Gram();
        }
        else
        {
            // LED_ctrl_1 = 0;
            OLED_ShowStringG(0, 4, "Light Off", 16, 0);
            OLED_DrawRoundRectangleG(85, 20, 124, 59, 8, 0, 1);
            OLED_Refresh_Gram();
        }
        OLED_ShowStringG(0, 6, "Cnt:", 16, 0);
        OLED_ShowNumG(45, 6, counter, 1, 16, 0);
        OLED_Refresh_Gram();

        while(1)
        {
            modbus_RTU_slave();

            modbus_get_bit_status(slave_light_coil_addr, &light_status_temp);   //读取灯状态寄存器
            if(light_status_temp != light_status)
            {
                OLED_GRAM_Clear(0);
                light_status = light_status_temp;
                if(light_status)
                {
                    modbus_get_reg_value(slave_counter_reg_addr, &counter);     //读取计数寄存器
                    counter++;
                    modbus_write_reg_value(slave_counter_reg_addr, &counter);   //写入计数寄存器
                    
                    // LED_ctrl_1 = 1;
                    OLED_ShowStringG(0, 4, "Light On", 16, 0);
                    OLED_DrawRoundRectangleG(85, 20, 124, 59, 8, 1, 1);
                }
                else
                {
                    // LED_ctrl_1 = 0;
                    OLED_GRAM_Clear(0);
                    OLED_ShowStringG(0, 4, "Light Off", 16, 0);
                    OLED_DrawRoundRectangleG(85, 20, 124, 59, 8, 0, 1);
                }
            }
            modbus_get_reg_value(slave_counter_reg_addr, &counter);
            OLED_ShowStringG(0, 6, "Cnt:", 16, 0);
            OLED_ShowNumG(45, 6, counter, 1, 16, 0);

            OLED_ShowStringG(0, 0, "Slave", 16, 1);
            OLED_ShowStringG(0, 2, "Addr:", 16, 0);
            OLED_ShowNumG(45, 2, slave_address, 3, 16, 0);
            OLED_Refresh_Gram();
            // if(key_scan2(10))
            // {
            //     counter = 0;
            // }
        }
    }
    else    //主机模式
    {
        PrintString1("UART1: Enter modbus master mode!\r\n");
        // PrintString2("UART2: Enter modbus master mode!\r\n");
        modbus_RTU_init(115200, 1, slave_address);

        OLED_GRAM_Clear(0);
        OLED_ShowStringG(0, 0, "Master", 16, 0);
        OLED_ShowStringG(0, 3, "Connecting...", 16, 0);
        OLED_Refresh_Gram();

        //熄灯并清除从机计数器
        // while(modbus_RTU_write_single_coil(slave_address, slave_light_coil_addr, 0, 300)) n_ms(10);
        // while(modbus_RTU_write_single_reg(slave_address, slave_counter_reg_addr, 0, 300)) n_ms(10);
        slave_status = 1;
        while(slave_status) slave_status = modbus_RTU_write_single_coil(slave_address, slave_light_coil_addr, 0, 300);
        slave_status = 1;
        while(slave_status) slave_status = modbus_RTU_write_single_reg(slave_address, slave_counter_reg_addr, 0, 300);

        while(1)
        {
            
            while(modbus_RTU_read_coils(slave_address, slave_light_coil_addr, 1, &light_status, 300)) modbus_connect_lost(); // 读取从机灯状态
            while(modbus_RTU_read_holding_regs(slave_address, slave_counter_reg_addr, 1, &counter, 300)) modbus_connect_lost(); // 读取从机计数值

            //刷新显示
            OLED_GRAM_Clear(0);
            OLED_ShowStringG(0, 0, "Master", 16, 1);
            OLED_ShowStringG(0, 2, "Dest:", 16, 0);
            OLED_ShowNumG(45, 2, slave_address, 3, 16, 0);
            if(light_status) 
            {
                OLED_ShowStringG(0, 4, "Light On", 16, 0);
                OLED_DrawRoundRectangleG(85, 20, 124, 59, 8, 1, 1);
            }
            else 
            {
                OLED_ShowStringG(0, 4, "Light Off", 16, 0);
                OLED_DrawRoundRectangleG(85, 20, 124, 59, 8, 0, 1);
            }
            OLED_ShowStringG(0, 6, "Cnt:", 16, 0);
            OLED_ShowNumG(42, 6, counter, 3, 16, 0);            
            OLED_Refresh_Gram();

            //检测按键
            if(key_scan2(90)) 
            {
                while(modbus_RTU_read_coils(slave_address, slave_light_coil_addr, 1, &light_status, 300)) modbus_connect_lost();
                light_status = !light_status;
                while(modbus_RTU_write_single_coil(slave_address, slave_light_coil_addr, light_status, 300)) modbus_connect_lost();
                while(modbus_RTU_read_holding_regs(slave_address, slave_counter_reg_addr, 1, &counter, 300)) modbus_connect_lost();
            }
        }
    }
}

