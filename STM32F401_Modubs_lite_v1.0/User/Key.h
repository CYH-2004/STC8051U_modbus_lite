#ifndef __KEY_H
#define __KEY_H

#include "main.h"

#define uchar unsigned char
#define uint unsigned short

void Key_Init(void);
void key_scan(void);
void key_scan1(uint dtime);
uchar key_scan2(uint dtime);

#endif
