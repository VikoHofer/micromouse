#ifndef FLOODFILL_H
#define FLOODFILL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern uint8_t const gSizeMouseMaze; // size of mouseMaze
//zum TESTEN
extern uint8_t const gSizeLabyrinth;
typedef char TMaze[33][33];

extern uint8_t const gPossibleDirections;
enum directions {
    top,
    rightt,
    bottom,
    leftt,
    none
};

typedef struct {
	bool top;
	bool right;
	bool bottom;
	bool left;
	uint8_t wayLength;
}Field;

typedef struct {
	uint8_t x, y;
}Point;

void InitMouseMaze(Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze]);
void SolveMaze(TMaze maze, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze]);
void InitMaze(TMaze maze);
void GetShortestPath(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], Point shortestPath[gSizeMouseMaze * gSizeMouseMaze]);
void TryShortestPath(TMaze maze, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], Point shortestPath[gSizeMouseMaze * gSizeMouseMaze],uint8_t* mouseX, uint8_t* mouseY,enum directions * currentMouseDirection);

#endif