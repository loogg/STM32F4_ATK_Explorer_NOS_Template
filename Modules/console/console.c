#include "system.h"
#include "console.h"

#ifdef SYSTEM_USING_CONSOLE_SHELL
#include "chry_ringbuffer.h"
#include "shell.h"
#include "task_run.h"

extern UART_HandleTypeDef huart1;

static SHELL_TypeDef _shell = {0};
static chry_ringbuffer_t _rx_rb = {0};
static uint8_t _rx_rb_pool[256];

static void console_rx_handler(uint8_t *buf, uint32_t len) {
    if (len > 0) {
        if (!chry_ringbuffer_check_full(&_rx_rb)) {
            chry_ringbuffer_write(&_rx_rb, buf, len);
        }
    }
}

void USART1_IRQHandler(void) {
    if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE) != RESET)) {
        uint8_t ch = (uint8_t)(huart1.Instance->DR & 0x00FF);
        console_rx_handler(&ch, 1);

        __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
    } else {
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE) != RESET) {
            __HAL_UART_CLEAR_OREFLAG(&huart1);
        }
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_NE) != RESET) {
            __HAL_UART_CLEAR_NEFLAG(&huart1);
        }
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_FE) != RESET) {
            __HAL_UART_CLEAR_FEFLAG(&huart1);
        }
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_PE) != RESET) {
            __HAL_UART_CLEAR_PEFLAG(&huart1);
        }

        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_LBD) != RESET) {
            __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_LBD);
        }
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_CTS) != RESET) {
            __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_CTS);
        }
    }
}

static void console_hw_putc(char c) {
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);
    huart1.Instance->DR = (uint8_t)c;
    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);
}

static signed char _shell_read(char *ch) { return (chry_ringbuffer_read_byte(&_rx_rb, (uint8_t *)ch) ? 0 : -1); }

static void _shell_write(const char ch) { console_hw_putc(ch); }

#define TASK_RUN_PERIOD 0

static int console_shell_entry(struct task_pcb *task) {
    shellTask(&_shell);

    return 0;
}

#endif /* SYSTEM_USING_CONSOLE_SHELL */

int console_init(void) {
#ifdef SYSTEM_USING_CONSOLE_SHELL
    if (chry_ringbuffer_init(&_rx_rb, _rx_rb_pool, sizeof(_rx_rb_pool)) < 0) return -1;

    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);

    _shell.read = _shell_read;
    _shell.write = _shell_write;
    shellInit(&_shell);

    task_init(TASK_CONSOLE_SHELL, console_shell_entry, NULL, TASK_RUN_PERIOD);
    task_start(TASK_CONSOLE_SHELL);
#endif /* SYSTEM_USING_CONSOLE_SHELL */

    return 0;
}
