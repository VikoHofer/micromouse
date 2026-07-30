#ifndef BLEDEBUGGER_H
#define BLEDEBUGGER_H

#include <stdint.h>

/* Set to 1 to strip all debug output at compile time */
#ifndef DEACTIVATE_DEBUG
  #define DEACTIVATE_DEBUG 0
#endif

typedef enum {
	WallPos_Top = 0,
	WallPos_Left = 1,
	WallPos_Right = 2,
	WallPos_Bottom = 3
} WallPosition_t;

typedef enum {
	Move_Straight = 0,
	Move_Left = 1,
	Move_Right = 2
} Move_t;
/* =========================================================
 *  Wire protocol – command byte layout
 *
 *  Bit 7 encodes traffic direction:
 *
 *    Bit 7 = 0  →  mouse → debugger  (outgoing events/prints)
 *    Bit 7 = 1  →  debugger → mouse  (incoming commands)
 *
 *  Outgoing (mouse → debugger):        Incoming (debugger → mouse):
 *    0x00  printf text                   0x81  SetWall  + 1 byte payload
 *    0x01  SetWall event + 1 byte        0x82  Move     + 1 byte payload
 *    0x02  Move event    + 1 byte        0x83  STEP     (no payload)
 *
 *  Because all outgoing bytes have bit 7 = 0, any echo of TX
 *  traffic on the RX line is automatically ignored by the
 *  receiver state machine.
 * ========================================================= */
/* =========================================================
 *  TX command IDs  (mouse → debugger, bit 7 = 0)
 * ========================================================= */
typedef enum {
	cmd_printf  = 0x00,
	cmd_SetWall = 0x01,
	cmd_Move    = 0x02,
	cmd_STEP    = 0x03,
	cmd_MoveEncoder = 0x04
} TxCMD_t;

/* =========================================================
 *  RX command IDs  (debugger → mouse, bit 7 = 1)
 *  Strip the direction bit to get the logical command ID,
 *  which intentionally matches the TX enum for symmetry.
 * ========================================================= */
typedef enum {
	rxcmd_SetWall = 0x01,   /* wire byte: 0x81 */
	rxcmd_Move    = 0x02,   /* wire byte: 0x82 */
	rxcmd_STEP    = 0x03,    /* wire byte: 0x83 */
	rxcmd_MoveEncoder    = 0x04   /* wire byte: 0x83 */
} RxCMD_t;
/* =========================================================
 *  TX API  (mouse → debugger)
 * ========================================================= */
void Debug_SetWall(WallPosition_t WallPosition);
void Debug_Move(Move_t MoveDirection);
void WriteProtocol(TxCMD_t const Command,
                           uint8_t const *const Data,
                           uint16_t const DataLen);
/* =========================================================
 *  RX API  (debugger → mouse)
 * ========================================================= */

/**
 * @brief  Parse buffered UART bytes and dispatch complete commands.
 *         Call once per main-loop iteration:
 *
 *           while (1) {
 *               BLEDebugger_Process();
 *               ...
 *           }
 */
void BLEDebugger_Process(void);

/* =========================================================
 *  Command handlers  (weak no-ops – override in your app)
 *
 *  Example:
 *    // in maze.c
 *    void BLEDebugger_OnStep(void)             { Mouse_StepOnce(); }
 *    void BLEDebugger_OnMove(Move_t dir)       { Mouse_ExecuteMove(dir); }
 *    void BLEDebugger_OnSetWall(WallPosition_t w) { Maze_MarkWall(w); }
 * ========================================================= */
void BLEDebugger_OnSetWall(WallPosition_t wall);
void BLEDebugger_OnMove(Move_t direction);
void BLEDebugger_OnStep(void);

#endif /* BLEDEBUGGER_H */
