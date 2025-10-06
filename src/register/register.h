#ifndef __REGISTER_H__
#define __REGISTER_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"
#if (ZST_USE_REGISTER == 1)

typedef struct _register_object_
{
#if (ZST_REGISTER_ADDR16 == 1)
    union 
    {
        struct 
        {
            uint8_t addr1;
            uint8_t addr2;
        };
        uint16_t addr;
    };
#else
    uint8_t addr;
#endif
#if (SZT_TRGISTER_USE_NAME == 1)
    char * name;
#endif
    uint8_t * buffer;
    uint16_t size;
    struct _register_object_ * next;
} register_object_t;


// typedef struct 
// {
// #if (SZT_TRGISTER_USE_NAME == 1)
//     char * name;
// #endif
//     register_object_t * header;


// } register_t;

// ==========================================

// struct
// {
//     char * name;
    
// } reg;





#ifdef __cplusplus
}
#endif
#endif /* ZST_USE_REGISTER  */
#endif /* __REGISTER_H__ */

