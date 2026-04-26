#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

uint32_t get_ms(void);
void delay_ms(const uint32_t ms);

void systick_handler(void);

#ifdef __cplusplus
}
#endif

#endif