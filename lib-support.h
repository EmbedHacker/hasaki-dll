#ifndef _SHARED_H_
#define _SHARED_H_

#include <stdint.h>

#define STR(...)  #__VA_ARGS__
#define XSTR(...) STR(__VA_ARGS__)
#define _ES_HASH  #
#define ES_HASH() _ES_HASH
#ifndef ELFW
#define ELFW(type) _ELFW(__ELF_NATIVE_CLASS, type)
#define _ELFW(bits, type) __ELFW(bits, type)
#define __ELFW(bits, type) ELF##bits##_##type
#endif

#if defined(__x86_64__)
#include "arch/x86_64.h"
#elif defined(__arm__)
#include "arch/arm.h"
#elif defined(__aarch64__)
#include "arch/aarch64.h"
#else
#error "Unsupported architecture"
#endif

/*
 * Every architecture needs to define its own assembly macro with
 * prefix '_' in arch/, and matches all asm() blocks where the macro 
 * will be expanded.
 */
#define PUSH_S(x)        XSTR(_PUSH_S(x))
#define PUSH(x,y)        XSTR(_PUSH(x,y))
#define PUSH_IMM(x)      XSTR(_PUSH_IMM(x))
#define PUSH_STACK_STATE XSTR(_PUSH_STACK_STATE)
#define JMP_S(x)         XSTR(_JMP_S(x))
#define JMP_REG(x)       XSTR(_JMP_REG(x))
#define JMP(x,y)         XSTR(_JMP(x,y))
#define POP_S(x)         XSTR(_POP_S(x))
#define POP(x,y)         XSTR(_POP(x,y))
#define POP_STACK_STATE  XSTR(_POP_STACK_STATE)
#define CALL(x)          XSTR(_CALL(x))

typedef void *(*plt_resolver_t)(void *handle, int import_id);

struct program_header {
    size_t num;
    char** symtab;
    void **user_info;
};

typedef struct __DLoader_Internal *dloader_p;
extern struct __DLoader_API__ {
    dloader_p (*load)(const char *filename);
    void *(*get_info)(dloader_p);
} DLoader;

#define MDL_DEFINE_HEADER(user_info_value,user_info_symtab,size); \
struct program_header PROG_HEADER = {                 \
    .num = size,                                      \
    .symtab = user_info_symtab,                       \
    .user_info = user_info_value,  		      	      \
};

#endif
