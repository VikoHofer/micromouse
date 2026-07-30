#ifndef CUSTOMSYSFUNCTIONS_H
#define CUSTOMSYSFUNCTIONS_H
#include <stdio.h>
#include "Uart.h"

/*
    --- Custom System Functions ---
    _write called by printf when compiled with ARM GCC
    _fputc called by printf when compiled with Keil ARMCC/uvision
*/

int _write(int file, char *ptr, int len);
int fputc(int ch, FILE *f);

#endif