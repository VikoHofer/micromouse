/*************************************************************
Project		: Micromouse V1.1
File		: StateMachine.c
Description	: Statemachine for the Micromouse
Date		: 29.10.2024
*************************************************************/

#include "StateMachine.h"


/*********************************************************
STATEMACHINE RUN
*********************************************************/
void StateMachine_Run(State_t const * state)
{
    if(state != NULL)
    {
        while(1)
        {
           State_t const* newState = state->Process();
            if(state!= newState)
            {
                state->Exit();
                newState->Enter();
                state = newState;
            }
			
        }
        
    }
}
