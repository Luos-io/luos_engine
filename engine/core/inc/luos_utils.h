/******************************************************************************
 * @file luos utils
 * @brief some tools used to debug
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#ifndef LUOS_UTILS_H
#define LUOS_UTILS_H

#include <stdint.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define LUOS_ASSERTION

#if defined(LUOS_ASSERTION)
    #define LUOS_ASSERT(expr) \
        if (!(expr))          \
        Luos_assert((char *)__FILE__, __LINE__)
#else
    #define LUOS_ASSERT(expr)
#endif

#ifndef PUBLIC
    #define PUBLIC
#endif

/* This structure is used to manage node assertion informations
 */
typedef struct __attribute__((__packed__))
{
    union
    {
        struct __attribute__((__packed__))
        {
            uint32_t line;
            char file[100];
        };
        uint8_t unmap[100 + sizeof(uint32_t)];
    };
} luos_assert_t;

/*******************************************************************************
 * Function
 ******************************************************************************/

void Luos_assert(char *file, uint32_t line);
void node_assert(char *file, uint32_t line);
void Luos_JumpToBootloader(void);

#endif /* LUOS_UTILS_H */
