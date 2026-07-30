#ifndef DIAGONAL_H
#define DIAGONAL_H

#include "FloodFill.h"

typedef enum {
    CURVE_NONE = 0,
    CURVE_LEFT_90_OPENFRONT, //90° Kurve nach links danach gerade weiter
    CURVE_RIGTH_90_OPENFRONT, //90° Kurve nach rechts danach gerade weiter
    CURVE_LEFT_180_OPENFRONT, //180° Kurve nach links danach gerade weiter
    CURVE_LEFT_180_OPENSIDE,	//180° Kurve nach links danach nächste Kurve nach rechts
		CURVE_RIGTH_180_OPENFRONT, //180° Kurve nach rechts danach gerade weiter
    CURVE_RIGTH_180_OPENSIDE,	//180° Kurve nach rechts danach nächste Kurve nach links
		DIAGONAL_LEFT_ONE,	//eine diagonale nach links(45°)
		DIAGONAL_RIGTH_ONE,	//eine diagonale nach rechts(45°)
		DIAGONAL_LEFT_MORE,	//mehrere diagonalen nach links(45°)
		DIAGONAL_RIGTH_MORE	//mehrere diagonalen nach rechts(45°)
} CurveType_t;

typedef struct{
	bool startTurnRight; //45° beim Start der diagonalen 45° nach rechts oder links
	bool EndTurnRight;	//45° beim Ende der diagonalen 45° nach rechts oder links
	uint8_t moveDiags;	//wieveile Diagonalen werden gefahren
	int currentIndex;		//Der Index von shortestPath muss aktualisiert werden
	CurveType_t curveType; //curveType
}diagCommand;

//Prüft, ob eine diagonale kommt 0 = nein, 1 = 45° nach rechts, 2 = 45° nach links lenken danach
diagCommand CheckDiagonal(Point fastestPath[gSizeMouseMaze * gSizeMouseMaze], int currentIndex, bool direction);
diagCommand CheckCurveType(Point shortestPath[gSizeMouseMaze * gSizeMouseMaze], int currentIndex, bool direction);

#endif
