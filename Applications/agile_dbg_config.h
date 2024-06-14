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
#include "system.h"


#define AGILE_DBG_LOG_PRINTF SYSTEM_PRINTF

#ifdef SYSTEM_USING_CONSOLE_DEBUG
#define AGILE_DBG_LOG_ENABLE
#endif

#ifdef SYSTEM_USING_CONSOLE_DEBUG_COLOR
#define AGILE_DBG_LOG_COLOR
#endif

#define AGILE_DBG_LOG_TIME
#define AGILE_DBG_LOG_TIME_FORMAT "%u"
#define AGILE_DBG_LOG_GET_TIME() HAL_GetTick()

#ifdef __cplusplus
}
#endif

#endif /* __AGILE_DBG_CONFIG_H */
