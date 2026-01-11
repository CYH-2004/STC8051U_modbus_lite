#include "modbus.h"

#include "Timer.h"
#include "Uart4.h"      //modbus RTU使用UART4

#include "Uart1.h"      //调试串口
#include "Uart2.h"      //调试串口

uint8_t node_address;
uint8_t node_mode;

//modbus_RTU 本地存储器
volatile uint8_t xdata discrete_input_register[(DISCRETES_INPUT_MAX + 7) / 8];
volatile uint8_t xdata coil_register[(COILS_MAX + 7) / 8];
volatile uint16_t xdata input_register[INPUT_REG_MAX];
volatile uint16_t xdata holding_register[HOLDING_REG_MAX];

volatile uint8_t xdata RTU_TX_buffer[256];   //发送缓冲区

//modbus_RTU 初始化
uint8_t modbus_RTU_init(uint16_t brt, uint8_t mode, uint8_t addr);
uint8_t modbus_mode(void);

//辅助程序
uint16_t MODBUS_CRC16(uint8_t xdata *p, uint8_t n);
void set_bit(uint8_t *data1, uint8_t pos1, uint8_t *data2, uint8_t pos2);
uint8_t modbus_response_WR_check(uint8_t xdata *RX_mesg, uint8_t xdata *TX_mesg);

//modbus_RTU 本地存储器访问
uint8_t xdata *modbus_get_reg_addr(uint16_t reg_addr);
uint8_t modbus_get_bit_status(uint16_t reg_addr, uint8_t *bit_buf);
uint8_t modbus_write_bit_status(uint16_t reg_addr, uint8_t *bit_buf);
uint16_t modbus_get_reg_value(uint16_t reg_addr, uint8_t *bit_buf);
uint16_t modbus_write_reg_value(uint16_t reg_addr, uint16_t *reg_buf);

//modbus_RTU 主机程序
uint8_t modbus_RTU_read_coils(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint8_t *value, uint32_t timeout);              //(0x01) Read Coils
uint8_t modbus_RTU_read_discrete_input(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint8_t *value, uint32_t timeout);     //(0x02) Read Discrete Inputs
uint8_t modbus_RTU_read_holding_regs(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint16_t *RX_value_p, uint32_t timeout); //(0x03) Read Holding Registers
uint8_t modbus_RTU_write_single_coil(uint8_t slave_addr, uint16_t reg_addr, uint16_t value, uint32_t timeout);                          //(0x05) Write Single Coil
uint8_t modbus_RTU_write_single_reg(uint8_t slave_addr, uint16_t reg_addr, uint16_t value, uint32_t timeout);                           //(0x06) Write Single Register
uint8_t modbus_RTU_write_multi_coils(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint8_t *Value_p, uint32_t timeout);     //(0x0F) Write Multiple Coils
uint8_t modbus_RTU_write_multi_regs(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint16_t *Value_p, uint32_t timeout);     //(0x10) Write Multiple Registers

//modbus_RTU 从机程序
uint8_t modbus_RTU_slave_r1(uint8_t xdata *buffer_p, uint8_t data_len);     //(0x01) Read Coils
uint8_t modbus_RTU_slave_r2(uint8_t xdata *buffer_p, uint8_t data_len);     //(0x02) Read Discrete Inputs
uint8_t modbus_RTU_slave_r3(uint8_t xdata *buffer_p, uint8_t data_len);     //(0x03) Read Holding Registers
uint8_t modbus_RTU_slave_r4(uint8_t xdata *buffer_p, uint8_t data_len);     //(0x04) Read Input Registers
uint8_t modbus_RTU_slave_r5(uint8_t xdata *buffer_p, uint8_t data_len);     //(0x05) Write Single Coil
uint8_t modbus_RTU_slave_r6(uint8_t xdata *buffer_p, uint8_t data_len);     //(0x06) Write Single Register
uint8_t modbus_RTU_slave_r15(uint8_t xdata *buffer_p, uint8_t data_len);    //(0x0F) Write Multiple Coils
uint8_t modbus_RTU_slave_r16(uint8_t xdata *buffer_p, uint8_t data_len);    //(0x10) Write Multiple Registers

uint8_t modbus_RTU_slave_process(uint8_t xdata *buffer_p, uint8_t data_len);    //modbus_RTU 从机数据与应答处理
void modbus_RTU_slave(void);    //modbus_RTU 从机处理程序



//========================================================================
// 函数: uint8_t modbus_RTU_init(uint16_t brt, uint8_t mode, uint8_t addr)
// 描述: modbus_RTU 初始化函数
// 参数: brt: 串口波特率，该参数暂不使用.
//       mode: 模式: 0: 从机模式, 1: 主机模式.
//       addr: 节点地址.

// 返回: 1
//========================================================================
uint8_t modbus_RTU_init(uint16_t brt, uint8_t mode, uint8_t addr)
{
    uint16_t i;
    // UART3_config(0);
    UART4_config(2); 
    node_address = addr;
    node_mode = mode;

    for(i=0; i< (DISCRETES_INPUT_MAX + 7) / 8; i++)
    {
        discrete_input_register[i] = 0x00;
    }
    for(i=0; i< (COILS_MAX + 7) / 8; i++)
    {
        coil_register[i] = 0x00;
    }
    for(i=0; i< INPUT_REG_MAX; i++)
    {
        input_register[i] = 0x00;
    }
    for(i=0; i< HOLDING_REG_MAX; i++)
    {
        holding_register[i] = 0x0000;
    }
    return 1;
}

//获取当前模式
uint8_t modbus_mode(void)
{
    return node_mode;
}

//========================================================================
// 函数: uint16_t	MODBUS_CRC16(uint8_t *p, uint8_t n)
// 描述: 计算CRC16函数.
// 参数: *p: 要计算的数据指针.
//        n: 要计算的字节数.
// 返回: CRC16值.
// 版本: V1.0, 2022-3-18 梁工
//========================================================================
uint16_t MODBUS_CRC16(uint8_t xdata *p, uint8_t n)
{
	uint8_t	i;
	uint16_t crc16;

	crc16 = 0xffff;	//预置16位CRC寄存器为0xffff（即全为1）
	do
	{
		crc16 ^= (uint16_t)*p;	//把8位数据与16位CRC寄存器的低位相异或，把结果放于CRC寄存器
		for(i=0; i<8; i++)		//8位数据
		{
			if(crc16 & 1)	crc16 = (crc16 >> 1) ^ 0xA001;	//如果最低位为0，把CRC寄存器的内容右移一位(朝低位)，用0填补最高位，
															//再异或多项式0xA001
			else	crc16 >>= 1;							//如果最低位为0，把CRC寄存器的内容右移一位(朝低位)，用0填补最高位
		}
		p++;
	}while(--n != 0);
	return	(crc16);
}


//数据处理辅助函数：将data1的pos1位修改为data2的pos2位
void set_bit(uint8_t *data1, uint8_t pos1, uint8_t *data2, uint8_t pos2)
{
    uint8_t temp;

    //擦除目标位
    *data1 = ~(*data1);
    *data1 |= (0x01 << pos1);
    *data1 = ~(*data1);

    temp = (*data2 & (0x01 << pos2));
    temp = temp >> pos2;
    *data1 |= temp << pos1;//写入目标位
}


//modbus_RTU 主机写命令的应答检查
uint8_t modbus_response_WR_check(uint8_t xdata *RX_mesg, uint8_t xdata *TX_mesg)
{
    if(RX_mesg[0] != TX_mesg[0]) return 0;  //check slave address
    if(RX_mesg[1] != TX_mesg[1]) return 0;  //check function code
    if(RX_mesg[2] != TX_mesg[2]) return 0;  //check starting address Lo
    if(RX_mesg[3] != TX_mesg[3]) return 0;  //check starting address Hi
    if(RX_mesg[4] != TX_mesg[4]) return 0;  
    if(RX_mesg[5] != TX_mesg[5]) return 0;  
    return 1;
}


//获取指定本地寄存器地址
uint8_t xdata *modbus_get_reg_addr(uint16_t reg_addr)
{
    if((reg_addr >= DISCRETES_INPUT_START_ADDRESS) && (reg_addr < DISCRETES_INPUT_START_ADDRESS + DISCRETES_INPUT_MAX))
    {
        return &discrete_input_register[(reg_addr - DISCRETES_INPUT_START_ADDRESS) / 8];
    }
    else if((reg_addr >= COILS_START_ADDRESS) && (reg_addr < COILS_START_ADDRESS + COILS_MAX))
    {
        return &coil_register[(reg_addr - COILS_START_ADDRESS) / 8];
    }
    else if((reg_addr >= INPUT_REG_START_ADDRESS) && (reg_addr < INPUT_REG_START_ADDRESS + INPUT_REG_MAX))
    {
        return &input_register[(reg_addr - INPUT_REG_START_ADDRESS)];
    }
    else if((reg_addr >= HOLDING_REG_START_ADDRESS) && (reg_addr < HOLDING_REG_START_ADDRESS + HOLDING_REG_MAX))
    {
        return &holding_register[(reg_addr - HOLDING_REG_START_ADDRESS)];
    }
    return NULL;
}

//获取指定位值
uint8_t modbus_get_bit_status(uint16_t reg_addr, uint8_t *bit_buf)
{
    //检测目标是否为线圈寄存器
    if((reg_addr >= COILS_START_ADDRESS) && (reg_addr < COILS_START_ADDRESS + COILS_MAX))
    {
        //读取指定位
        *bit_buf = coil_register[(reg_addr - COILS_START_ADDRESS) / 8] & (0x01 << ((reg_addr - COILS_START_ADDRESS) % 8));
        *bit_buf = *bit_buf >> ((reg_addr - COILS_START_ADDRESS) % 8);
        return 1;
    }

    //检测目标是否为离散输入寄存器
    if((reg_addr >= DISCRETES_INPUT_START_ADDRESS) && (reg_addr < DISCRETES_INPUT_START_ADDRESS + DISCRETES_INPUT_MAX))
    {
        //读取指定位
        *bit_buf = discrete_input_register[(reg_addr - COILS_START_ADDRESS) / 8] & (0x01 << ((reg_addr - COILS_START_ADDRESS) % 8));
        *bit_buf = *bit_buf >> ((reg_addr - COILS_START_ADDRESS) % 8);
        return 1;
    }
    return 0;
}

//写入指定位值
uint8_t modbus_write_bit_status(uint16_t reg_addr, uint8_t *bit_buf)
{
    //检测目标是否为线圈寄存器
    if((reg_addr >= COILS_START_ADDRESS) && (reg_addr < COILS_START_ADDRESS + COILS_MAX))
    {
        //写入指定位
        set_bit(coil_register[(reg_addr - COILS_START_ADDRESS) / 8], ((reg_addr - COILS_START_ADDRESS) % 8), *bit_buf, 0);
        return 1;
    }
    return 0;
}

//获取指定寄存器值
uint16_t modbus_get_reg_value(uint16_t reg_addr, uint16_t *reg_buf)
{
    //检测目标是否为输入寄存器
    if((reg_addr >= INPUT_REG_START_ADDRESS) && (reg_addr < INPUT_REG_START_ADDRESS + INPUT_REG_MAX))
    {
        *reg_buf = input_register[(reg_addr - INPUT_REG_START_ADDRESS)];
        return 1;
    }

    //检测目标是否为保持寄存器
    if((reg_addr >= HOLDING_REG_START_ADDRESS) && (reg_addr < HOLDING_REG_START_ADDRESS + HOLDING_REG_MAX))
    {
        *reg_buf = holding_register[(reg_addr - HOLDING_REG_START_ADDRESS)];
        return 1;
    }
    return 0;
}

//写入指定寄存器值
uint16_t modbus_write_reg_value(uint16_t reg_addr, uint16_t *reg_buf)
{
    //检测目标是否为保持寄存器
    if((reg_addr >= HOLDING_REG_START_ADDRESS) && (reg_addr < HOLDING_REG_START_ADDRESS + HOLDING_REG_MAX))
    {
        holding_register[(reg_addr - HOLDING_REG_START_ADDRESS)] = *reg_buf;
        return 1;
    }
    return 0;
}


//modbus_RTU 主机指令，读取线圈(0x01)
uint8_t modbus_RTU_read_coils(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint8_t *value, uint32_t timeout)
{
    uint8_t i;
    uint8_t wait_flag;
    uint8_t response_error_code;
    uint8_t *RX_buffer_p_temp;
    uint16_t crc;
    uint32_t start_time;

    //检查寄存器数量
    if(quantity > 0x07D0) return 0x0F;

    RTU_TX_buffer[0] = slave_addr;      //从站地址
    RTU_TX_buffer[1] = 0x01;            //功能码
    RTU_TX_buffer[2] = (uint8_t)(reg_addr >> 8); //存储器起始地址
    RTU_TX_buffer[3] = (uint8_t)(reg_addr);
    RTU_TX_buffer[4] = (uint8_t)(quantity >> 8); //线圈数量
    RTU_TX_buffer[5] = (uint8_t)(quantity);

    crc = MODBUS_CRC16(RTU_TX_buffer, 6);
    RTU_TX_buffer[6] = (uint8_t)(crc);
    RTU_TX_buffer[7] = (uint8_t)(crc >> 8);

    UART4_send(RTU_TX_buffer, 8);//发送请求帧

    //等待从机响应
    wait_flag = 1;
    start_time = get_systick();
    while(wait_flag)
    {
        if((get_systick() - start_time) > timeout)
        {
            response_error_code = 1;
            wait_flag = 0;  //响应超时
        }
        if(B_RX4_OK == 1)
        {
            //验证响应帧
            response_error_code = 0;
            RX_buffer_p_temp = get_RX4_buffer_address();
            if(MODBUS_CRC16(RX_buffer_p_temp, get_RX4_buffer_length())) response_error_code = 0x59;
            if(RX_buffer_p_temp[0] != slave_addr) response_error_code = 0x60;
            if(RX_buffer_p_temp[1] != 0x01) response_error_code = 0x61;
            if(RX_buffer_p_temp[2] != (quantity + 7) / 8) response_error_code = 0x62;

            if(response_error_code == 0)//响应帧验证成功
            {
                for(i = 0; i < RX_buffer_p_temp[2]; i++)//填充寄存器数据
                {
                    value[i] = RX_buffer_p_temp[3 + i];     
                }
                wait_flag = 0;
            }
            UART4_RX_buffer_reset();//清接收标志
        }
    }
    return response_error_code;
}

//modbus_RTU 主机指令，读取离散输入(0x02)。实际除功能码外其余与读取线圈完全一致
uint8_t modbus_RTU_read_discrete_input(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint8_t *value, uint32_t timeout)
{
    uint8_t i;
    uint8_t wait_flag;
    uint8_t response_error_code;
    uint8_t *RX_buffer_p_temp;
    uint16_t crc;
    uint32_t start_time;

    //检查寄存器数量
    if(quantity > 0x07D0) return 0x0F;
    RTU_TX_buffer[0] = slave_addr;      //从站地址
    RTU_TX_buffer[1] = 0x02;            //功能码
    RTU_TX_buffer[2] = (uint8_t)(reg_addr >> 8); //存储器起始地址
    RTU_TX_buffer[3] = (uint8_t)(reg_addr);
    RTU_TX_buffer[4] = (uint8_t)(quantity >> 8); //线圈数量
    RTU_TX_buffer[5] = (uint8_t)(quantity);

    crc = MODBUS_CRC16(RTU_TX_buffer, 6);
    RTU_TX_buffer[6] = (uint8_t)(crc);
    RTU_TX_buffer[7] = (uint8_t)(crc >> 8);

    UART4_send(RTU_TX_buffer, 8);//发送请求帧

    //等待从机响应
    wait_flag = 1;
    start_time = get_systick();
    while(wait_flag)
    {
        if((get_systick() - start_time) > timeout)
        {
            response_error_code = 1;
            wait_flag = 0;  //响应超时
        }
        if(B_RX4_OK == 1)
        {
            //验证响应帧
            response_error_code = 0;
            RX_buffer_p_temp = get_RX4_buffer_address();
            crc = MODBUS_CRC16(RX_buffer_p_temp, get_RX4_buffer_length());
            if(crc) response_error_code = 0x59;
            if(RX_buffer_p_temp[0] != slave_addr) response_error_code = 0x60;
            if(RX_buffer_p_temp[1] != 0x02) response_error_code = 0x61;
            if(RX_buffer_p_temp[2] != (uint8_t)((quantity + 7) / 8)) response_error_code = 0x62;

            if(response_error_code == 0)//响应帧验证成功
            {
                for(i = 0; i < RX_buffer_p_temp[2]; i++)//填充寄存器数据
                {
                    value[i] = RX_buffer_p_temp[3 + i];     
                }
                wait_flag = 0;
            }
            UART4_RX_buffer_reset();//清接收标志
        }
    }
    return response_error_code;
}

//modbus_RTU 主机指令，读取寄存器(0x03)
uint8_t modbus_RTU_read_holding_regs(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint16_t *RX_value_p, uint32_t timeout)
{
    uint8_t i;
    uint8_t wait_flag;
    uint8_t response_error_code;
    uint8_t *RX_buffer_p;
    uint16_t crc;
    uint32_t start_time;

    //检查寄存器数量
    if(quantity > 0x007D) return 0x0F;
    RTU_TX_buffer[0] = slave_addr;      //从站地址
    RTU_TX_buffer[1] = 0x03;            //功能码
    RTU_TX_buffer[2] = (uint8_t)(reg_addr >> 8); //存储器起始地址
    RTU_TX_buffer[3] = (uint8_t)(reg_addr);
    RTU_TX_buffer[4] = (uint8_t)(quantity >> 8); //寄存器数量
    RTU_TX_buffer[5] = (uint8_t)(quantity);
    crc = MODBUS_CRC16(RTU_TX_buffer, 6);
    RTU_TX_buffer[6] = (uint8_t)(crc);
    RTU_TX_buffer[7] = (uint8_t)(crc >> 8);

    UART4_send(RTU_TX_buffer, 8);//发送请求帧

    //等待从机响应
    wait_flag = 1;
    start_time = get_systick();
    while(wait_flag)
    {
        if((get_systick() - start_time) > timeout)
        {
            response_error_code = 1;
            wait_flag = 0;  //响应超时
            // PrintString2("Response time out!\r\n");
        }
        if(B_RX4_OK == 1)
        {
            //验证响应帧
            response_error_code = 0;
            RX_buffer_p = get_RX4_buffer_address();
            crc = MODBUS_CRC16(RX_buffer_p, get_RX4_buffer_length());
            if(crc) response_error_code = 0x59;
            if(RX_buffer_p[0] != slave_addr) response_error_code = 0x60;
            if(RX_buffer_p[1] != 0x03) response_error_code = 0x61;
            if(RX_buffer_p[2] != (uint8_t)(quantity * 2)) response_error_code = 0x62;

            if(response_error_code == 0)//响应帧验证成功
            {
                for(i = 0; i < RX_buffer_p[2] / 2; i++)//填充寄存器数据
                {
                    RX_value_p[i] = (uint16_t)(RX_buffer_p[3 + 2 * i]) << 8;;     
                    RX_value_p[i] |= (uint16_t)(RX_buffer_p[4 + 2 * i]);
                }
                wait_flag = 0;
            }
            UART4_RX_buffer_reset();//清接收标志
        }
    }
    return response_error_code;
}



//modbus_RTU 主机指令，写入单线圈(0x05)
uint8_t modbus_RTU_write_single_coil(uint8_t slave_addr, uint16_t reg_addr, uint16_t value, uint32_t timeout)
{
    uint8_t wait_flag;
    uint8_t response_error_code;
    uint16_t crc;
    uint32_t start_time;

    //设定线圈状态码
    if(value) value = 0xFF00;
    else value = 0x00; 

    RTU_TX_buffer[0] = slave_addr;      //从站地址
    RTU_TX_buffer[1] = 0x05;            //功能码
    RTU_TX_buffer[2] = (uint8_t)(reg_addr >> 8); //存储器起始地址
    RTU_TX_buffer[3] = (uint8_t)(reg_addr);
    RTU_TX_buffer[4] = (uint8_t)(value >> 8); //线圈状态码
    RTU_TX_buffer[5] = (uint8_t)(value);
    crc = MODBUS_CRC16(RTU_TX_buffer, 6);
    RTU_TX_buffer[6] = (uint8_t)(crc);
    RTU_TX_buffer[7] = (uint8_t)(crc >> 8);

    UART4_send(RTU_TX_buffer, 8);//发送请求帧

    //等待从机响应
    wait_flag = 1;
    start_time = get_systick();
    while(wait_flag)
    {
        if((get_systick() - start_time) > timeout)
        {
            response_error_code = 1;
            wait_flag = 0;
        }
        if(B_RX4_OK == 1)
        {
            response_error_code = 0;
            //验证响应帧
            if(MODBUS_CRC16(get_RX4_buffer_address(), get_RX4_buffer_length()) == 0)	//CRC16校验，错误帧丢弃
            {
                response_error_code = !modbus_response_WR_check(RTU_TX_buffer, get_RX4_buffer_address());   //内容检查
                if(response_error_code == 0)
                {
                    wait_flag = 0;
                }
            }
            else
            {
                response_error_code = 0xF0;
            }
            UART4_RX_buffer_reset();//清接收标志
        }
    }
    return response_error_code;
}

//modbus_RTU 主机指令，写入单寄存器(0x06)
uint8_t modbus_RTU_write_single_reg(uint8_t slave_addr, uint16_t reg_addr, uint16_t value, uint32_t timeout)
{
    uint8_t wait_flag;
    uint8_t response_error_code;
    uint16_t crc;
    uint32_t start_time;

    RTU_TX_buffer[0] = slave_addr;      //从站地址
    RTU_TX_buffer[1] = 0x06;            //功能码
    RTU_TX_buffer[2] = (uint8_t)(reg_addr >> 8); //存储器起始地址
    RTU_TX_buffer[3] = (uint8_t)(reg_addr);
    RTU_TX_buffer[4] = (uint8_t)(value >> 8);    //寄存器值
    RTU_TX_buffer[5] = (uint8_t)(value);
    crc = MODBUS_CRC16(RTU_TX_buffer, 6);
    RTU_TX_buffer[6] = (uint8_t)(crc);
    RTU_TX_buffer[7] = (uint8_t)(crc >> 8);

    UART4_send(RTU_TX_buffer, 8);//发送请求帧

    //等待从机响应
    if(RTU_TX_buffer[0] != 0)
    {
        wait_flag = 1;
        start_time = get_systick();
        while(wait_flag)
        {
            if((get_systick() - start_time) > timeout)
            {
                response_error_code = 1;
                wait_flag = 0;
            }
            if(B_RX4_OK == 1)
            {
                response_error_code = 0;
                //验证响应帧
                if(MODBUS_CRC16(get_RX4_buffer_address(), get_RX4_buffer_length()) == 0)	//CRC16校验，错误帧丢弃
                {
                    response_error_code = !modbus_response_WR_check(RTU_TX_buffer, get_RX4_buffer_address());   //内容检查
                    if(response_error_code == 0)
                    {
                        wait_flag = 0;
                    }
                }
                else
                {
                    response_error_code = 0xF0;
                }
                UART4_RX_buffer_reset();//清接收标志
            }
        }
    }
    else response_error_code = 0;
    
    return response_error_code;
}

//modbus_RTU 主机指令，写入多个线圈(0x0F)
uint8_t modbus_RTU_write_multi_coils(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint8_t *Value_p, uint32_t timeout)
{
    uint8_t i;
    uint8_t wait_flag;
    uint8_t response_error_code;
    uint16_t crc;
    uint32_t start_time;

    //检查线圈数量
    if(quantity > 0x07B0) return 0x0F;
    RTU_TX_buffer[0] = slave_addr;      //从站地址
    RTU_TX_buffer[1] = 0x0F;            //功能码
    RTU_TX_buffer[2] = (uint8_t)(reg_addr >> 8); //存储器起始地址
    RTU_TX_buffer[3] = (uint8_t)(reg_addr);
    RTU_TX_buffer[4] = (uint8_t)(quantity >> 8); //线圈数量
    RTU_TX_buffer[5] = (uint8_t)(quantity);
    RTU_TX_buffer[6] = (uint8_t)(quantity / 8 + (quantity % 8 ? 1 : 0)); //字节数

    //填充线圈数据
    for(i = 0; i < RTU_TX_buffer[6]; i++)
    {
        RTU_TX_buffer[7 + i] = Value_p[i];
    }
    //尾字节多余数据置0
    if(quantity % 8) RTU_TX_buffer[6 + RTU_TX_buffer[6]] &= ~(0xFF << (quantity % 8));

    crc = MODBUS_CRC16(RTU_TX_buffer, (uint8_t)(7 + RTU_TX_buffer[6]));
    RTU_TX_buffer[7 + RTU_TX_buffer[6]] = (uint8_t)(crc);
    RTU_TX_buffer[7 + 1 + RTU_TX_buffer[6]] = (uint8_t)(crc >> 8);

    UART4_send(RTU_TX_buffer, (uint8_t)(7 + 2 + RTU_TX_buffer[6]));//发送请求帧

    //等待从机响应
    if(RTU_TX_buffer[0] != 0)
    {
        wait_flag = 1;
        start_time = get_systick();
        while(wait_flag)
        {
            if((get_systick() - start_time) > timeout)
            {
                response_error_code = 1;
                wait_flag = 0;
            }
            if(B_RX4_OK == 1)
            {
                response_error_code = 0;
                //验证响应帧
                if(MODBUS_CRC16(get_RX4_buffer_address(), get_RX4_buffer_length()) == 0)	//CRC16校验，错误帧丢弃
                {
                    response_error_code = !modbus_response_WR_check(RTU_TX_buffer, get_RX4_buffer_address());   //内容检查
                    if(response_error_code == 0)
                    {
                        wait_flag = 0;
                    }
                }
                else
                {
                    response_error_code = 0xF0;
                }
                UART4_RX_buffer_reset();//清接收标志
            }
        }
    }
    else response_error_code = 0;
    return response_error_code;
}


//modbus_RTU 主机指令，写入多个寄存器(0x10)
uint8_t modbus_RTU_write_multi_regs(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint16_t *Value_p, uint32_t timeout)
{
    uint8_t i;
    uint8_t wait_flag;
    uint8_t response_error_code;
    uint16_t crc;
    uint32_t start_time;

    //检查寄存器数量
    if(quantity > 0x007B) return 0x0F;
    RTU_TX_buffer[0] = slave_addr;      //从站地址
    RTU_TX_buffer[1] = 0x10;            //功能码
    RTU_TX_buffer[2] = (uint8_t)(reg_addr >> 8); //存储器起始地址
    RTU_TX_buffer[3] = (uint8_t)(reg_addr);
    RTU_TX_buffer[4] = (uint8_t)(quantity >> 8); //寄存器数量
    RTU_TX_buffer[5] = (uint8_t)(quantity);
    RTU_TX_buffer[6] = (uint8_t)(quantity * 2); //字节数
    for(i = 0; i < quantity; i++)  //填充寄存器数据
    {
        RTU_TX_buffer[7 + 2 * i] = (uint8_t)(Value_p[i] >> 8);
        RTU_TX_buffer[7 + 1 + 2 * i] = (uint8_t)(Value_p[i]);
    }
    crc = MODBUS_CRC16(RTU_TX_buffer, (uint8_t)(7 + 2 * quantity));
    RTU_TX_buffer[7 + 2 * quantity] = (uint8_t)(crc);
    RTU_TX_buffer[7 + 1 + 2 * quantity] = (uint8_t)(crc >> 8);

    UART4_send(RTU_TX_buffer, (uint8_t)(7 + 2 + 2 * quantity));//发送请求帧

    //等待从机响应
    if(RTU_TX_buffer[0] != 0)
    {
        wait_flag = 1;
        start_time = get_systick();
        while(wait_flag)
        {
            if((get_systick() - start_time) > timeout)
            {
                response_error_code = 1;
                wait_flag = 0;  //响应超时
            }
            if(B_RX4_OK == 1)
            {
                response_error_code = 0;
                //验证响应帧
                if(MODBUS_CRC16(get_RX4_buffer_address(), get_RX4_buffer_length()) == 0)	//CRC16校验，错误帧丢弃
                {
                    response_error_code = !modbus_response_WR_check(RTU_TX_buffer, get_RX4_buffer_address());   //内容检查
                    if(response_error_code == 0)
                    {
                        wait_flag = 0;
                    }
                }
                else response_error_code = 0xF0;
                UART4_RX_buffer_reset();//清接收标志
            }
        }
    }
    else response_error_code = 0;
    return response_error_code;
}







//modbus_RTU 从机数据处理
/*
    [支持的指令]

 |读线圈状态（0x01）|
  请求
数据:   站号(地址)  功能码   寄存地址   寄存器个数    CRC16
偏移:      0          1       2 3         4 5       6 7
字节:   1 byte   1 byte    1byte      2*n byte  2 byte

  返回
数据:站号(地址)  功能码   读出字节数  读出数据  CRC16
偏移:      0       1        2           3~      最后2字节
字节:   1 byte   1 byte    1byte      2*n byte  2 byte


 |读离散输入（0x02）|
  请求
数据:   站号(地址)  功能码   寄存地址   寄存器个数    CRC16
偏移:      0          1       2 3         4 5       6 7
字节:   1 byte   1 byte    1byte      2*n byte  2 byte

  返回
数据:站号(地址)  功能码   读出字节数  读出数据  CRC16
偏移:      0       1        2           3~      最后2字节
字节:   1 byte   1 byte    1byte      2*n byte  2 byte

 |读保持寄存器（0x03）|
  请求
数据: 	 地址    功能码   寄存地址 寄存器个数  写入字节数   写入数据   CRC16
偏移:     0        1        2 3      4 5          6          7~        最后2字节
字节:   1 byte   1 byte    2 byte   2 byte      1byte       2*n byte   2 byte

  返回
数据:站号(地址)  功能码   写入字节数  CRC16
偏移:      0       1        2       最后2字节
字节:   1 byte   1 byte    1byte    2 byte


......(懒得写了)

*/

//modbus_RTU 从机读线圈指令处理(0x01)
uint8_t modbus_RTU_slave_r1(uint8_t xdata *buffer_p, uint8_t data_len)
{
    uint8_t i;
    uint16_t coil_num; //写入线圈个数
    uint16_t coil_index;
	uint16_t reg_addr;	//寄存器地址
    uint16_t reg_addr_temp;
    uint16_t crc;

    if(data_len != 6) return 0x91;	//命令长度错误
    if((((uint16_t)buffer_p[4] >> 8) + (uint16_t)buffer_p[5]) > HOLDING_REG_MAX) return 0x93; //读取线圈个数错误

    reg_addr = (uint16_t)(buffer_p[2] << 8) + buffer_p[3] ;
    coil_num = (uint16_t)(buffer_p[4] << 8) + buffer_p[5];
    if((reg_addr < COILS_START_ADDRESS) || (reg_addr > COILS_START_ADDRESS + COILS_MAX)) return 0x92;   //线圈地址错误

    //指令正确，开始准备应答
    if(buffer_p[0] != 0)	//非广播地址则应答
    {
        RTU_TX_buffer[0] = node_address;            //站号地址
        RTU_TX_buffer[1] = 0x01;		            //功能码
        RTU_TX_buffer[2] = (coil_num + 7) / 8;  //返回字节数，向上取整

        reg_addr_temp = (reg_addr - COILS_START_ADDRESS) / 8;
        coil_index = (reg_addr - COILS_START_ADDRESS) % 8;   //定位起始位
        for(i = 0; i < coil_num; i++)      //读取线圈值
        {
            if(i % 8 == 0) RTU_TX_buffer[3 + i / 8] = 0;
            set_bit(&RTU_TX_buffer[3 + i / 8], (uint8_t)(i % 8), &coil_register[reg_addr_temp + coil_index / 8], (uint8_t)(coil_index % 8));
            coil_index++;
        }
        crc = MODBUS_CRC16(RTU_TX_buffer, (uint8_t)(3 + RTU_TX_buffer[2]));    
        RTU_TX_buffer[3 + RTU_TX_buffer[2]] = crc;
        RTU_TX_buffer[4 + RTU_TX_buffer[2]] = (uint8_t)(crc >> 8);

        UART4_send(RTU_TX_buffer, (uint8_t)(5 + RTU_TX_buffer[2]));
    }
    return 0;
}

//modbus_RTU 从机读离散输入指令处理(0x02)
uint8_t modbus_RTU_slave_r2(uint8_t xdata *buffer_p, uint8_t data_len)
{
    uint8_t i;
    uint16_t descrete_num; //写入线圈个数
	uint16_t reg_addr;	//寄存器地址
    uint16_t reg_addr_temp;
    uint16_t descrete_index;
    uint16_t crc;

    if(data_len != 6) return 0x91;	//命令长度错误
    if((((uint16_t)buffer_p[4] >> 8) + (uint16_t)buffer_p[5]) > HOLDING_REG_MAX) return 0x93; //读取输入个数错误

    reg_addr = (uint16_t)(buffer_p[2] << 8) + buffer_p[3] ;
    descrete_num = (uint16_t)(buffer_p[4] << 8) + buffer_p[5];
    if((reg_addr < DISCRETES_INPUT_START_ADDRESS) || (reg_addr > DISCRETES_INPUT_START_ADDRESS + DISCRETES_INPUT_MAX)) return 0x92;   //离散输入地址错误

    //指令正确，开始准备应答
    if(buffer_p[0] != 0)	//非广播地址则应答
    {
        RTU_TX_buffer[0] = node_address;            //站号地址
        RTU_TX_buffer[1] = 0x01;		            //功能码
        RTU_TX_buffer[2] = (descrete_num + 7) / 8;  //返回字节数，向上取整

        reg_addr_temp = (reg_addr - COILS_START_ADDRESS) / 8;
        descrete_index = (reg_addr - COILS_START_ADDRESS) % 8;   //定位起始位
        for(i = 0; i < descrete_num; i++)      //读取离散输入值
        {
            if(i % 8 == 0) RTU_TX_buffer[3 + i / 8] = 0;
            set_bit(&RTU_TX_buffer[3 + i / 8], (uint8_t)(i % 8), &discrete_input_register[reg_addr_temp + descrete_index / 8], (uint8_t)(descrete_index % 8));
            descrete_index++;
        }
        crc = MODBUS_CRC16(RTU_TX_buffer, (uint8_t)(3 + RTU_TX_buffer[2]));    
        RTU_TX_buffer[3 + RTU_TX_buffer[2]] = crc;
        RTU_TX_buffer[4 + RTU_TX_buffer[2]] = (uint8_t)(crc >> 8);

        UART4_send(RTU_TX_buffer, (uint8_t)(5 + RTU_TX_buffer[2]));
    }
    return 0;
}

//modbus_RTU 从机读多保持寄存器指令处理(0x03)
uint8_t modbus_RTU_slave_r3(uint8_t xdata *buffer_p, uint8_t data_len)
{
    uint8_t i,j;
    uint8_t reg_num;	//写入寄存器个数
	uint16_t reg_addr;	//寄存器地址
    uint16_t reg_addr_temp;
    uint16_t crc;

    if(buffer_p[0] != 0)	//非广播地址则应答
    {
        if(data_len != 6)		return 0x91;	//命令长度错误
        if(buffer_p[4] != 0)	return 0x93;    //读出寄存器个数错误
        if((buffer_p[5]==0) || (buffer_p[5] > HOLDING_REG_MAX))	return 0x93;	//读出寄存器个数错误
        reg_addr = ((uint16_t)buffer_p[2] << 8) + buffer_p[3];	//寄存器地址
        reg_num = buffer_p[5];	//读出寄存器个数

        if((reg_addr + (uint16_t)buffer_p[5]) > (HOLDING_REG_START_ADDRESS + HOLDING_REG_MAX)) return 0x92;	//寄存器地址错误
        if(reg_addr < HOLDING_REG_START_ADDRESS) return 0x92;	//寄存器地址错误

        //指令正确，开始准备应答
        RTU_TX_buffer[0] = node_address;    //站号地址
        RTU_TX_buffer[1] = 0x03;		    //读功能码
        RTU_TX_buffer[2] = reg_num * 2;	    //返回字节数
        reg_addr_temp = reg_addr - HOLDING_REG_START_ADDRESS;	//寄存器数据下标
        
        for(i = 0, j = 3; i < reg_num; i++)
        {
            RTU_TX_buffer[j] = (uint8_t)(holding_register[reg_addr_temp] >> 8);
            RTU_TX_buffer[j+1] = (uint8_t)holding_register[reg_addr_temp];
            reg_addr_temp++;
            j += 2;
        }
        crc = MODBUS_CRC16(RTU_TX_buffer, (uint8_t)(3 + RTU_TX_buffer[2]));
        RTU_TX_buffer[3 + RTU_TX_buffer[2]] = crc;
        RTU_TX_buffer[4 + RTU_TX_buffer[2]] = (uint8_t)(crc >> 8);
        UART4_send(RTU_TX_buffer, (uint8_t)(5 + RTU_TX_buffer[2]));
    }
    return 0;
}

//modbus_RTU 从机读多输入寄存器指令处理(0x04)
uint8_t modbus_RTU_slave_r4(uint8_t xdata *buffer_p, uint8_t data_len)
{
    uint8_t i,j;
    uint8_t reg_num;	//写入寄存器个数
	uint16_t reg_addr;	//寄存器地址
    uint16_t reg_addr_temp;
    uint16_t crc;

    if(buffer_p[0] != 0)	//非广播地址则应答
    {
        if(data_len != 6)		return 0x91;	//命令长度错误
        if(buffer_p[4] != 0)	return 0x93;    //读出寄存器个数错误
        if((buffer_p[5]==0) || (buffer_p[5] > INPUT_REG_MAX))	return 0x93;	//读出寄存器个数错误
        reg_addr = ((uint16_t)buffer_p[2] << 8) + buffer_p[3];	//寄存器地址
        reg_num = buffer_p[5];	//读出寄存器个数

        if((reg_addr + (uint16_t)buffer_p[5]) > (INPUT_REG_START_ADDRESS + INPUT_REG_MAX)) return 0x92;	//寄存器地址错误
        if(reg_addr < INPUT_REG_START_ADDRESS) return 0x92;	//寄存器地址错误

        //指令正确，开始准备应答
        if(buffer_p[0] != 0)	//非广播地址则应答
        {
            RTU_TX_buffer[0] = node_address;    //站号地址
            RTU_TX_buffer[1] = 0x04;		    //读功能码
            RTU_TX_buffer[2] = reg_num * 2;	    //返回字节数
            reg_addr_temp = reg_addr - INPUT_REG_START_ADDRESS;	//寄存器数据下标
            
            for(i = 0, j = 3; i < reg_num; i++)
            {
                RTU_TX_buffer[j] = (uint8_t)(holding_register[reg_addr_temp] >> 8);
                RTU_TX_buffer[j+1] = (uint8_t)holding_register[reg_addr_temp];
                reg_addr_temp++;
                j += 2;
            }
            crc = MODBUS_CRC16(RTU_TX_buffer, (uint8_t)(3 + RTU_TX_buffer[2]));
            RTU_TX_buffer[3 + RTU_TX_buffer[2]] = crc;
            RTU_TX_buffer[4 + RTU_TX_buffer[2]] = (uint8_t)(crc >> 8);
            UART4_send(RTU_TX_buffer, (uint8_t)(5 + RTU_TX_buffer[2]));
        }
    }
    return 0;
}

//modbus_RTU 从机写单线圈指令处理(0x05)
uint8_t modbus_RTU_slave_r5(uint8_t xdata *buffer_p, uint8_t data_len)
{
    uint8_t i,j;
	uint16_t reg_addr;	//寄存器地址
    uint16_t reg_addr_temp;
    uint16_t crc;

    if(data_len != 6) return 0x91;	//命令长度错误
    if((buffer_p[4]!=0xff || buffer_p[5]!=0x00) && buffer_p[4]!=0x00) return 0x93;    //写入数据错误

    reg_addr = ((uint16_t)buffer_p[2] << 8) + buffer_p[3];	//读取寄存器地址
    if(reg_addr >= (COILS_START_ADDRESS + COILS_MAX)) return 0x92;	    //线圈地址错误
    if(reg_addr < COILS_START_ADDRESS) return 0x92;     //线圈地址错误

    //指令正确，开始写入寄存器
    reg_addr_temp = reg_addr - COILS_START_ADDRESS;
    i = reg_addr_temp / 8; //定位寄存器索引
    j = reg_addr_temp % 8; //定位寄存器位
    if(buffer_p[4] == 0xff) coil_register[i] |= 0x01 << j;  //写入数据, 大端模式
    else coil_register[i] &= (~(0x01 << j));

    //发送应答
    if(buffer_p[0] != 0)	//非广播地址则应答
    {
        for(i = 0; i < 6; i++)	RTU_TX_buffer[i] = buffer_p[i];	//要返回的应答
        crc = MODBUS_CRC16(RTU_TX_buffer, 6);
        RTU_TX_buffer[6] = (uint8_t)crc;
        RTU_TX_buffer[7] = (uint8_t)(crc >> 8);
        UART4_send(RTU_TX_buffer, 8);
    }
    return 0;
}

//modbus_RTU 从机写单寄存器指令处理(0x06)
uint8_t modbus_RTU_slave_r6(uint8_t xdata *buffer_p, uint8_t data_len)
{
    uint8_t i,j;
    uint8_t reg_num;	//写入寄存器个数
	uint16_t reg_addr;	//寄存器地址
    uint16_t reg_addr_temp;
    uint16_t crc;

    if(data_len != 6) return 0x91;   //命令长度错误

    reg_addr = ((uint16_t)buffer_p[2] << 8) + buffer_p[3];	//读取寄存器地址
    if((reg_addr) > (HOLDING_REG_START_ADDRESS + HOLDING_REG_MAX)) return 0x93;	//寄存器地址错误
    if(reg_addr < HOLDING_REG_START_ADDRESS) return 0x93;	//寄存器地址错误

    //指令正确，开始写入寄存器
    reg_addr_temp = reg_addr - HOLDING_REG_START_ADDRESS;
    holding_register[reg_addr_temp] = ((uint16_t)buffer_p[4] << 8) + (uint16_t)buffer_p[5];	//写入数据, 大端模式

    //发送应答
    if(buffer_p[0] != 0)	//非广播地址则应答
    {
        for(i = 0; i < 6; i++)	RTU_TX_buffer[i] = buffer_p[i];	//要返回的应答
        crc = MODBUS_CRC16(RTU_TX_buffer, 6);
        RTU_TX_buffer[6] = (uint8_t)crc;
        RTU_TX_buffer[7] = (uint8_t)(crc >> 8);
        UART4_send(RTU_TX_buffer, 8);
    }
    return 0;
}

//modbus_RTU 从机写多线圈指令处理(0x0F)
uint8_t modbus_RTU_slave_r15(uint8_t xdata *buffer_p, uint8_t data_len)
{
    uint8_t i;
    uint16_t descrete_num; //写入线圈个数
    uint16_t coil_index;
	uint16_t reg_addr;	//寄存器地址
    uint16_t reg_addr_temp;
    uint16_t crc;

    if(data_len < 8) return 0x91;   //命令长度错误

    descrete_num = ((uint16_t)buffer_p[4] << 8) + buffer_p[5];    //线圈数
    if(((descrete_num + 7) / 8) != buffer_p[6])	return 0x93;	//写入线圈个数与字节数错误
    if((descrete_num == 0) || (descrete_num > COILS_MAX))	return 0x93;	  //写入线圈个数错误

    reg_addr = ((uint16_t)buffer_p[2] << 8) + buffer_p[3];	//读取寄存器地址
    if(reg_addr >= (COILS_START_ADDRESS + COILS_MAX)) return 0x92;  //线圈地址错误
    if(reg_addr < COILS_START_ADDRESS) return 0x92;     //线圈地址错误

    //指令正确，开始写入寄存器
    reg_addr_temp = ((reg_addr - COILS_START_ADDRESS) / 8);
    coil_index = (reg_addr - COILS_START_ADDRESS) % 8;   //定位起始位
    for(i = 0; i < descrete_num; i++)    //写入数据
    {
        set_bit(&coil_register[reg_addr_temp + coil_index / 8], (uint8_t)(coil_index % 8), &buffer_p[7 + i / 8], (uint8_t)(i % 8));
        coil_index ++;
    }

    //发送应答
    if(buffer_p[0] != 0)	//非广播地址则应答
    {
        for(i = 0; i < 6; i++)	RTU_TX_buffer[i] = buffer_p[i];	//要返回的应答
        crc = MODBUS_CRC16(RTU_TX_buffer, 6);
        RTU_TX_buffer[6] = (uint8_t)crc;
        RTU_TX_buffer[7] = (uint8_t)(crc >> 8);
        UART4_send(RTU_TX_buffer, 8);
    }
    return 0;
}

//modbus_RTU 从机写多保持寄存器指令处理(0x0F)
uint8_t modbus_RTU_slave_r16(uint8_t xdata *buffer_p, uint8_t data_len)
{
    uint8_t i,j;
    uint8_t reg_num;	//写入寄存器个数
	uint16_t reg_addr;	//寄存器地址
    uint16_t reg_addr_temp;
    uint16_t crc;

    if(data_len < 9) return 0x91;   //命令长度错误
    if((buffer_p[4] != 0) || ((buffer_p[5] *2) != buffer_p[6]))	return 0x92;	//写入寄存器个数与字节数错误
    if((buffer_p[5]==0) || (buffer_p[5] > HOLDING_REG_MAX))	return 0x92;	    //写入寄存器个数错误

    reg_addr = ((uint16_t)buffer_p[2] << 8) + buffer_p[3];	//读取寄存器地址
    reg_num = buffer_p[5];	    //读取写入寄存器个数
    
    if((reg_addr + (uint16_t)buffer_p[5]) > (HOLDING_REG_START_ADDRESS + HOLDING_REG_MAX)) return 0x93;	//寄存器地址错误
    if(reg_addr < HOLDING_REG_START_ADDRESS) return 0x93;	//寄存器地址错误
    if((buffer_p[6] + 7) != data_len) return 0x91;	        //命令长度错误

    //指令正确，开始写入寄存器
    reg_addr_temp = reg_addr - HOLDING_REG_START_ADDRESS;
    for(i = 0, j = 7; i < reg_num; i++)
    {
        holding_register[reg_addr_temp] = ((uint16_t)buffer_p[j] << 8) + (uint16_t)buffer_p[j+1];	//写入数据, 大端模式
        reg_addr_temp++;
        j += 2;
    }

    //发送应答
    if(buffer_p[0] != 0)	//非广播地址则应答
    {
        for(i = 0; i < 6; i++)	RTU_TX_buffer[i] = buffer_p[i];	//要返回的应答
        crc = MODBUS_CRC16(RTU_TX_buffer, 6);
        RTU_TX_buffer[6] = (uint8_t)crc;
        RTU_TX_buffer[7] = (uint8_t)(crc >> 8);
        UART4_send(RTU_TX_buffer, 8);
    }
    return 0;
}

uint8_t modbus_RTU_slave_process(uint8_t xdata *buffer_p, uint8_t data_len)
{
    uint8_t error_code;
    error_code = 0xFF;
    switch(buffer_p[1])
    {
        case 0x01:  // [读线圈]
            error_code = modbus_RTU_slave_r1(buffer_p, data_len);
            break;
        case 0x02:  // [读离散输入]
            error_code = modbus_RTU_slave_r2(buffer_p, data_len);
            break;
        case 0x03:  //  [读多保持寄存器]
            error_code = modbus_RTU_slave_r3(buffer_p, data_len);
            break;
        case 0x04:  //  [读多输入寄存器]
            error_code = modbus_RTU_slave_r4(buffer_p, data_len);
            break;
        case 0x05:  //  [写单线圈]
            error_code = modbus_RTU_slave_r5(buffer_p, data_len);
            break;
        case 0x06:  //  [写单保持寄存器]
            error_code = modbus_RTU_slave_r6(buffer_p, data_len);
            break;
        case 0x0f:  //  [写多线圈]
            error_code = modbus_RTU_slave_r15(buffer_p, data_len);
            break;
        case 0x10:  //  [写多寄存器]
            error_code = modbus_RTU_slave_r16(buffer_p, data_len);
            break;
    }
    return error_code;
}

//modbus从机处理程序
void modbus_RTU_slave(void)
{
    uint16_t i;
    uint8_t error_code;
    uint8_t data_len;
    uint8_t xdata *buffer_p;

    error_code = 0xff;
    if(B_RX4_OK == 1)
    {
        // PrintString1("Receive data! \r\n");
        data_len = get_RX4_buffer_length();
        buffer_p = get_RX4_buffer_address();
        if(MODBUS_CRC16(buffer_p, data_len) == 0)	//CRC16校验，错误帧丢弃
        {
            // PrintString1("CRC correct! \r\n");
            if(data_len > 2) data_len -= 2;	//减去CRC16校验字节
            if((buffer_p[0] == 0x00) || (buffer_p[0] == node_address))	//站号比对（地址相符或为广播地址）
            {
                error_code = modbus_RTU_slave_process(buffer_p, data_len);	//MODBUS-RTU协议解析
                if(error_code != 0)	//数据错误处理
                {

                }
            }
        }
        else 
        {
            // PrintString1("CRC error! \r\n");
        }
        UART4_RX_buffer_reset();//清接收标志

    }
}

