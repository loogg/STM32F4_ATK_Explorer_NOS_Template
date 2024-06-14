#include <stdint.h>
#include <time.h>
#include <unistd.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <agile_dbg.h>

char buffer[80];

const char *test_get_time(void) {
    time_t rawtime;
    struct tm info;

    time(&rawtime);
    localtime_r(&rawtime, &info);

    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &info);

    return buffer;
}

int main(void) {
    LOG_D("debug test");
    sleep(1);
    LOG_I("info test");
    sleep(1);
    LOG_W("warning test");
    sleep(1);
    LOG_E("error test");

    return 0;
}
