## Test program analysis

Now that the shared library is designed, there is also the dlopen interface to load the shared library. Next, directly load the dynamic library in the test program and find the required function to use.
As shown below:

```
1: int main(int argv,char **argc)
2:  {
3: void* handle;
4: int result;
5: typedef int (*func_t)(int, int);
6:
7: func_t add_function;
8: func_t dec_function;
9:      
10: handle = dlopen(argc[1],LD_LAZY);
11:
12: if(!handle)
13: {
14: printf("dlopen %s failed!\n",argc[1]);
15: return -1;
16:}
17:
18: printf("\r\n\r\nTest dynamic module functions :>>>>\n");
19: printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
20:
21: add_function = (func_t)dlsym(handle,"add_func");
22: if(!add_function)
twenty three:      {
24: printf("dlsym %p failed!\n",handle);
25: return -1;
26:}
27: result = add_function(1000,24);
28:
29: printf("add_function result is:%d\n",result);
30:
31: dec_function = (func_t)dlsym(handle,"dec_func");
32: if(!dec_function)
33: {
34: printf("dlsym %p failed!\n",handle);
35: return -1;
36:}
37: result = dec_function(170,2);
38:
39: printf("dec_function result is:%d\n",result);
40: printf("-------------------------------------\n");
41: printf("Congratulations, your test passed!!\r\n\r\n");
42: return 0;
43:}
```
The test code is very simple, let's analyze it for everyone:

1. Line 10, using the dlopen interface, load the shared library file with the path specified by the first parameter, and return the dynamic linker object,
From the analysis of "The Principle Analysis of Dynamically Loaded Shared Libraries", it can be seen that the dynamic linker object has recorded the virtual memory location of the shared library at this time
Also got a pointer to the function table/symbol table

2. Lines 21 to 26, use the dlsym function to find the add_func function in the dynamic library, the function pointer has been recorded on the first loading section of the shared library,
At this point, the shared library has been loaded into virtual memory and recorded in the dynamic linker object. According to the function table/symbol table on the dynamic connector, after finding that the corresponding character string of the function is the same, you can obtain the corresponding function pointer.

3. Line 27, call the found function.

4. Lines 28~37, ibid.