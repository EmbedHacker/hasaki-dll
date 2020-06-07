#ifndef __DLMODULE_H_
#define __DLMODULE_H_

#define LD_LAZY       0x00000
void *dlopen (const char *filename, int flag);
void *dlsym(void *handle, const char *symbol);

#endif