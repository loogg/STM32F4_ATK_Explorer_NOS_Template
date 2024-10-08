#include "system.h"

#ifdef SYSTEM_USING_CONSOLE

#include "main.h"
#include <stdio.h>

#ifdef __clang__
__asm(".global __use_no_semihosting\n\t");
#else
#pragma import(__use_no_semihosting_swi)
#endif

extern UART_HandleTypeDef huart1;

// 标准库需要的支持函数
struct __FILE {
    int handle;
};
FILE __stdout;

// 定义 _sys_exit() 以避免使用半主机模式
void _sys_exit(int x) { x = x; }

// 重定义 fputc 函数
int fputc(int ch, FILE *f) {
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);
    huart1.Instance->DR = (uint8_t)ch;
    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);

    return ch;
}

#endif /* SYSTEM_USING_CONSOLE */
