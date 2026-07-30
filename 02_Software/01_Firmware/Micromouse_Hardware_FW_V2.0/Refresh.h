#ifndef REFRESH_H
#define REFRESH_H

#include "Queue.h"


uint8_t Refresh(Point const pos, Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze]);
uint8_t RefreshNew(Point const pos, Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze]);

#endif