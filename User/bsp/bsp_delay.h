#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern void delay_init(void);
extern void delay_us(uint16_t nus);
extern void delay_ms(uint16_t nms);

#ifdef __cplusplus
}
#endif


