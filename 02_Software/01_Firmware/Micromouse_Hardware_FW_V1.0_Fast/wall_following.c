#include "wall_following.h"
#include "config.h"

#include "wall_following_ir.h"

void WallFollowing_Init(void)
{
    WallFollowing_IR_Init();
}

float WallFollowing_GetCorrectionDps(float velocity_mms)
{
    return WallFollowing_IR_GetCorrectionDps(velocity_mms);
}