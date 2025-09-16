#ifndef __C_LOG_H__
#define __C_LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../../zst_conf.h"

#if (ZST_USE_LOG == 1)

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

    typedef enum
    {
        E_CLOGU = 1, // user 
        E_CLOGE,
        E_CLOGW,
        E_CLOGI,
        E_CLOGD,
        E_CLOGV,
    } LOG_LEVEL_E;

    typedef enum
    {
        CLOG_USER,
        CLOG_NONE,   /*!< No log output */
        CLOG_ERROR,  /*!< Critical errors, software module can not recover on its own */
        CLOG_WARN,   /*!< Error conditions from which recovery measures have been taken */
        CLOG_INFO,   /*!< Information messages which describe normal flow of events */
        CLOG_DEBUG,  /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
        CLOG_VERBOSE /*!< Bigger chunks of debugging information, or frequent messages which can potentially flood the output. */
    } log_level_t;

    void clog_write(log_level_t level, const char *tag, const char *format, ...) __attribute__((format(printf, 3, 4)));

    uint32_t clog_timestamp(void);

#define CLOG_COLOR_BLACK "30"
#define CLOG_COLOR_RED "31"
#define CLOG_COLOR_GREEN "32"
#define CLOG_COLOR_BROWN "33"
#define CLOG_COLOR_BLUE "34"
#define CLOG_COLOR_PURPLE "35"
#define CLOG_COLOR_CYAN "36"
#define CLOG_COLOR(COLOR) "\033[0;" COLOR "m"
#define CLOG_BOLD(COLOR) "\033[1;" COLOR "m"
#define CLOG_RESET_COLOR "\033[0m"
#define CLOG_COLOR_E CLOG_COLOR(CLOG_COLOR_RED)
#define CLOG_COLOR_W CLOG_COLOR(CLOG_COLOR_BROWN)
#define CLOG_COLOR_I CLOG_COLOR(CLOG_COLOR_GREEN)
#define CLOG_COLOR_D
#define CLOG_COLOR_V

#define CLOG_FORMAT(letter, format) CLOG_COLOR_##letter #letter " (%d) %s: " format CLOG_RESET_COLOR "\n"

#define CLOG_LEVEL(level, tag, format, ...)                                                             \
    do                                                                                                  \
    {                                                                                                   \
        if (level == CLOG_ERROR)                                                                        \
        {                                                                                               \
            clog_write(CLOG_ERROR, tag, CLOG_FORMAT(E, format), clog_timestamp(), tag, ##__VA_ARGS__);   \
        }                                                                                               \
        else if (level == CLOG_WARN)                                                                    \
        {                                                                                               \
            clog_write(CLOG_WARN, tag, CLOG_FORMAT(W, format), clog_timestamp(), tag, ##__VA_ARGS__);    \
        }                                                                                               \
        else if (level == CLOG_DEBUG)                                                                   \
        {                                                                                               \
            clog_write(CLOG_DEBUG, tag, CLOG_FORMAT(D, format), clog_timestamp(), tag, ##__VA_ARGS__);   \
        }                                                                                               \
        else if (level == CLOG_VERBOSE)                                                                 \
        {                                                                                               \
            clog_write(CLOG_VERBOSE, tag, CLOG_FORMAT(V, format), clog_timestamp(), tag, ##__VA_ARGS__); \
        }                                                                                               \
        else                                                                                            \
        {                                                                                               \
            clog_write(CLOG_INFO, tag, CLOG_FORMAT(I, format), clog_timestamp(), tag, ##__VA_ARGS__);    \
        }                                                                                               \
    } while (0)

#define CLOGE(tag, format, ...) CLOG_LEVEL(CLOG_ERROR, tag, format, ##__VA_ARGS__)
#define CLOGW(tag, format, ...) CLOG_LEVEL(CLOG_WARN, tag, format, ##__VA_ARGS__)
#define CLOGI(tag, format, ...) CLOG_LEVEL(CLOG_INFO, tag, format, ##__VA_ARGS__)
#define CLOGD(tag, format, ...) CLOG_LEVEL(CLOG_DEBUG, tag, format, ##__VA_ARGS__)
#define CLOGV(tag, format, ...) CLOG_LEVEL(CLOG_VERBOSE, tag, format, ##__VA_ARGS__)

#define DEF_CLOG_DEFAULT_LEVEL ZST_LOG_LEVEL

#define CLOG(level, tag, format, ...)                                                               \
    do                                                                                              \
    {                                                                                               \
        if (level <= DEF_CLOG_DEFAULT_LEVEL)                                                        \
        {                                                                                           \
            switch (level)                                                                          \
            {                                                                                       \
            case E_CLOGV:                                                                           \
                CLOGV(tag, "%s %s (%d): " format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                break;                                                                              \
            case E_CLOGD:                                                                           \
                CLOGD(tag, "%s %s (%d): " format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                break;                                                                              \
            case E_CLOGI:                                                                           \
                CLOGI(tag, "%s %s (%d): " format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                break;                                                                              \
            case E_CLOGW:                                                                           \
                CLOGW(tag, "%s %s (%d): " format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                break;                                                                              \
            case E_CLOGE:                                                                           \
                CLOGE(tag, "%s %s (%d): " format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                break;                                                                              \
            case E_CLOGU:                                                                           \
                CLOGD(tag, format, ##__VA_ARGS__);                                                   \
                break;                                                                              \
            default:                                                                                \
                CLOGI(tag, "%s %s (%d): " format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
                break;                                                                              \
            }                                                                                       \
        }                                                                                           \
    } while (0)

#define ZST_LOG(format, ...)       CLOG(E_CLOGU, "", format, ##__VA_ARGS__)
#define ZST_LOGE(tag, format, ...) CLOG(CLOG_ERROR, tag, format, ##__VA_ARGS__)
#define ZST_LOGW(tag, format, ...) CLOG(CLOG_WARN, tag, format, ##__VA_ARGS__)
#define ZST_LOGI(tag, format, ...) CLOG(CLOG_INFO, tag, format, ##__VA_ARGS__)
#define ZST_LOGD(tag, format, ...) CLOG(CLOG_DEBUG, tag, format, ##__VA_ARGS__)
#define ZST_LOGV(tag, format, ...) CLOG(CLOG_VERBOSE, tag, format, ##__VA_ARGS__)

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif // !__C_LOG_H__
