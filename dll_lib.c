#include "lib-support.h"

int add_func(int a, int b)
{
    return (a + b);
}

int dec_func(int a, int b)
{
    return (a - b);
}

void *func_table[] = {
    add_func, dec_func
};

char *symtab[] = {
    "add_func","dec_func"
}

MDL_DEFINE_HEADER(func_table,symtab,sizeof(func_table)/sizeof(func_table[0]));
