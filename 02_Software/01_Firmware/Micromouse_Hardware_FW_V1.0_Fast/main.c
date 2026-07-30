/*************************************************************
Project		: Micromouse V2.0 / Hardware Version / Maze solver
File		: main.c
Descrption	: Firmware / Ablauf mittels Statemachine
Date		: 16.01.2025
*************************************************************/

#include "StateMachine.h"


int main(void){
	
	// define start State
    State_t const* state = &State_Initialize;
    state->Enter(); // Set to Enter Mode
	
    // excecute StateMachine
    while(1==1){
        
        StateMachine_Run(state);
    }
	
	return 0;
}
