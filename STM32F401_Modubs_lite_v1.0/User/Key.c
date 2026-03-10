

#include "Key.h"

#define uint unsigned short
#define uchar unsigned char


//修改按键引脚（仅限PA，如需修改，请前往Key_Init)
#define KEY_OK_PIN 			GPIO_PIN_1
#define KEY_UP_PIN 			GPIO_PIN_2
#define KEY_DOWN_PIN 		GPIO_PIN_0


/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
	uchar i;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitTypeDef GPIO_key_InitStructure[6] = 
	{  
   		{KEY_DOWN_PIN,	GPIO_MODE_INPUT,	GPIO_PULLUP, 	GPIO_SPEED_FREQ_VERY_HIGH},	// 
    	{KEY_OK_PIN, 	GPIO_MODE_INPUT, 	GPIO_PULLUP,	GPIO_SPEED_FREQ_VERY_HIGH},	//  
		{KEY_UP_PIN, 	GPIO_MODE_INPUT, 	GPIO_PULLUP,	GPIO_SPEED_FREQ_VERY_HIGH},	//   
  	};
	
	for (i = 0; i < 3; i++)
    {  
        HAL_GPIO_Init(GPIOA, &GPIO_key_InitStructure[i]);  
    }  
}



//等待单按键按下*********************************************
void key_scan()
{
	uint8_t io_status;
	HAL_GPIO_WritePin(GPIOA, KEY_OK_PIN, GPIO_PIN_SET);
key_in:
	HAL_Delay(10);
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
	if(io_status>0)
	{
		goto key_in;
	}
key_hold:
	HAL_Delay(10);
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
	if(io_status<1)
	{
		goto key_hold;
	}
}

//延时处理按键程序********************************************************************************
//仅检测单键，等待时间有限
void key_scan1(uint dtime)						//dtime=5000>>>>time<=8s
{
	uchar io_status;
	uint count;
	HAL_GPIO_WritePin(GPIOA, KEY_OK_PIN, GPIO_PIN_SET);
	count=0;
key_in:
	HAL_Delay(10);
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
	if(io_status>0)
	{
		if(count<dtime)
		{
			count++;
			goto key_in;
		}
	}
key_hold:
	HAL_Delay(10);
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
	if(io_status<1)
	{
		if(count<dtime)
		{
			count++;
			goto key_hold;
		}
	}
}

//延时检测按键程序********************************************************************************
//编写于2024年寒假，当时不知道IO口可以位操作，结果就照着之前的按键检测用字节操作了
//多键检测,返回值1=
//修改于2024年6月19日，暂未验证，如果出问题可以回旧版本复制回来
//该子程序已于20240825修改适配stm32f401
uchar key_scan2(uint dtime)
{
	uchar pin_address;
	uchar io_status;
	uint count;
	count=0;
	pin_address=0;//扫描结果初始化
	HAL_GPIO_WritePin(GPIOA, KEY_OK_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, KEY_DOWN_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, KEY_UP_PIN, GPIO_PIN_SET);

scan_start://端口扫描，在任意按键被按下前重复检测，直到延时结束
	HAL_Delay(10);
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
	if(count>dtime)
	{
		goto scan_finish;
	}	
	if (io_status<1) 
		{
			pin_address=1;
			goto scan_check;
		}
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_DOWN_PIN);
	if (io_status<1) 
		{
			pin_address=2;
			goto scan_check;
		}
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_UP_PIN);
	if (io_status<1) 
		{
			pin_address=3;
			goto scan_check;
		}
	if (count<dtime) 
		{
			count++;
			goto scan_start;
		}
	goto scan_finish;
scan_check://检测到按键按下后确认按键松开，直到延时结束
	switch (pin_address)
	{
		case 1:
			HAL_Delay(10);
  			io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
			if(io_status<1&&count<dtime)
			{
				count++;
				goto scan_check;
			}
			goto scan_start;
			break;
		case 2:
			HAL_Delay(10);
			io_status=HAL_GPIO_ReadPin(GPIOA, KEY_DOWN_PIN);
			if(io_status<1&&count<dtime)
			{
				count++;
				goto scan_check;
			}
			goto scan_start;
			break;
		case 3:
			HAL_Delay(10);
			io_status=HAL_GPIO_ReadPin(GPIOA, KEY_UP_PIN);
			if(io_status<1&&count<dtime)
			{
				count++;
				goto scan_check;
			}
			goto scan_start;
			break;
		default://无输入状态
			goto scan_start;//检测确认完成，再次进入检测按键按下
	}
scan_finish://结束检测，返回检测结果
	return pin_address;//无操作时为0，第0脚时为1，第1脚时为2，第2脚时为3
}

//按键检测与反应时间反馈*************************************************************************************
uint key_scan3(uint dtime)						//dtime=5000>>>>time<=8s
{
	uchar io_status;
	uint count;
	HAL_GPIO_WritePin(GPIOA, KEY_OK_PIN, GPIO_PIN_SET);
	count=0;
key_in:
	HAL_Delay(10);
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
	if(io_status>0)
	{
		if(count<dtime)
		{
			count++;
			goto key_in;
		}
	}
key_hold:
	HAL_Delay(10);
  	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
	if(io_status<1)
	{
		if(count<dtime)
		{
			count++;
			goto key_hold;
		}
	}
return count%50;//返回范围缩限
}



//按键等待检测***********************************************************************
//多键检测，不限时
uint key_scan4()
{
	uchar io_status;
	//uint count;
	uint pin_address;
	pin_address=0;//扫描结果初始化

	HAL_GPIO_WritePin(GPIOA, KEY_OK_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, KEY_DOWN_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, KEY_UP_PIN, GPIO_PIN_SET);//端口初始化

scan_start://端口扫描，在任意按键被按下前重复检测，直到延时结束
	HAL_Delay(10);
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
	if (io_status<1) 
		{
			pin_address=1;
			goto scan_check;
		}
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_DOWN_PIN);
	if (io_status<1) 
		{
			pin_address=2;
			goto scan_check;
		}
	io_status=HAL_GPIO_ReadPin(GPIOA, KEY_UP_PIN);
	if (io_status<1) 
		{
			pin_address=3;
			goto scan_check;
		}

	//count++;
	goto scan_start;
scan_check://检测到按键按下后确认按键松开，直到延时结束
	switch (pin_address)
	{
		case 1:
			HAL_Delay(10);
  			io_status=HAL_GPIO_ReadPin(GPIOA, KEY_OK_PIN);
			if(io_status<1)
			{
				//count++;
				goto scan_check;
			}
			goto scan_finish;
			break;
		case 2:
			HAL_Delay(10);
  			io_status=HAL_GPIO_ReadPin(GPIOA, KEY_DOWN_PIN);
			if(io_status<1)
			{
				//count++;
				goto scan_check;
			}
			goto scan_finish;
			break;
		case 3:
			HAL_Delay(10);
  			io_status=HAL_GPIO_ReadPin(GPIOA, KEY_UP_PIN);
			if(io_status<1)
			{
				//count++;
				goto scan_check;
			}
			goto scan_finish;
			break;
		default:
			goto scan_start;//检测确认完成，再次进入检测按键按下
	}
scan_finish://结束检测，返回检测结果
	return pin_address;//无操作时为0，第0脚时为1，第1脚时为2，第2脚时为3
}
