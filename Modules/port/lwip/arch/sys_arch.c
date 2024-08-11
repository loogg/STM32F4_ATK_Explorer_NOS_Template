#include "system.h"
#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/apps/netbiosns.h"
#include "task_run.h"

u32_t sys_now(void) { return HAL_GetTick(); }

#define TASK_RUN_PERIOD 0
static int lwip_sys_check_timeouts_entry(struct task_pcb *task) {
    sys_check_timeouts();

    return 0;
}

void lwip_system_init(void) {
    lwip_init();

    netbiosns_init();

    task_init(TASK_LWIP_SYS_CHECK_TIMEOUTS, lwip_sys_check_timeouts_entry, NULL, TASK_RUN_PERIOD);
    task_start(TASK_LWIP_SYS_CHECK_TIMEOUTS);
}
