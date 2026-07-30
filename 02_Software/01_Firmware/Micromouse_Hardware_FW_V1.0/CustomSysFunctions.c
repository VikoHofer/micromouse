#include <stdio.h>
#include "CustomSysFunctions.h"
#include <stdint.h>

//--- KEIL COMPILER ---
// Disable semihosting for ARM Compiler 6 -> otherwise no work in release build
__asm(".global __use_no_semihosting");

void _sys_exit(int return_code)
{
    while (1); // Prevent semihosting exit trap?
}

int fputc(int ch, FILE *f)
{
    Uart6_PutChar((uint8_t)ch);
    return ch;
}

// Required by non-semihosted runtime
int _sys_open(const char *name, int openmode)
{
    return -1; // Not supported
}

void _ttywrch(int ch)
{
    Uart6_PutChar((uint8_t)ch);
}

//--- ARM GCC ---
int _write(int file, char *ptr, int len)
{
    if(ptr == 0 || len <= 0) return 0;

    Uart6_SendData((uint8_t*)ptr, (uint16_t)len);
    return len;
}