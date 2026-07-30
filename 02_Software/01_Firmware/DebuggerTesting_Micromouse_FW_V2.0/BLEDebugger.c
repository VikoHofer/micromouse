#include <stdio.h>
#include "BLEDebugger.h"
#include <stdint.h>
#include "Uart.h"
#include <stdbool.h>
#include "Delay.h"
#include <string.h>
#include "system_analysis.h"
#include "motion_control.h"
/* =========================================================
 *  Compiler-portable weak attribute
 * ========================================================= */
#if defined(__ARMCC_VERSION)
  #define WEAK __weak
#elif defined(__GNUC__)
  #define WEAK __attribute__((weak))
#else
  #define WEAK
#endif

/* =========================================================
 *  Protocol direction bit
 *
 *  Bit 7 of the command byte encodes traffic direction:
 *
 *    0xxxxxxx  →  mouse → debugger  (TX, outgoing events)
 *    1xxxxxxx  →  debugger → mouse  (RX, incoming commands)
 *
 *  This means BLEDebugger_Process() can reject every byte
 *  that does NOT have bit 7 set, so echoed TX bytes are
 *  invisible to the RX state machine.
 *
 *  The BLE debugger app must send commands with bit 7 set:
 *    cmd_SetWall_IN = 0x81
 *    cmd_Move_IN    = 0x82
 *    cmd_STEP_IN    = 0x83
 * ========================================================= */
#define CMD_DIR_BIT       0x80u          /* set   = incoming (debugger→mouse) */
#define CMD_IS_INCOMING(b) ((b) & CMD_DIR_BIT)
#define CMD_ID(b)          ((b) & ~CMD_DIR_BIT)
#define RX_MAX_PAYLOAD 16u  // Changed from 8u to 16u


/* =========================================================
 *  TX internals – unchanged
 * ========================================================= */
#define DebugBufferSize 250
static volatile uint8_t DebugBuffer[DebugBufferSize];

static volatile bool     Protocol_IsPrintfCMD = false;
static volatile uint32_t LastPrintfCMD        = 0;
static volatile uint32_t LastUartMsgTS        = 0;

#define PrintfCMDTimeoutMS 1
#define MessageDelay       10

void WriteProtocol(TxCMD_t const Command,
                           uint8_t const *const Data,
                           uint16_t const DataLen)
{
    Protocol_IsPrintfCMD = Command == cmd_printf;

    DebugBuffer[0] = (uint8_t)Command;   /* bit 7 = 0 → outgoing */

    uint16_t ToCopyLen = DataLen;
    if (ToCopyLen > DebugBufferSize - 1) ToCopyLen = DebugBufferSize - 1;

    memcpy((uint8_t *)DebugBuffer + 1, Data, ToCopyLen);

    if ((SysTick_Get() - MessageDelay) <= LastUartMsgTS) Delay_ms(MessageDelay);

    Uart6_SendData((uint8_t *)DebugBuffer, ToCopyLen + 1);
    LastUartMsgTS = SysTick_Get();
}

/* =========================================================
 *  TX public API
 * ========================================================= */
void Debug_SetWall(WallPosition_t const WallPosition)
{
    if (DEACTIVATE_DEBUG) return;
    uint8_t Pos = (uint8_t)WallPosition;
    WriteProtocol(cmd_SetWall, &Pos, 1);
}

void Debug_Move(Move_t const MoveDirection)
{
    if (DEACTIVATE_DEBUG) return;
    uint8_t Direction = (uint8_t)MoveDirection;
    WriteProtocol(cmd_Move, &Direction, 1);
}

/* =========================================================
 *  RX state machine
 *
 *  Key rule: any byte WITHOUT bit 7 set is discarded while
 *  in RX_WAIT_CMD state.  This filters out echoed TX bytes
 *  (which always have bit 7 = 0) before they can corrupt
 *  the RX state machine.
 * ========================================================= */
typedef enum {
    RX_WAIT_CMD,
    RX_WAIT_DATA
} RxState_t;

static RxState_t RxState        = RX_WAIT_CMD;
static RxCMD_t   RxPendingCmd   = rxcmd_STEP;
static uint8_t   RxDataBuf[RX_MAX_PAYLOAD];
static uint8_t   RxDataIdx      = 0;
static uint8_t   RxDataExpected = 0;

static uint8_t ExpectedDataLen(RxCMD_t cmd)
{
    switch (cmd)
    {
        case rxcmd_SetWall: return 1;
        case rxcmd_Move:    return 1;
        case rxcmd_STEP:    return 0;
        case rxcmd_MoveEncoder:    return 16;
        default:            return 0;
    }
}

void BLEDebugger_OnSetWall(WallPosition_t wall);
void BLEDebugger_OnMove(Move_t direction);
void BLEDebugger_OnStep(void);
void BLEDebugger_MoveEncoder(float n_fields, float top_speed, float acceleration, float final_speed);

//static void DispatchCommand(RxCMD_t cmd, uint8_t *data, uint8_t len)
//{
//    switch (cmd)
//    {
//        case rxcmd_SetWall:
//            if (len >= 1) BLEDebugger_OnSetWall((WallPosition_t)data[0]);
//            break;
//        case rxcmd_Move:
//            if (len >= 1) BLEDebugger_OnMove((Move_t)data[0]);
//            break;
//        case rxcmd_STEP:
//            BLEDebugger_OnStep();
//            break;
//        case rxcmd_MoveEncoder:
//						printf("IN MOVE ENCODER CASE\n");
//						if(len >= 4){
//							BLEDebugger_MoveEncoder(data[0],data[1],data[2],data[3]);
//						}
//            break;
//        default:
//            break;
//    }
//}
static void DispatchCommand(RxCMD_t cmd, uint8_t *data, uint8_t len)
{
    switch (cmd)
    {
        case rxcmd_SetWall:
            if (len >= 1) BLEDebugger_OnSetWall((WallPosition_t)data[0]);
            break;
        case rxcmd_Move:
            if (len >= 1) BLEDebugger_OnMove((Move_t)data[0]);
            break;
        case rxcmd_STEP:
            BLEDebugger_OnStep();
            break;
        case rxcmd_MoveEncoder:
            if(len >= 16){
                // Create an array to hold the 4 floats
                float encoder_args[4];
                
                // Safely copy the 16 raw bytes into the float array
                memcpy(encoder_args, data, 16);
                
                // Pass the floats directly to the movement function
                BLEDebugger_MoveEncoder(encoder_args[0], encoder_args[1], encoder_args[2], encoder_args[3]);
            }
            break;
        default:
            break;
    }
}
/**
 * @brief  Parse buffered UART bytes and dispatch complete commands.
 *         Call once per main-loop iteration.
 *
 *         Bytes with bit 7 = 0 are silently discarded while waiting
 *         for a command byte – this prevents echoed TX traffic (printf
 *         output, Debug_SetWall, Debug_Move) from being misinterpreted
 *         as incoming commands.
 *
 *         The BLE debugger app must send command bytes with bit 7 set:
 *           STEP    → 0x83
 *           Move    → 0x82 + 1 payload byte
 *           SetWall → 0x81 + 1 payload byte
 */
void BLEDebugger_Process(void)
{
    int32_t raw;

    while ((raw = Uart6_GetChar()) != NO_DATA_IN_RX_BUFFER)
    {
        uint8_t byte = (uint8_t)raw;
        switch (RxState)
        {
            case RX_WAIT_CMD:
                /* -----------------------------------------------
                 * Discard any byte that is NOT an incoming command
                 * (bit 7 must be set).  This silently swallows all
                 * echoed TX bytes before they cause false triggers.
                 * ----------------------------------------------- */
                if (!CMD_IS_INCOMING(byte)) break;

                RxPendingCmd   = (RxCMD_t)CMD_ID(byte);
                RxDataExpected = ExpectedDataLen(RxPendingCmd);
                RxDataIdx      = 0;

                if (RxDataExpected == 0)
                {
                    DispatchCommand(RxPendingCmd, NULL, 0);
                    /* RxState stays RX_WAIT_CMD */
                }
                else
                {
                    RxState = RX_WAIT_DATA;
                }
                break;

            case RX_WAIT_DATA:
                /* Payload bytes are accepted as-is – no direction check */
                if (RxDataIdx < RX_MAX_PAYLOAD)
                    RxDataBuf[RxDataIdx] = byte;
                RxDataIdx++;

                if (RxDataIdx >= RxDataExpected)
                {
                    DispatchCommand(RxPendingCmd, RxDataBuf, RxDataIdx);
                    RxState = RX_WAIT_CMD;
                }
                break;

            default:
                RxState = RX_WAIT_CMD;
                break;
        }
    }
}

void BLEDebugger_OnSetWall(WallPosition_t const WallPosition)
{
	if(DEACTIVATE_DEBUG) return;
	
	uint8_t Pos = (uint8_t)WallPosition;
	WriteProtocol(cmd_SetWall, &Pos, 1);
}

void BLEDebugger_OnMove(Move_t const MoveDirection)
{
	if(DEACTIVATE_DEBUG) return;
	
	uint8_t Direction = (uint8_t)MoveDirection;
	WriteProtocol(cmd_Move, &Direction, 1);
}

void BLEDebugger_OnStep(void){
	printf("Starting step response...\n"); // ADD THIS LINE

	if(DEACTIVATE_DEBUG) return;
	//
	create_step_respose();
	//printf("Initialize Done!!Micromouse Running on: 180000000 Hz\n");

// HARDWARE FLUSH: Send 20 spaces to force the BLE module's 
// internal UART buffer to overflow and transmit immediately.
}
void BLEDebugger_MoveEncoder(float n_fields, float top_speed, float acceleration, float final_speed)
{
	if(DEACTIVATE_DEBUG) return;
	printf("in BLEDebugger_MoveEncoder");
	printf("n_fields: %.03f, top_speed: %.03f, acceleration: %.03f, final_speed: %.03f, ",n_fields,top_speed,acceleration,final_speed);
	if(!move(n_fields,top_speed,acceleration,final_speed)) 
		printf("Move Encode Failed\n");
	
	
}

/* =========================================================
 *  Keil / ARM Compiler 6 – disable semihosting
 * ========================================================= */
__asm(".global __use_no_semihosting");
void _sys_exit(int return_code)                        { (void)return_code; while (1); }
int  _sys_open(const char *name, int openmode)         { (void)name; (void)openmode; return -1; }

/* =========================================================
 *  Printf redirect (TX only – unchanged)
 * ========================================================= */
static void WritePrintfViaSingleBytes(uint8_t ch)
{
    if (!Protocol_IsPrintfCMD ||
        (SysTick_Get() - PrintfCMDTimeoutMS) > LastPrintfCMD)
    {
        if ((SysTick_Get() - MessageDelay) <= LastUartMsgTS) Delay_ms(MessageDelay);
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

void _ttywrch(int ch)
{
    if (DEACTIVATE_DEBUG) return;
    WritePrintfViaSingleBytes((uint8_t)ch);
}

/* ARM GCC */
int _write(int file, char *ptr, int len)
{
    (void)file;
    if (DEACTIVATE_DEBUG) return len;
    if (ptr == NULL || len <= 0) return 0;
    WriteProtocol(cmd_printf, (uint8_t *)ptr, (uint16_t)len);
    return len;
}
