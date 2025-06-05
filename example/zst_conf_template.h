#ifndef __ZST_CONF_H__
#define __ZST_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"


#define ZST_USE_ALLOC    1
#define ZST_TICK_CUSTOM  1
#define ZST_USE_LOG      0
#define ZST_USE_TIMER    0
#define ZST_USE_EVENT    0





#if ZST_USE_ALLOC
    #define ZST_MEM_CUSTOM_INCLUDE     "stdlib.h"
    #define ZST_MEM_CUSTOM_ALLOC(x)     malloc(x)
    #define ZST_MEM_CUSTOM_FREE(x)      free(x)
    #define ZST_MEM_CUSTOM_REALLOC(x,y) realloc(x,y)
#endif

/*Use a custom tick source that tells the elapsed time in milliseconds.
 *It removes the need to manually update the tick with `zst_tick_inc()`).
*/
#if ZST_TICK_CUSTOM
    #define ZST_TICK_CUSTOM_INCLUDE     "Arduino.h"
    #define ZST_TICK_CUSTOM_SYS_TIME    (millis())
#endif


#if ZST_USE_LOG
    // 0 - 5
    #define ZST_LOG_LEVEL  5
#endif

/**
 * public events
 */
#if (ZST_USE_EVENT == 1)

    #define ZST_EVENT_PUBLIC ZST_EVENTR_0,\
                             ZST_EVENTR_1
#endif


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __ZST_CONF_H__ */
