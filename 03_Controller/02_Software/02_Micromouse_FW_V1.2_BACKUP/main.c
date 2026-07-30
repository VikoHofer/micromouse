/*************************************************************
Project		: Micromouse V1.1 / Maze solver
File		: main.c
Descrption	: Firmware / Ablauf mittels Statemachine
Date		: 09.09.2024
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
