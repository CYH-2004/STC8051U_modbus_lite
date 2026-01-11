# STC8051U_modbus_lite

一个基于STC8051U34K64单片机的Modbus_RTU协议实现

## 2026.1.11  v1.0

版本：v1.0
该版本使用单片机UART4作为串口进行通信，固定波特率115200，不使用奇偶校验。

> 当前已支持的Modbus_RTU协议功能包括:
>
> * (0x01) Read Coils
> * (0x02) Read Discrete Inputs
> * (0x03) Read Holding Registers
> * (0x04) Read Input Registers
> * (0x05) Write Single Coil
> * (0x06) Write Single Register
> * (0x0F) Write Multiple Coils
> * (0x10) Write Multiple Registers

演示程序说明：
演示程序使用自制的核心板，UART4引脚使用P0.0(RxD4)、P0.1(TxD4)，显示使用常见的0.96寸OLED屏幕(驱动芯片SSD1306)。
功能为主机端控制从机上灯的亮灭，灯的亮灭状态通过连接至P3.2的按键控制切换，从机端会对灯的亮起次数进行记录，主机可进行读取。
主机程序支持自动断联检测和恢复。
> 演示程序使用的显示屏图形库来自于STC论坛的网友
> 原贴链接<https://www.stcaimcu.com/thread-6246-1-2.html>

演示程序运行流程如下:
1.程序启动后检测连接至P3.2的按键是否按下，按下则进入从机模式，否则进入主机模式，默认从机地址为16;
2.程序启动后初始设置灯熄灭，计数值清零;
3.主机循环读取从机灯状态与计数值，并检测按键是否按下，按下则切换从机灯状态；
4.从机循环处理主机命令，并根据当前灯状态和计数值刷新显示。

> 程序还有很多改进点，当前效果与预期还差很多。但是由于期末了，以后再作说明和修改（其实这个小项目也是为了做现场总线的大作业）
