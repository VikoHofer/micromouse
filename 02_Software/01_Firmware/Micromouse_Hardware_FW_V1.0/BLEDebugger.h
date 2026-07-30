#ifndef BLEDEBUGGER_H
#define BLEDEBUGGER_H

#define DEACTIVATE_DEBUG false

/*
	Debug functions for debugging with Micro Mouse BLE Debugger
	
	- use printf(..) for text based debug messages
	
	protocol is defined 02_Software/03_DebugDisplay/BLEDebugDisplay/Protocol.md
*/

typedef enum {
	WallPos_Top = 0,
	WallPos_Left = 1,
	WallPos_Right = 2,
	WallPos_Bottom = 3
} WallPosition_t;

//Signals the debugger that a wall has been detected at given position.
void Debug_SetWall(WallPosition_t const WallPosition);

typedef enum {
	Move_Straight = 0,
	Move_Left = 1,
	Move_Right = 2
} Move_t;

//Signals the debugger that the mouse moved in given direction.
void Debug_Move(Move_t const MoveDirection);


// --- functions to redirect printf to uart ---

/*
    --- Custom System Functions ---
    _write called by printf when compiled with ARM GCC
    _fputc called by printf when compiled with Keil ARMCC/uvision
*/
#include <stdio.h>
#include "Uart.h"

int _write(int file, char *ptr, int len);
int fputc(int ch, FILE *f);


#endif