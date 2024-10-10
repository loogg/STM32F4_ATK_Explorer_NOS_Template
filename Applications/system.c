#include "system.h"
#include "console.h"
#include <FreeRTOS.h>
#include <task.h>

#define DBG_TAG "system"
#define DBG_LVL DBG_INFO
#include <agile_dbg.h>

#define INIT_TASK_PRIO        10
#define INIT_TASK_STACK_DEPTH 256

static TaskHandle_t _init_tid = NULL;
static StackType_t _init_tid_stack[INIT_TASK_STACK_DEPTH];
static StaticTask_t _init_tid_tcb;

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

    _init_tid = xTaskCreateStatic(init_entry, "init", INIT_TASK_STACK_DEPTH, NULL, INIT_TASK_PRIO, _init_tid_stack, &_init_tid_tcb);

    if (_init_tid == NULL) {
        LOG_E("create init task failed");
        return -1;
    }
    vTaskStartScheduler();

    return 0;
}
