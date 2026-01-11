#ifndef __modbus_H
#define __modbus_H

#include "AI8051U.h"
#include "SysConfig.h"

// Modbus registers address
#define DISCRETES_INPUT_START_ADDRESS   0x0000
#define COILS_START_ADDRESS             0x2710
#define INPUT_REG_START_ADDRESS         0x4E20
#define HOLDING_REG_START_ADDRESS       0x7530

// Modbus registers size
#define DISCRETES_INPUT_MAX     100
#define COILS_MAX               100
#define INPUT_REG_MAX           50
#define HOLDING_REG_MAX         50

// Modbus setting
// #define MASTER_TIMEOUT_MS       100


extern uint8_t node_address;
extern uint8_t node_mode;

extern volatile uint8_t xdata discrete_input_register[(DISCRETES_INPUT_MAX + 7) / 8];
extern volatile uint8_t xdata coil_register[(COILS_MAX + 7) / 8];
extern volatile uint16_t xdata input_register[INPUT_REG_MAX];
extern volatile uint16_t xdata holding_register[HOLDING_REG_MAX];



extern uint8_t modbus_RTU_init(uint16_t brt, uint8_t mode, uint8_t addr);   // modbus_RTU初始化
extern uint8_t modbus_mode(void);  //获取当前模式

extern uint8_t xdata *modbus_get_reg_addr(uint16_t reg_addr);  //获取指定本地寄存器地址
extern uint8_t modbus_get_bit_status(uint16_t reg_addr, uint8_t *bit_buf);     //获取指定本地寄存器位
extern uint8_t modbus_write_bit_status(uint16_t reg_addr, uint8_t *bit_buf);   //设置指定本地寄存器位
extern uint16_t modbus_get_reg_value(uint16_t reg_addr, uint8_t *bit_buf);     //获取指定本地寄存器值
extern uint16_t modbus_write_reg_value(uint16_t reg_addr, uint16_t *reg_buf);  //设置指定本地寄存器值

//modbus_RTU 主机程序
extern uint8_t modbus_RTU_read_coils(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint8_t *value, uint32_t timeout);
extern uint8_t modbus_RTU_read_discrete_input(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint8_t *value, uint32_t timeout);
extern uint8_t modbus_RTU_read_holding_regs(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint16_t *RX_value_p, uint32_t timeout);
extern uint8_t modbus_RTU_write_single_coil(uint8_t slave_addr, uint16_t reg_addr, uint16_t value, uint32_t timeout);
extern uint8_t modbus_RTU_write_single_reg(uint8_t slave_addr, uint16_t reg_addr, uint16_t value, uint32_t timeout);
extern uint8_t modbus_RTU_write_multi_coils(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint8_t *Value_p, uint32_t timeout);
extern uint8_t modbus_RTU_write_multi_regs(uint8_t slave_addr, uint16_t reg_addr, uint16_t quantity, uint16_t *Value_p, uint32_t timeout);

//modbus_RTU 从机程序
extern void modbus_RTU_slave(void);

#endif