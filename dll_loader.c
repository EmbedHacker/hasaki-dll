#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "lib-support.h"
#include "dlmodule.h"

int main(int argv,char **argc)
{
    void* handle;
    int result;
    typedef int (*func_t)(int, int);

    func_t add_function;
    func_t dec_function;
    
    handle = dlopen(argc[1],LD_LAZY);

     if(!handle)
    {
        printf("dlopen %s failed!\n",argc[1]);
        return -1;
    }

    printf("\r\n\r\nTest dynamic module functions :>>>>\n");
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    add_function  =  (func_t)dlsym(handle,"add_func");
    if(!add_function)
    {
        printf("dlsym %p failed!\n",handle);
        return -1;
    }
    result = add_function(1000,24);

    printf("add_function result is:%d\n",result);

    dec_function  =  (func_t)dlsym(handle,"dec_func");
    if(!dec_function)
    {
        printf("dlsym %p failed!\n",handle);
        return -1;
    }
    result = dec_function(170,2);

    printf("dec_function result is:%d\n",result);
    printf("-------------------------------------\n");
    printf("Congratulations, your test passed!!\r\n\r\n");
    return 0;
}
