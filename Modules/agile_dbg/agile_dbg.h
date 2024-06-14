/**
 * @file agile_dbg.h
 * @author 马龙伟 (2544047213@qq.com)
 * @brief debug 定义头文件
 * @date 2024-03-12
 *
 @verbatim
    使用:

    #define DBG_TAG "MOD_TAG"
    #define DBG_LVL DBG_INFO
    #include <agile_dbg.h>

 @endverbatim
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __AGILE_DBG_H
#define __AGILE_DBG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "agile_dbg_config.h"

#ifndef AGILE_DBG_LOG_PRINTF
#define AGILE_DBG_LOG_PRINTF(...)
#endif /* AGILE_DBG_LOG_PRINTF */

#ifdef AGILE_DBG_LOG_ENABLE
#ifndef DBG_ENABLE
#define DBG_ENABLE
#endif /* DBG_ENABLE */
#else
#undef DBG_ENABLE
#endif /* AGILE_DBG_LOG_ENABLE */

#ifdef AGILE_DBG_LOG_COLOR
#ifndef DBG_COLOR
#define DBG_COLOR
#endif /* DBG_COLOR */
#endif /* AGILE_DBG_LOG_COLOR */

/* DEBUG level */
#define DBG_ERROR   0
#define DBG_WARNING 1
#define DBG_INFO    2
#define DBG_LOG     3

#ifdef DBG_TAG
#ifndef DBG_SECTION_NAME
#define DBG_SECTION_NAME DBG_TAG
#endif /* DBG_SECTION_NAME */
#else
#ifndef DBG_SECTION_NAME
#define DBG_SECTION_NAME "DBG"
#endif /* DBG_SECTION_NAME */
#endif /* DBG_TAG */

#ifdef DBG_ENABLE

#ifdef DBG_LVL
#ifndef DBG_LEVEL
#define DBG_LEVEL DBG_LVL
#endif /* DBG_LEVEL */
#else
#ifndef DBG_LEVEL
#define DBG_LEVEL DBG_WARNING
#endif /* DBG_LEVEL */
#endif /* DBG_LVL */

/*
 * The color for terminal (foreground)
 * BLACK    30
 * RED      31
 * GREEN    32
 * YELLOW   33
 * BLUE     34
 * PURPLE   35
 * CYAN     36
 * WHITE    37
 */
#ifdef DBG_COLOR
#ifdef AGILE_DBG_LOG_TIME
#define _DBG_LOG_HDR(lvl_name, color_n)                                                 \
    AGILE_DBG_LOG_PRINTF("\033[" #color_n "m[" AGILE_DBG_LOG_TIME_FORMAT "] [" lvl_name \
                         "/" DBG_SECTION_NAME "] ",                                     \
                         AGILE_DBG_LOG_GET_TIME())
#else
#define _DBG_LOG_HDR(lvl_name, color_n) \
    AGILE_DBG_LOG_PRINTF("\033[" #color_n "m[" lvl_name "/" DBG_SECTION_NAME "] ")
#endif /* AGILE_DBG_LOG_TIME */
#define _DBG_LOG_X_END AGILE_DBG_LOG_PRINTF("\033[0m\r\n")
#else
#ifdef AGILE_DBG_LOG_TIME
#define _DBG_LOG_HDR(lvl_name, color_n)                                                          \
    AGILE_DBG_LOG_PRINTF("[" AGILE_DBG_LOG_TIME_FORMAT "] [" lvl_name "/" DBG_SECTION_NAME "] ", \
                         AGILE_DBG_LOG_GET_TIME())
#else
#define _DBG_LOG_HDR(lvl_name, color_n) AGILE_DBG_LOG_PRINTF("[" lvl_name "/" DBG_SECTION_NAME "] ")
#endif /* AGILE_DBG_LOG_TIME */
#define _DBG_LOG_X_END AGILE_DBG_LOG_PRINTF("\r\n")
#endif /* DBG_COLOR */

#define dbg_log_line(lvl, color_n, fmt, ...)      \
    do {                                          \
        _DBG_LOG_HDR(lvl, color_n);               \
        AGILE_DBG_LOG_PRINTF(fmt, ##__VA_ARGS__); \
        _DBG_LOG_X_END;                           \
    } while (0)

#define dbg_raw(...) AGILE_DBG_LOG_PRINTF(__VA_ARGS__);

#else
#define dbg_log_line(lvl, color_n, fmt, ...)
#define dbg_raw(...)
#endif /* DBG_ENABLE */

#if (DBG_LEVEL >= DBG_LOG)
#define LOG_D(fmt, ...) dbg_log_line("D", 0, fmt, ##__VA_ARGS__)
#else
#define LOG_D(...)
#endif

#if (DBG_LEVEL >= DBG_INFO)
#define LOG_I(fmt, ...) dbg_log_line("I", 32, fmt, ##__VA_ARGS__)
#else
#define LOG_I(...)
#endif

#if (DBG_LEVEL >= DBG_WARNING)
#define LOG_W(fmt, ...) dbg_log_line("W", 33, fmt, ##__VA_ARGS__)
#else
#define LOG_W(...)
#endif

#if (DBG_LEVEL >= DBG_ERROR)
#define LOG_E(fmt, ...) dbg_log_line("E", 31, fmt, ##__VA_ARGS__)
#else
#define LOG_E(...)
#endif

#define LOG_RAW(...) dbg_raw(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* __AGILE_DBG_H */
