#include "system.h"
#include "console.h"
#include "netif/ethernetif.h"

#define DBG_TAG "system"
#define DBG_LVL DBG_INFO
#include <agile_dbg.h>

extern void lwip_system_init(void);

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

int system_init(void) {
    LOG_I("system init start.");

    console_init();

    lwip_system_init();
    ethernetif_system_init();

    LOG_I("system init end.");

    return 0;
}
