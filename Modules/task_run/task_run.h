#ifndef __TASK_RUN_H
#define __TASK_RUN_H
#include <stdint.h>

enum task_index {
    TASK_CONSOLE_SHELL,
    TASK_INDEX_MAX
};

struct task_pcb {
    uint8_t enable;
    uint32_t tick_timeout;
    uint32_t period;
    uint32_t event;
    int (*task_entry)(struct task_pcb *task);
    void *user_data;
};

void task_init(enum task_index index, int (*task_entry)(struct task_pcb *task), void *user_data,
               uint32_t period);
void task_start(enum task_index index);
void task_stop(enum task_index index);
int task_event_send(enum task_index index, uint32_t set);
void task_process(void);

#endif
