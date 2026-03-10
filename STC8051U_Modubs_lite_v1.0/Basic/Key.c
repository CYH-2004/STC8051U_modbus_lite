
#include "Key.h"

//处理按键程序*************************************************************************************
//仅检测单键，会等待按键松开
void key_scan(void)						//按键处理
{
	uchar i;
	KEY_OK=1;
key_in:
	n_ms(10);
	i=KEY_OK;
	if(i>0)	
	goto key_in;
key_hold:
	n_ms(10);
	i=KEY_OK;
	if(i<1)
	goto key_hold;
}

//延时处理按键程序********************************************************************************
//仅检测单键，等待时间有限
void key_scan1(uint16_t dtime)						//dtime=5000>>>>time<=8s
{
	uchar i;
	uint j;
	KEY_OK=1;
	j=0;
key_in:
	n_ms(10);
	i=KEY_OK;
	if(i>0)
	{
		if(j<dtime)
		{
			j++;
			goto key_in;
		}
	}
key_hold:
	n_ms(10);
	i=KEY_OK;
	if(i<1)
	{
		if(j<dtime)
		{
			j++;
			goto key_hold;
		}
	}
}

//延时检测按键程序********************************************************************************
//编写于2024年寒假，当时不知道IO口可以位操作，结果就照着之前的按键检测用字节操作了
//多键检测
//修改于2024年6月19日，暂未验证，如果出问题可以回旧版本复制回来
uint8_t key_scan2(uint16_t dtime)
{
	uchar i,pin_address;
	uint j;
	j=0;
	pin_address=0;//扫描结果初始化
	KEY_OK=1;
	KEY_UP=1;
	KEY_DOWN=1;

scan_start://端口扫描，在任意按键被按下前重复检测，直到延时结束
	n_ms(10);
	if(j>dtime)
	{
		goto scan_finish;
	}	
	i=KEY_OK;
	if (i<1) 
		{
			pin_address=1;
			goto scan_check;
		}
	i=KEY_DOWN;
	if (i<1) 
		{
			pin_address=2;
			goto scan_check;
		}
	i=KEY_UP;
	if (i<1) 
		{
			pin_address=3;
			goto scan_check;
		}
	if (j<dtime) 
		{
			j++;
			goto scan_start;
		}
	goto scan_finish;
scan_check://检测到按键按下后确认按键松开，直到延时结束
	switch (pin_address)
	{
		case 1:
			n_ms(10);
			i=KEY_OK;
			if(i<1&&j<dtime)
			{
				j++;
				goto scan_check;
			}
			goto scan_start;
			break;
		case 2:
			n_ms(10);
			i=KEY_DOWN;
			if(i<1&&j<dtime)
			{
				j++;
				goto scan_check;
			}
			goto scan_start;
			break;
		case 3:
			n_ms(10);
			i=KEY_UP;
			if(i<1&&j<dtime)
			{
				j++;
				goto scan_check;
			}
			goto scan_start;
			break;
		default://无输入状态
			goto scan_start;//检测确认完成，再次进入检测按键按下
	}
scan_finish://结束检测，返回检测结果
	P1=0x00;
	return pin_address;//无操作时为0，第0脚时为1，第1脚时为2，第2脚时为3
}

//按键检测与反应时间反馈*************************************************************************************
uint16_t key_scan3(uint16_t dtime)						//dtime=5000>>>>time<=8s
{
	uchar i;
	uint j;
	KEY_OK=1;
	j=0;
key_in:
	n_ms(10);
	i=KEY_OK;
	if(i>0)
	{
		if(j<dtime)
		{
			j++;
			goto key_in;
		}
	}
key_hold:
	n_ms(10);
	i=KEY_OK;
	if(i<1)
	{
		if(j<dtime)
		{
			j++;
			goto key_hold;
		}
	}
return j%50;//返回范围缩限
}



//按键等待检测***********************************************************************
//多键检测，不限时
uint16_t key_scan4()
{
	uchar i;
	uint j,pin_address;
	j=0;
	pin_address=0;//扫描结果初始化
	KEY_OK=1;
	KEY_UP=1;
	KEY_DOWN=1;

scan_start://端口扫描，在任意按键被按下前重复检测，直到延时结束
	n_ms(10);
	i=KEY_OK;
	if (i<1) 
		{
			pin_address=1;
			goto scan_check;
		}
	i=KEY_DOWN;
	if (i<1) 
		{
			pin_address=2;
			goto scan_check;
		}
	i=KEY_UP;
	if (i<1) 
		{
			pin_address=3;
			goto scan_check;
		}

	j++;
	goto scan_start;
scan_check://检测到按键按下后确认按键松开，直到延时结束
	switch (pin_address)
	{
		case 1:
			n_ms(10);
			i=KEY_OK;
			if(i<1)
			{
				j++;
				goto scan_check;
			}
			goto scan_finish;
			break;
		case 2:
			n_ms(10);
			i=KEY_DOWN;
			if(i<1)
			{
				j++;
				goto scan_check;
			}
			goto scan_finish;
			break;
		case 3:
			n_ms(10);
			i=KEY_UP;
			if(i<1)
			{
				j++;
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



