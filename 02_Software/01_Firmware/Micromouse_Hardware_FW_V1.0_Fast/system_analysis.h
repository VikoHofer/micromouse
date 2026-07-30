/*************************************************************
Project		: Micromouse V2.0
File		: system_analysis.h
Description	: Functionality to make controlled motion on the mouse
Date		: 11.03.2026
*************************************************************/

#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
* SYSTEM ANALYSE
*******************************************************************************
* Create step response of the mouse
* 1. Button Press -> Drive and sample data
* 2. Button Press -> Send data via PY Debugger
*******************************************************************************/
void create_step_respose(void);
void create_step_angle_vel(void);



#endif