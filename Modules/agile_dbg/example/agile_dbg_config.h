/**
 * @file agile_dbg_config.h
 * @author 马龙伟 (2544047213@qq.com)
 * @brief debug 配置头文件
 * @date 2024-03-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __AGILE_DBG_CONFIG_H
#define __AGILE_DBG_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

const char *test_get_time(void);

#define AGILE_DBG_LOG_PRINTF printf

#define AGILE_DBG_LOG_ENABLE
#define AGILE_DBG_LOG_COLOR

#define AGILE_DBG_LOG_TIME
#define AGILE_DBG_LOG_TIME_FORMAT "%s"
#define AGILE_DBG_LOG_GET_TIME() test_get_time()

#ifdef __cplusplus
}
#endif

#endif /* __AGILE_DBG_CONFIG_H */
