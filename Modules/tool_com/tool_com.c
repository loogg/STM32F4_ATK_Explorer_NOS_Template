#include "tool_com.h"
#include "system.h"
#include "task_run.h"
#include "mbtcp_master.h"

#define DBG_TAG "tool_com"
#define DBG_LVL DBG_INFO
#include <agile_dbg.h>

static mbtcp_master_t _master = {0};

#define TASK_RUN_PERIOD 0

static int tool_com_entry(struct task_pcb *task) {
    static uint8_t err_cnt = 0;

    uint16_t hold_regs[10] = {0};
    int status = mbtcp_master_read_registers(&_master, 0, 10, hold_regs, 1000);
    if (status == MBTCP_MASTER_OPT_OK) {
        err_cnt = 0;
        // LOG_I("Hold Registers:");
        // for (int i = 0; i < 10; i++)
        //     LOG_I("Register [%d]: 0x%04X", i, hold_regs[i]);

        // printf("\r\n\r\n\r\n");
    } else if (status == MBTCP_MASTER_OPT_TIMEOUT){
        err_cnt++;
        if (err_cnt > 5) {
            LOG_E("timeout %d times, reconnecting...", err_cnt);
            mbtcp_master_stop(&_master);
            mbtcp_master_start(&_master, "192.168.2.124", 502);
            err_cnt = 0;
        }
    }

    return 0;
}

int tool_com_init(void) {
    mbtcp_master_start(&_master, "192.168.2.124", 502);

    task_init(TASK_TOOL_COM, tool_com_entry, NULL, TASK_RUN_PERIOD);
    task_start(TASK_TOOL_COM);

    return 0;
}
