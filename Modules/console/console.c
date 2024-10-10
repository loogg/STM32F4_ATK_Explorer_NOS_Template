#include "system.h"
#include "console.h"
#include <FreeRTOS.h>
#include <task.h>

#ifdef SYSTEM_USING_CONSOLE_SHELL
#include "chry_ringbuffer.h"
#include "shell.h"

#define SHELL_TASK_PRIO 12
#define SHELL_TASK_STACK_DEPTH 256

extern UART_HandleTypeDef huart1;

static SHELL_TypeDef _shell = {0};
static chry_ringbuffer_t _rx_rb = {0};
static uint8_t _rx_rb_pool[256];
static TaskHandle_t _shell_tid = NULL;

void USART1_IRQHandler(void) {
    if ((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) && (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE) != RESET)) {
        uint8_t ch = (uint8_t)(huart1.Instance->DR & 0x00FF);
        chry_ringbuffer_write_byte(&_rx_rb, ch);
        if (_shell_tid != NULL) {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(_shell_tid, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

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

static signed char _shell_read(char *ch) {
    while (!chry_ringbuffer_read_byte(&_rx_rb, (uint8_t *)ch)) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }

    return 0;
}

static void _shell_write(const char ch) { console_hw_putc(ch); }

#endif /* SYSTEM_USING_CONSOLE_SHELL */

int console_init(void) {
#ifdef SYSTEM_USING_CONSOLE_SHELL
    if (chry_ringbuffer_init(&_rx_rb, _rx_rb_pool, sizeof(_rx_rb_pool)) < 0) return -1;

    _shell.read = _shell_read;
    _shell.write = _shell_write;
    shellInit(&_shell);

    if (xTaskCreate(shellTask, "shell", SHELL_TASK_STACK_DEPTH, &_shell, SHELL_TASK_PRIO, &_shell_tid) != pdPASS) return -1;

    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
#endif /* SYSTEM_USING_CONSOLE_SHELL */

    return 0;
}
