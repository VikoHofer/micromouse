#include <stdio.h>
#include "BLEDebugger.h"
#include <stdint.h>
#include "Uart.h"
#include <stdbool.h>
#include "Delay.h"
#include <string.h>

#define DebugBufferSize 250
static volatile uint8_t DebugBuffer[DebugBufferSize];

typedef enum {
	cmd_printf = 0,
	cmd_SetWall = 1,
	cmd_Move = 2
} ProtocolCMD_t;

static volatile bool Protocol_IsPrintfCMD = false;
static volatile uint32_t LastPrintfCMD = 0;
static volatile uint32_t LastUartMsgTS = 0;
#define PrintfCMDTimeoutMS 1
#define MessageDelay 10

static void WriteProtocol(ProtocolCMD_t const Command, uint8_t const*const Data, uint16_t const DataLen)
{	
	Protocol_IsPrintfCMD = Command == cmd_printf;
	
	DebugBuffer[0] = Command;
	
	uint16_t ToCopyLen = DataLen;
	if(ToCopyLen > DebugBufferSize - 1) ToCopyLen = DebugBufferSize - 1;
	
	memcpy((uint8_t*)DebugBuffer + 1, Data, ToCopyLen);
	
	if((SysTick_Get() - MessageDelay) <= LastUartMsgTS) Delay_ms(MessageDelay);
	
	Uart6_SendData((uint8_t*)DebugBuffer, ToCopyLen + 1);
	LastUartMsgTS = SysTick_Get();
}

void Debug_SetWall(WallPosition_t const WallPosition)
{
	if(DEACTIVATE_DEBUG) return;
	
	uint8_t Pos = (uint8_t)WallPosition;
	WriteProtocol(cmd_SetWall, &Pos, 1);
}

void Debug_Move(Move_t const MoveDirection)
{
	if(DEACTIVATE_DEBUG) return;
	
	uint8_t Direction = (uint8_t)MoveDirection;
	WriteProtocol(cmd_Move, &Direction, 1);
}

//--- KEIL COMPILER ---
// Disable semihosting for ARM Compiler 6 -> otherwise no work in release build
__asm(".global __use_no_semihosting");

void _sys_exit(int return_code)
{
    while (1); // Prevent semihosting exit trap?
}

static void WritePrintfViaSingleBytes(uint8_t ch)
{
	if(!Protocol_IsPrintfCMD || (SysTick_Get() - PrintfCMDTimeoutMS) > LastPrintfCMD)
	{
		if((SysTick_Get() - MessageDelay) <= LastUartMsgTS) Delay_ms(MessageDelay);
		
		Uart6_PutChar(cmd_printf);
		Protocol_IsPrintfCMD = true;
	}
	
	Uart6_PutChar(ch);
	LastPrintfCMD = SysTick_Get();
	LastUartMsgTS = LastPrintfCMD;
}

int fputc(int ch, FILE *f)
{
	if(DEACTIVATE_DEBUG) return ch;
	
	WritePrintfViaSingleBytes((uint8_t)ch);
	return ch;
}

// Required by non-semihosted runtime
int _sys_open(const char *name, int openmode)
{
    return -1; // Not supported
}

void _ttywrch(int ch)
{
	if(DEACTIVATE_DEBUG) return;
	WritePrintfViaSingleBytes((uint8_t)ch);
}

//--- ARM GCC ---
int _write(int file, char *ptr, int len)
{
	  if(DEACTIVATE_DEBUG) return len;
    if(ptr == 0 || len <= 0) return 0;

    WriteProtocol(cmd_printf, (uint8_t*)ptr, (uint16_t)len);
    return len;
}