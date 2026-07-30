/*************************************************************
Project		: Micromouse V2.0
File		: config.h
Description	: Collection of all configuration defines and constants
Date		: 15.04.2026
*************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include "stdbool.h"
#include "stdint.h"

// ***** CONFIG **************************************

/* Choose Hardware */
#define HW_VERSION_1   1

#ifndef TARGET_HW
#define TARGET_HW HW_VERSION_1
#endif


#if TARGET_HW == HW_VERSION_1
    #include "config_hw_v1.h"
#endif

/* ----------------------------------------------------------
 * IR wall-following config
 * Separate tuning because semantics differ from ToF
 * ---------------------------------------------------------- */
 
#define MAX_VALUE_WALLS 120
 
 
#define IR_WALL_LEFT_REF            100.0f
#define IR_WALL_RIGHT_REF           100.0f

#define IR_WALL_DEADBAND            3.5f
#define IR_WALL_MAX_ADJUST_DPS      500.0f

#define IR_WALL_VALID_MIN			15.0f
#define IR_WALL_VALID_MAX			102.0f

/*
#define IR_WALL_KP                 6.5f
#define IR_WALL_KD                  1.5f

*/
#define IR_WALL_KP                 	5.0f
#define IR_WALL_KD                  1.5f

#define IR_WALL_FILTER_ALPHA        0.0f

#define IR_DELAY 1

#ifndef IR_OFFSET
#define IR_OFFSET                  0.0f
#endif

#ifndef FRONT_THRESHOLD
#define FRONT_THRESHOLD            180.0f
#endif

#ifndef FRONT_MIDDLE_THRESHOLD
#define FRONT_MIDDLE_THRESHOLD     120.0f
#endif

#ifndef IR_INVALID_VALUE
#define IR_INVALID_VALUE          (-1.0f)
#endif

#ifndef IR_NOMINAL_REFERENCE
#define IR_NOMINAL_REFERENCE       100.0f
#endif

#ifndef IR_CALIB_SAMPLES
#define IR_CALIB_SAMPLES           30U
#endif

#ifndef IR_CALIB_DELAY_MS
#define IR_CALIB_DELAY_MS          10U
#endif

/* Per-sensor filter tuning */
#ifndef IR_ALPHA_FRONT
#define IR_ALPHA_FRONT             0
#endif

#ifndef IR_ALPHA_SIDE
#define IR_ALPHA_SIDE              0
#endif

#ifndef IR_ALPHA_45
#define IR_ALPHA_45                0
#endif



#endif