#include "system.h"
#include "console.h"
#include <FreeRTOS.h>
#include <task.h>
#include "shell.h"
#include <string.h>

#define DBG_TAG "system"
#define DBG_LVL DBG_INFO
#include <agile_dbg.h>

#define INIT_TASK_PRIO        10
#define INIT_TASK_STACK_DEPTH 256

extern TIM_HandleTypeDef htim11;

static TaskHandle_t _init_tid = NULL;
static StackType_t _init_tid_stack[INIT_TASK_STACK_DEPTH];
static StaticTask_t _init_tid_tcb;

volatile uint32_t CPU_RunTime = 0UL;

static int cmd_free(int argc, char *agrv[]) {
    size_t total = 0, used = 0, max_used = 0;

    taskENTER_CRITICAL();
    total = configTOTAL_HEAP_SIZE;
    used = total - xPortGetFreeHeapSize();
    max_used = total - xPortGetMinimumEverFreeHeapSize();
    taskEXIT_CRITICAL();

    SYSTEM_PRINTF("total   : %d\r\n", total);
    SYSTEM_PRINTF("used    : %d\r\n", used);
    SYSTEM_PRINTF("maximum : %d\r\n", max_used);

    return 0;
}
SHELL_EXPORT_CMD(free, cmd_free, Show the memory usage in the system.);

static char _task_info_buf[1024];

static int cmd_ps(int argc, char *agrv[]) {
    memset(_task_info_buf, 0, sizeof(_task_info_buf));
    vTaskListTasks(_task_info_buf, sizeof(_task_info_buf));
    SYSTEM_PRINTF("---------------------------------------------\r\n");
    SYSTEM_PRINTF("thread       status     pri   lfstack index\r\n");
    SYSTEM_PRINTF("%s", _task_info_buf);
    SYSTEM_PRINTF("---------------------------------------------\r\n");

    memset(_task_info_buf, 0, sizeof(_task_info_buf));
    vTaskGetRunTimeStatistics(_task_info_buf, sizeof(_task_info_buf));
    SYSTEM_PRINTF("thread         count          percent\r\n");
    SYSTEM_PRINTF("%s", _task_info_buf);
    SYSTEM_PRINTF("---------------------------------------------\r\n\n");

    return 0;
}
SHELL_EXPORT_CMD(ps, cmd_ps, List threads in the system.);

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM11) {
        CPU_RunTime++;
    }
}

void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_TRG_COM_TIM11_IRQn 0 */

  /* USER CODE END TIM1_TRG_COM_TIM11_IRQn 0 */
  HAL_TIM_IRQHandler(&htim11);
  /* USER CODE BEGIN TIM1_TRG_COM_TIM11_IRQn 1 */

  /* USER CODE END TIM1_TRG_COM_TIM11_IRQn 1 */
}

void system_delay_us(uint32_t us) {
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;

    ticks = us * reload / (1000000 / (1000U / uwTickFreq));
    told = SysTick->VAL;
    while (1) {
        tnow = SysTick->VAL;
        if (tnow != told) {
            if (tnow < told) {
                tcnt += told - tnow;
            } else {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks) {
                break;
            }
        }
    }
}

uint32_t HAL_GetTick(void) {
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) HAL_IncTick();

    return uwTick;
}

void HAL_Delay(__IO uint32_t Delay) {
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        vTaskDelay(pdMS_TO_TICKS(Delay));
    } else {
        for (uint32_t count = 0; count < Delay; count++) {
            system_delay_us(1000);
        }
    }
}

extern void xPortSysTickHandler(void);
void SysTick_Handler(void) {
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) HAL_IncTick();

    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
}

static void vRtosShowVersion(void) {
    SYSTEM_PRINTF("\r\n \\    |    /\r\n");
    SYSTEM_PRINTF("- FreeRTOS -     Operating System\r\n");
    SYSTEM_PRINTF(" /    |    \\     %d.%d.%d build %s %s\r\n", tskKERNEL_VERSION_MAJOR, tskKERNEL_VERSION_MINOR, tskKERNEL_VERSION_BUILD, __DATE__,
                  __TIME__);
}

static void init_entry(void *arg) {
    LOG_I("system init start.");

    console_init();

    LOG_I("system init end.");

    vTaskDelete(NULL);
}

int system_init(void) {
    vRtosShowVersion();

    HAL_TIM_Base_Start_IT(&htim11);

    _init_tid = xTaskCreateStatic(init_entry, "init", INIT_TASK_STACK_DEPTH, NULL, INIT_TASK_PRIO, _init_tid_stack, &_init_tid_tcb);

    if (_init_tid == NULL) {
        LOG_E("create init task failed");
        return -1;
    }
    vTaskStartScheduler();

    return 0;
}
