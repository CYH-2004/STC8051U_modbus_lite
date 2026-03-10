/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "key.h"
#include "oled12824_iic.h"
#include "modbus.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern uint8_t modbus_rx_standby;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void modbus_connect_lost(void)
{
    // OLED_GRAM_Clear(0);
    // if(modbus_mode())   //主机模式
    // {
    //     OLED_ShowStringG(0, 0, "Master mode", 16, 1);
    // }
    // else  //从机模式
    // {
    //     OLED_ShowStringG(0, 0, "Slave mode", 16, 1);
    // }
    // OLED_ShowStringG(0, 3, "Reconnecting...", 16, 0);
    // OLED_Refresh_Gram();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  uint8_t light_status;
  uint8_t light_status_temp;
  uint16_t counter;

  uint8_t slave_address;
  uint16_t slave_light_coil_addr;
  uint16_t slave_counter_reg_addr;

  volatile uint8_t slave_status;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */

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
      PrintString1("UART1: Enter modbus slave mode!\r\n");
      // PrintString2("UART2: Enter modbus slave mode!\r\n");
      modbus_RTU_init(0, slave_address);

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
          OLED_ShowNumG(45, 6, counter, 3, 16, 0);

          OLED_ShowStringG(0, 0, "Slave", 16, 1);
          OLED_ShowStringG(0, 2, "Addr:", 16, 0);
          OLED_ShowNumG(45, 2, slave_address, 3, 16, 0);
          OLED_Refresh_Gram();
      }
  }
  else    //主机模式
  {
      PrintString1("UART1: Enter modbus master mode!\r\n");
      // PrintString2("UART2: Enter modbus master mode!\r\n");
      modbus_RTU_init(1, slave_address);

      OLED_GRAM_Clear(0);
      OLED_ShowStringG(0, 0, "Master", 16, 0);
      OLED_ShowStringG(0, 3, "Connecting...", 16, 0);
      OLED_Refresh_Gram();

      // while(1)
      // {
      //   modbus_RTU_write_single_coil(slave_address, slave_light_coil_addr, 0, 1000);
      //   key_scan();
      // }

      //熄灯并清除从机计数器
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
          if(key_scan2(50)) 
          {
              while(modbus_RTU_read_coils(slave_address, slave_light_coil_addr, 1, &light_status, 300)) modbus_connect_lost();
              light_status = !light_status;
              while(modbus_RTU_write_single_coil(slave_address, slave_light_coil_addr, light_status, 300)) modbus_connect_lost();
              while(modbus_RTU_read_holding_regs(slave_address, slave_counter_reg_addr, 1, &counter, 300)) modbus_connect_lost();
          }
      }
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
