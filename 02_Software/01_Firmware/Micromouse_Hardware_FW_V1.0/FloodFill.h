#ifndef FLOODFILL_H
#define FLOODFILL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "config.h"
#include "sensor_normalize.h"

enum directions {
    top,
    rightt,
    bottom,
    leftt,
    none
};

typedef struct {
	uint8_t x, y;
}Point;

typedef struct {
	bool top;
	bool right;
	bool bottom;
	bool left;
	uint8_t wayLength;
}Field;

extern uint8_t const gSizeMouseMaze; // size of mouseMaze
//zum TESTEN
extern uint8_t const gSizeLabyrinth;
extern Field mouseMaze[16][16];
typedef char TMaze[33][33];
extern TMaze maze;

extern uint8_t const gPossibleDirections;

void InitMouseMaze(Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze]);
void SolveMaze(TMaze maze, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze]);
void InitMaze(TMaze maze);
void GetShortestPath(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], Point shortestPath[gSizeMouseMaze * gSizeMouseMaze]);
uint8_t TryShortestPath(TMaze maze, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], Point shortestPath[gSizeMouseMaze * gSizeMouseMaze],uint8_t* mouseX, uint8_t* mouseY,enum directions * currentMouseDirection);

#endif