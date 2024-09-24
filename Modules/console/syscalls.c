#include "system.h"

#ifdef SYSTEM_USING_CONSOLE

#include "main.h"
#include <string.h>

#ifdef __clang__
__asm(".global __use_no_semihosting\n\t");
#else
#pragma import(__use_no_semihosting_swi)
#endif

extern UART_HandleTypeDef huart1;

static void _uart_putc(int ch) {
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);
    huart1.Instance->DR = (uint8_t)ch;
    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);
}

/* This function writes a character to the console. */
void _ttywrch(int ch) { ch = ch; }

/* for exit() and abort() */
void _sys_exit(int x) { x = x; }

FILE __stdout;

#if defined(__GNUC__) && !defined(__clang__)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE {
    _uart_putc(ch);
    return ch;
}

#endif /* SYSTEM_USING_CONSOLE */
