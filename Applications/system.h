#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdio.h>

#define SOFTWARE_VERSION "1.0.1"

#define SYSTEM_USING_CONSOLE
#ifdef SYSTEM_USING_CONSOLE
#define SYSTEM_PRINTF printf
#define SYSTEM_USING_CONSOLE_DEBUG
#define SYSTEM_USING_CONSOLE_DEBUG_COLOR
#define SYSTEM_USING_CONSOLE_SHELL
#else
#define SYSTEM_PRINTF(...)
#endif

#define USB_LOW_USE_FS 0

void system_delay_us(uint32_t us);
int system_init(void);

#endif
