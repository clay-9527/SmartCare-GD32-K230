#ifndef _BSP_SHT30_H_
#define _BSP_SHT30_H_

#include "gd32f4xx.h"

extern double Temperature, Humidity;


#define u8 unsigned char

void SHT30_GPIO_Init(void);
char SHT30_Read(uint16_t dat);

#endif
