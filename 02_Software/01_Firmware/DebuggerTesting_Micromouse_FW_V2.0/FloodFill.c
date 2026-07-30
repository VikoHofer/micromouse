#include "FloodFill.h"
#include "Refresh.h"
#include <string.h>
#include "BOARD_LED.h"
#include <stdio.h>
#include "TimeOfLight.h"
#include "motion_control.h"
#include "Delay.h"
#include "board_button.h"
#include "Diagonal.h"
#include "BLEDebugger.h"
#include "config.h"
#include <math.h>


uint8_t const gSizeMouseMaze = 16;
uint8_t const gSizeLabyrinth = 33;
uint8_t const gPossibleDirections = 4;
Field mouseMaze[16][16];
TMaze maze;

bool SENSOR_ENABLE = false;


void PrintHorizontalRow(Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze], size_t row)
{
    char buffer[256];
    int pos = 0;

    if (mouseMaze[row][0].top == true)
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, " --");
    else
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, "   ");

    for (size_t i = 1; i < gSizeMouseMaze; i++)
    {
        if (mouseMaze[row][i].top == true)
            pos += snprintf(buffer + pos, sizeof(buffer) - pos, " --");
        else
            pos += snprintf(buffer + pos, sizeof(buffer) - pos, "   ");
    }

    printf("%s\n", buffer);
		Delay_ms(10);
}

void PrintMouseLabyrinth(Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze], Point mousePos)
{
    char buffer[256];

    for (size_t i = 0; i < gSizeMouseMaze; i++)
    {
        int pos = 0;

        // obere Wandzeile
        if (i == 0)
        {
            buffer[pos++] = ' ';
        }

        for (size_t j = 0; j < gSizeMouseMaze; j++)
        {
            if (mouseMaze[i][j].top)
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, " --");
            else
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "   ");
        }

        buffer[pos++] = '\n';
        buffer[pos] = '\0';

        printf("%s", buffer);
        Delay_ms(10);

        // Zellinhalt
        pos = 0;

        for (size_t j = 0; j < gSizeMouseMaze; j++)
        {
            if (mouseMaze[i][j].left)
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "|");
            else
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, " ");

            if (mousePos.x == j && mousePos.y == i)
            {
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "\033[1;31mMM\033[0m");
            }
            else if (mouseMaze[i][j].wayLength < 10)
            {
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, " %d", mouseMaze[i][j].wayLength);
            }
            else
            {
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "%d", mouseMaze[i][j].wayLength);
            }

            if (j == gSizeMouseMaze - 1)
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "|");
        }

        buffer[pos++] = '\n';
        buffer[pos] = '\0';

        printf("%s", buffer);
        Delay_ms(10);
    }

    printf(" -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n");
    Delay_ms(10);
}

//Setzt die �u�eren W�nde auf True
static void SetOuterWalls(Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze]) {
	for (uint8_t i = 0; i < gSizeMouseMaze; i++) {
		mouseMaze[0][i].top = true;
		mouseMaze[gSizeMouseMaze - 1][i].bottom = true;
	}
	for (uint8_t i = 0; i < gSizeMouseMaze; i++) {
		mouseMaze[i][0].left = true;
		mouseMaze[i][gSizeMouseMaze - 1].right = true;
	}
}

//initialisiert das 16x16 Feld der Maus
//aeussere Waende werden gesetzt die Zahlen werden eingefuegt
void InitMouseMaze(Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze])
{
	uint8_t startNumberRow = 14;
	uint8_t runningNumber = 0;
	uint8_t const midNumber = 7;



	for (uint8_t i = 0; i < gSizeMouseMaze; i++) {
		for (uint8_t j = 0; j < gSizeMouseMaze; j++) {
			mouseMaze[i][j].wayLength = startNumberRow - runningNumber;
			mouseMaze[i][j].top = false;
			mouseMaze[i][j].right = false;
			mouseMaze[i][j].bottom = false;
			mouseMaze[i][j].left = false;
			if (j < midNumber) {
				runningNumber++;
			}
			if (j > midNumber) {
				runningNumber--;
			}
		}
		runningNumber = 0;
		if (i < midNumber) {
			startNumberRow--;
		}
		if (i > midNumber) {
			startNumberRow++;
		}
	}

	SetOuterWalls(mouseMaze);
}

static bool FoundGoal(uint8_t * mouseX, uint8_t* mouseY) {

	if (*mouseX >= 7 && *mouseX <= 8) {
		if (*mouseY >= 7 && *mouseY <= 8) {

			return false;
		}
	}
	return true;
}

void UpdateWallTop(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],uint8_t const mouseX, uint8_t const mouseY){
	Debug_SetWall(WallPos_Top);
    mousemaze[mouseY][mouseX].top = true;
    if (mouseY != 0) {
        mousemaze[mouseY - 1][mouseX].bottom = true;
    }
}

void UpdateWallRight(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],uint8_t const mouseX, uint8_t const mouseY){
	Debug_SetWall(WallPos_Right);
    mousemaze[mouseY][mouseX].right = true;
    if (mouseX != gSizeMouseMaze - 1) {
        mousemaze[mouseY][mouseX + 1].left = true;
    }
}

void UpdateWallBottom(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],uint8_t const mouseX, uint8_t const mouseY){
	Debug_SetWall(WallPos_Bottom);
    mousemaze[mouseY][mouseX].bottom = true;
    if (mouseY != gSizeMouseMaze - 1) {
        mousemaze[mouseY + 1][mouseX].top = true;
    }
}

void UpdateWallLeft(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],uint8_t const mouseX, uint8_t const mouseY){
	Debug_SetWall(WallPos_Left);
    mousemaze[mouseY][mouseX].left = true;
    if (mouseX != 0) {
        mousemaze[mouseY][mouseX - 1].right = true;
    }
}


static void SetWalls(char const maze[33][33], Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], uint8_t const mouseX, uint8_t const mouseY
    ,enum directions mouseDriveDirection) {

	//uint8_t mazeX = mouseX * 2 + 1;
	//uint8_t mazeY = mouseY * 2 + 1;
    
//    //SOFTWARE MAUS 1, --> Software Maus 2 ist realistischer dieser Code kann
//    //gegebenfalls gel�scht werden.
//	//Die Waende vom aktuellen Feld setzen, aber auch fuer das anliegende Feld
//	if (maze[mazeY - 1][mazeX] == '*') {
//		mousemaze[mouseY][mouseX].top = true;
//		if (mouseY != 0) {
//			mousemaze[mouseY - 1][mouseX].bottom = true;
//		}
//	}
//	if (maze[mazeY][mazeX + 1] == '*') {
//		mousemaze[mouseY][mouseX].right = true;
//		if (mouseX != gSizeMouseMaze - 1) {
//			mousemaze[mouseY][mouseX + 1].left = true;
//		}
//	}
//	if (maze[mazeY + 1][mazeX] == '*') {
//		mousemaze[mouseY][mouseX].bottom = true;
//		if (mouseY != gSizeMouseMaze - 1) {
//			mousemaze[mouseY + 1][mouseX].top = true;
//		}
//	}
//	if (maze[mazeY][mazeX - 1] == '*') {
//		mousemaze[mouseY][mouseX].left = true;
//		if (mouseX != 0) {
//			mousemaze[mouseY][mouseX - 1].right = true;
//		}
//	}
//    
//    //SOFTWARE MAUS 2
//    if(mouseDriveDirection == rightt){
//        if (maze[mazeY - 1][mazeX] == '*') {
//            UpdateWallTop(mousemaze,mouseX,mouseY);
//        }
//        if (maze[mazeY][mazeX + 1] == '*'){
//            UpdateWallRight(mousemaze,mouseX,mouseY);
//        }
//        if (maze[mazeY + 1][mazeX] == '*'){
//            UpdateWallBottom(mousemaze,mouseX,mouseY);
//        }
//    }
//    else if(mouseDriveDirection == bottom){
//        if (maze[mazeY][mazeX + 1] == '*') {
//            UpdateWallRight(mousemaze,mouseX,mouseY);
//        }
//        if (maze[mazeY + 1][mazeX] == '*'){
//            UpdateWallBottom(mousemaze,mouseX,mouseY);
//        }
//        if (maze[mazeY][mazeX - 1] == '*'){
//            UpdateWallLeft(mousemaze,mouseX,mouseY);
//        }
//    }

//    else if(mouseDriveDirection == leftt){
//        if (maze[mazeY + 1][mazeX] == '*') {
//            UpdateWallBottom(mousemaze,mouseX,mouseY);
//        }
//        if (maze[mazeY][mazeX - 1] == '*'){
//            UpdateWallLeft(mousemaze,mouseX,mouseY);
//        }
//        if (maze[mazeY - 1][mazeX] == '*'){
//            UpdateWallTop(mousemaze,mouseX,mouseY);
//        }
//    }

//    else if(mouseDriveDirection == top){
//        if (maze[mazeY][mazeX - 1] == '*') {
//            UpdateWallLeft(mousemaze,mouseX,mouseY);
//        }
//        if (maze[mazeY - 1][mazeX] == '*'){
//            UpdateWallTop(mousemaze,mouseX,mouseY);
//        }
//        if (maze[mazeY][mazeX + 1] == '*'){
//            UpdateWallRight(mousemaze,mouseX,mouseY);
//        }
//    }
    
    
    //�NDERUNG HARDWARE MAUS
    uint32_t leftValue = 0;
    uint32_t middleValue = 0;
    uint32_t rightValue = 0;
    
    //Fuer die Wanderkennung in Ordnung
    //Zum Nachregeln siehe die Messwerte im Ordner
    //micromouse\02_Software\01_Firmware\02_SensorData\Sensor_Maze_Tests_Rene.xlsx"
		

	
    uint32_t const maxValue = 120;
    
    //zweimal weil erster Aufruf Pfusch ist
    //GetValuesForMouse(&rightValue,&middleValue,&leftValue);
    
    // PA0
    // duration 5ms!!
    leftValue = readSensor_Left();

    // PA1
    // duration 5ms!!
    rightValue = readSensor_Right();

    // PB0
    middleValue = (uint32_t)((readSensor_Front2() + readSensor_Front()) / 2);
		
		printf("Setwalls");
		printf("leftValue: %u", leftValue);
		printf("rightValue: %u", rightValue);
		printf("middleValue: %u", middleValue);
    
    if(mouseDriveDirection == rightt){
            
        if (leftValue < maxValue) {
            UpdateWallTop(mousemaze,mouseX,mouseY);
        }
        if (middleValue < maxValue){
            UpdateWallRight(mousemaze,mouseX,mouseY);
        }
        if (rightValue < maxValue){
            UpdateWallBottom(mousemaze,mouseX,mouseY);
        }
    }
    else if(mouseDriveDirection == bottom){
        if (leftValue < maxValue) {
            UpdateWallRight(mousemaze,mouseX,mouseY);
        }
        if (middleValue < maxValue){
            UpdateWallBottom(mousemaze,mouseX,mouseY);
        }
        if (rightValue < maxValue){
            UpdateWallLeft(mousemaze,mouseX,mouseY);
        }
    }

    else if(mouseDriveDirection == leftt){
        if (leftValue < maxValue) {
            UpdateWallBottom(mousemaze,mouseX,mouseY);
        }
        if (middleValue < maxValue){
            UpdateWallLeft(mousemaze,mouseX,mouseY);
        }
        if (rightValue < maxValue){
            UpdateWallTop(mousemaze,mouseX,mouseY);
        }
    }

    else if(mouseDriveDirection == top){
        if (leftValue < maxValue) {
            UpdateWallLeft(mousemaze,mouseX,mouseY);
        }
        if (middleValue < maxValue){
            UpdateWallTop(mousemaze,mouseX,mouseY);
        }
        if (rightValue < maxValue){
            UpdateWallRight(mousemaze,mouseX,mouseY);
        }
	}

}


static uint8_t GetSmallestValue(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],
                        uint8_t tempValues[gPossibleDirections], uint8_t const mouseX, uint8_t const mouseY) {
    
  uint8_t const INT_MAX = 255;
	uint8_t smallestTemp = INT_MAX;

	int dx[] = { 0, 1, 0, -1 };
	int dy[] = { -1, 0, 1, 0 };

	for (uint8_t i = 0; i < gPossibleDirections; ++i) {
		int newX = mouseX + dx[i];
		int newY = mouseY + dy[i];
		
		if(newX < 0 || newY < 0 || newX > gSizeMouseMaze || newY > gSizeMouseMaze){
			continue;
		}
        //RHAxxx right und left vertauscht?
        if( i == 0 && mousemaze[mouseY][mouseX].top){continue;}
        if( i == 1 && mousemaze[mouseY][mouseX].right){continue;}
        if( i == 2 && mousemaze[mouseY][mouseX].bottom){continue;}
        if( i == 3 && mousemaze[mouseY][mouseX].left){continue;}

		if (newX < gSizeMouseMaze && newY < gSizeMouseMaze && newX >= 0 && newY >= 0) {
			tempValues[i] = mousemaze[newY][newX].wayLength;

			if (tempValues[i] != INT_MAX && tempValues[i] < smallestTemp) {
				smallestTemp = tempValues[i];
			}
		}
		else {
			tempValues[i] = INT_MAX;
		}
	}
	return smallestTemp;
}
                        
static void RefreshBehindWalls(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],
                        Point pos, uint8_t * mouseX, uint8_t * mouseY, enum directions direction) {
	if (direction != none) {
        //pruefen von Wand und ob das Feld innerhalb der Labyrinthgrenzen ist.
		if (mousemaze[*mouseY][*mouseX].right == true && (*mouseX + 1) < (gSizeMouseMaze - 1)) {
			pos.x = *mouseX + 1;
			pos.y = *mouseY;
			Refresh(pos, mousemaze);
		}
		if (mousemaze[*mouseY][*mouseX].bottom == true && (*mouseY + 1) < (gSizeMouseMaze - 1)) {
			pos.x = *mouseX;
			pos.y = *mouseY + 1;
			Refresh(pos, mousemaze);
		}
		if (mousemaze[*mouseY][*mouseX].left == true && (*mouseX) > 0) {
			pos.x = *mouseX - 1;
			pos.y = *mouseY;
			Refresh(pos, mousemaze);
		}
		if (mousemaze[*mouseY][*mouseX].top == true && (*mouseY) > 0) {
			pos.x = *mouseX;
			pos.y = *mouseY - 1;
			Refresh(pos, mousemaze);
		}
	}
}

//�NDERUNG HARDWARE MAUS 
//ROTATE MAUS
void RotateMouse(enum directions currentDirection, enum directions rotateToDirection){
    
    if(rotateToDirection == none || currentDirection == none){
        return;
    }
    
    //rotate 0 = no rotation
    //rotate 1 = rotate right
    //rotate 2 = rotate left
    //rotate 3 = turn 180� left or right(will be defined later)
    int rotate = 0;
    
    if(currentDirection == rotateToDirection){ rotate = 0;}
    else if(currentDirection == top && rotateToDirection == leftt){rotate = 2;}
    else if(currentDirection == top && rotateToDirection == rightt){rotate = 1;}
    else if(currentDirection == top && rotateToDirection == bottom){rotate = 3;}
    
    else if(currentDirection == rightt && rotateToDirection == leftt){rotate = 3;}
    else if(currentDirection == rightt && rotateToDirection == top){rotate = 2;}
    else if(currentDirection == rightt && rotateToDirection == bottom){rotate = 1;}
    
    else if(currentDirection == bottom && rotateToDirection == leftt){rotate = 1;}
    else if(currentDirection == bottom && rotateToDirection == rightt){rotate = 2;}
    else if(currentDirection == bottom && rotateToDirection == top){rotate = 3;}
    
    else if(currentDirection == leftt && rotateToDirection == top){rotate = 1;}
    else if(currentDirection == leftt && rotateToDirection == rightt){rotate = 3;}
    else if(currentDirection == leftt && rotateToDirection == bottom){rotate = 2;}
    
    //AENDERUNG HARDWARE MAUS 
    //ROTATE MAUS
    //Funktion fuer Motor
    if(rotate == 1){
    	//TODO auf anderes tunright aendern
      turn(1);
	  //Debug_Move(Move_Right);
    }
    else if (rotate == 2){
      turn(-1);
	  Debug_Move(Move_Left);
    }
    else if (rotate == 3){
			printf("rotate 3\n");
      turn(2);
			printf("leave rotate 3\n");
//			Debug_Move(Move_Right);
//			Debug_Move(Move_Right);
    }
		
}

enum directions DecideDriveDirection(uint8_t tempValues[gPossibleDirections], uint8_t smallestTemp,
	uint8_t * mouseX, uint8_t * mouseY, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], enum directions const mouseDriveDirection)
{
	enum directions getDirection = none;
    
	Point pos = { 0,0 };
    
    //Code mit Wanderkennung
    if (smallestTemp >= mousemaze[*mouseY][*mouseX].wayLength){
		pos.x = *mouseX;
        pos.y = *mouseY;
        Refresh(pos, mousemaze);
		smallestTemp = GetSmallestValue(mousemaze, tempValues, *mouseX, *mouseY);
	}
		
		
    if (tempValues[1] == smallestTemp && mousemaze[*mouseY][*mouseX].right == false) {
        getDirection = rightt;
    }
    else if (tempValues[2] == smallestTemp && mousemaze[*mouseY][*mouseX].bottom == false) {
        getDirection = bottom;
    }
    else if (tempValues[3] == smallestTemp && mousemaze[*mouseY][*mouseX].left == false) {
        getDirection = leftt;
    }
    else if (tempValues[0] == smallestTemp && mousemaze[*mouseY][*mouseX].top == false) {
        getDirection = top;
    }

    
    //�NDERUNG HARDWARE MAUS 
    //ROTATE MAUS
    RotateMouse(mouseDriveDirection,getDirection);

//	Refresh ueberall wo neue Waende sind
//	Derzeit noch dopplet, da darueber auch diese Positionen aktualisiert werden
	RefreshBehindWalls(mousemaze, pos, mouseX, mouseY, getDirection);

	return getDirection;

}

static void Drive(enum directions direction, uint8_t * mouseX, uint8_t * mouseY) {

    //HARDWARE und SOFTWARE MAUS
	if (direction == top) {
		*mouseY = *mouseY - 1;
	}
	else if (direction == bottom) {
		*mouseY = *mouseY + 1;
	}
	else if (direction == leftt) {
		*mouseX = *mouseX - 1;
	}
	else if (direction == rightt) {
		*mouseX = *mouseX + 1;
	}
	else if (direction == none) {
		//do nothing (must not be an error)
	}
    
    //HARDWARE MAUS
    //Muss nur noch gerade fahren da in der RotateMouse die Maus gedreht wird.
    //Motor Funktion hier beide R�der gleich schnell
    //mouseX und mouseY Wert muss aber richtig angepasst werden.
		Debug_Move(Move_Straight);
    move(1, 400, 1500,0);
	

}

//HARDWARE MAUS
void ScanCenter3Right(TMaze maze, uint8_t * mouseX, uint8_t * mouseY,Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],enum directions * currentMouseDirection){
    
	SetWalls(maze, mousemaze, *mouseX, *mouseY,*currentMouseDirection);
    for(uint8_t i = 0; i < 4;i++){
        Point p;
        p.x = *mouseX;
        p.y = *mouseY;
        RefreshBehindWalls(mousemaze, p, mouseX, mouseY, *currentMouseDirection);
				Drive(*currentMouseDirection,mouseX,mouseY);
        SetWalls(maze, mousemaze, *mouseX, *mouseY,*currentMouseDirection);
        //Beim letzten mal nicht mehr updaten
        if(i != 3){
            enum directions updateDirection = *currentMouseDirection;
            if(updateDirection == leftt){
                updateDirection = top;
            }else{
                updateDirection++;
            }
						//HARDWARE Maus
						RotateMouse(*currentMouseDirection,updateDirection);
						//SOFTWARE Maus
            *currentMouseDirection = updateDirection;
						
        }
    }
    SetWalls(maze, mousemaze, *mouseX, *mouseY,*currentMouseDirection);
    Point p;
    p.x = *mouseX;
    p.y = *mouseY;
    RefreshBehindWalls(mousemaze, p, mouseX, mouseY, *currentMouseDirection);
		
    
}


//HARDWARE MAUS
void ScanCenter3Left(TMaze maze, uint8_t * mouseX, uint8_t * mouseY,Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],enum directions * currentMouseDirection){
    
    for(uint8_t i = 0; i < 4;i++){
        SetWalls(maze, mousemaze, *mouseX, *mouseY,*currentMouseDirection);
        Point p;
        p.x = *mouseX;
        p.y = *mouseY;
        RefreshBehindWalls(mousemaze, p, mouseX, mouseY, *currentMouseDirection);
				Drive(*currentMouseDirection,mouseX,mouseY);
        
        //Beim letzten mal nicht mehr updaten
        if(i != 3){
            enum directions updateDirection = *currentMouseDirection;
            if(updateDirection == top){
                updateDirection = leftt;
            }else{
                updateDirection--;
            }
            //HARDWARE Maus
			RotateMouse(*currentMouseDirection,updateDirection);
			//SOFTWARE Maus
            *currentMouseDirection = updateDirection;
        }
    }

    SetWalls(maze, mousemaze, *mouseX, *mouseY,*currentMouseDirection);
    Point p;
    p.x = *mouseX;
    p.y = *mouseY;
    RefreshBehindWalls(mousemaze, p, mouseX, mouseY, *currentMouseDirection);
    
    
}

//Diese Funktion dient dem Abscannen vom Ziel und anschliessender Positionierung auf 8|8
static void ScanCenter(TMaze maze, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],
                uint8_t tempValues[gPossibleDirections], uint8_t * mouseX, uint8_t * mouseY, enum directions * currentMouseDirection) {
	
       
        //ALLGEMEINE L�SUNG
        //Die Maus kommt am Ziel an 7,7 bis 8,8
        //Es gibt zwei unterschiedliche M�glichkeiten je nach Ausrichtung und Zielfeld
        //Bsp 8,8 und Ausrichtung top --> Maus muss gerade und dreimal links fahren.
        //Bsp 8,8 und Ausrichtung links --> Maus muss gerade und dreimal rechts fahren.

        
        if(*mouseX == 8 && *mouseY == 8){
            if(*currentMouseDirection == top){
                ScanCenter3Left(maze, mouseX, mouseY, mousemaze, currentMouseDirection);
            }
            else{
								printf("ScanCenter3Right");
                ScanCenter3Right(maze, mouseX, mouseY, mousemaze, currentMouseDirection);
            }
        }

        else if(*mouseX == 7 && *mouseY == 7){
            if(*currentMouseDirection == bottom){
                ScanCenter3Left(maze, mouseX, mouseY, mousemaze, currentMouseDirection);
            }
            else{
                ScanCenter3Right(maze, mouseX, mouseY, mousemaze, currentMouseDirection);
            }
        }   

        else if(*mouseX == 8 && *mouseY == 7){
            if(*currentMouseDirection == leftt){
                ScanCenter3Left(maze, mouseX, mouseY, mousemaze, currentMouseDirection);
            }
            else{
                ScanCenter3Right(maze, mouseX, mouseY, mousemaze, currentMouseDirection);
            }
        }  

        else if(*mouseX == 7 && *mouseY == 8){
            if(*currentMouseDirection == rightt){
                ScanCenter3Left(maze, mouseX, mouseY, mousemaze, currentMouseDirection);
            }
            else{
                ScanCenter3Right(maze, mouseX, mouseY, mousemaze, currentMouseDirection);
            }
        }  
        
}
								
void DriveFast(uint8_t * mouseX, uint8_t * mouseY, Point shortestPath[gSizeMouseMaze * gSizeMouseMaze]){
	
	if(*mouseX != 0 || *mouseY != 0){
		return;
	}
	
}

uint8_t EvaluateTurn(int IndexNew, Point shortestPath[gSizeMouseMaze * gSizeMouseMaze]){
	
			//TURN
		if(IndexNew == 0){
			//kein turn mehr n�tig
		}
		//Turn einf�gen, 0 = rechts, 1 = unten, 2 = links, 3 = oben
		else if(shortestPath[IndexNew].x - shortestPath[IndexNew-1].x == 1){
			return 0;
		}

		else if(shortestPath[IndexNew].y - shortestPath[IndexNew-1].y == 1){
			return 1;
		}
		
		else if(shortestPath[IndexNew].x - shortestPath[IndexNew-1].x == -1){
			return 2;
		}

		else if(shortestPath[IndexNew].y - shortestPath[IndexNew-1].y == -1){
			return 3;
		}
	
		//Fehlerwert
		return 255;
}

//Drive fast
void GetDriveCommands(uint8_t * mouseX, uint8_t * mouseY, Point shortestPath[gSizeMouseMaze * gSizeMouseMaze], 
		enum directions * currentMouseDirection, uint8_t Index){

	
	//0 = y, 1 = x Richtung
	bool direction = 0;
	float fastPathMove[255] = {0};
	int fastPathTurn[255] = {0};
	uint8_t fastPathIndex = 0;
	int IndexNew = Index;

	//Maus richtig ausrichten
	if(*mouseX == 0 && *mouseY == 0){
			
		//Ausrichtung nach rechts
		if(shortestPath[IndexNew - 1].x == 1){
			RotateMouse(*currentMouseDirection,rightt);
			*currentMouseDirection= rightt;
			direction = 0;
		}
		//Ausrichtung nach unten
		else 	if(shortestPath[IndexNew - 1].y == 1){
			RotateMouse(*currentMouseDirection,bottom);
			*currentMouseDirection = bottom;
			direction = 1;
		}
		

		
		//den k�rzesten Weg abwechselnd in move und turn abspeichern
		while(IndexNew >= 0){
			
			//EVALUATE CurveType
			diagCommand diagCom = CheckCurveType(shortestPath,IndexNew,direction);
			
			//Pr�fung ist notwendig, weil nach einer diagonalen nicht klar ist
			// welchen wert direction hat.
			if(shortestPath[IndexNew].x - shortestPath[IndexNew-1].x != 0){
				direction = 0;
			}
			else{
				direction = 1;
			}
			
			//MOVE
			//Erkennen von langen geraden in x Richtung
			float pathLength = 0;
			if(direction == 0){
				while(shortestPath[IndexNew].x != shortestPath[IndexNew-1].x){
					pathLength++;
					IndexNew--;
					if(IndexNew <= 0){
						break;
					}

				}
				direction = 1;
			}
			//Erkennen von langen geraden in y Richtung
			else if (direction == 1){
				//RHA TODO pr�fen, ob && shortestPath[IndexNew].y != 7 ben�tigt wird falls ja bei x auch einf�gen
				while(shortestPath[IndexNew].y != shortestPath[IndexNew-1].y /*&& shortestPath[IndexNew].y != 7*/){
					pathLength++;
					IndexNew--;
					if(IndexNew <= 0){
						break;
					}

				}
				direction = 0;
			}
			
			//TEST erkennen der verschiedenen Kurvenarten
			if(diagCom.curveType == CURVE_LEFT_90_OPENFRONT ||
				diagCom.curveType == CURVE_RIGTH_90_OPENFRONT){
				fastPathMove[fastPathIndex] = pathLength;
				uint8_t turnValue = EvaluateTurn(IndexNew,shortestPath);
				fastPathTurn[fastPathIndex] = turnValue;
				fastPathIndex++;
				continue;
			}
			if(diagCom.curveType == CURVE_LEFT_180_OPENFRONT ||
				diagCom.curveType == CURVE_RIGTH_180_OPENFRONT){
				fastPathMove[fastPathIndex] = pathLength;
				uint8_t turnValue = EvaluateTurn(IndexNew,shortestPath);
				fastPathTurn[fastPathIndex] = turnValue;
				IndexNew--;
				fastPathIndex++;
				fastPathMove[fastPathIndex] = 1;
				turnValue = EvaluateTurn(IndexNew,shortestPath);
				fastPathTurn[fastPathIndex] = turnValue;
				fastPathIndex++;
				//gleiche Richtung!
				direction = !direction;
				continue;
			}
			else if(diagCom.curveType == CURVE_LEFT_180_OPENSIDE ||
				diagCom.curveType == CURVE_RIGTH_180_OPENSIDE){
				fastPathMove[fastPathIndex] = pathLength;
				uint8_t turnValue = EvaluateTurn(IndexNew,shortestPath);
				fastPathTurn[fastPathIndex] = turnValue;
				IndexNew--;
				fastPathIndex++;
				fastPathMove[fastPathIndex] = 1;
				turnValue = EvaluateTurn(IndexNew,shortestPath);
				fastPathTurn[fastPathIndex] = turnValue;
				fastPathIndex++;
				//bis hier gleich wie Openfront
				IndexNew--;
				fastPathMove[fastPathIndex] = 1;
				turnValue = EvaluateTurn(IndexNew,shortestPath);
				fastPathTurn[fastPathIndex] = turnValue;
				fastPathIndex++;
				continue;
			}
			//DIAGONALE
			else if(diagCom.curveType == DIAGONAL_LEFT_ONE ||
				diagCom.curveType == DIAGONAL_RIGTH_ONE ||
				diagCom.curveType == DIAGONAL_LEFT_MORE ||
				diagCom.curveType == DIAGONAL_RIGTH_MORE)
			{	
				//Index neu setzen
				IndexNew = diagCom.currentIndex+1;
				
				//pathLength um 0.5 reduzieren f�r die korrekte Positionierung f�r die diagonale
				pathLength -= 0.5f;
				//move bis zum Start der diagonalen
				fastPathMove[fastPathIndex] = pathLength;
				
				//Drehung f�r Positionierung f�r die Diagonale
				if(diagCom.startTurnRight){
					fastPathTurn[fastPathIndex] = 4;
				}
				else{
					fastPathTurn[fastPathIndex] = 5;
				}
				fastPathIndex++;
				//move f�r die Diagonale
				fastPathMove[fastPathIndex] = (diagCom.moveDiags * 1.4142)/2;	//Diagonale/2
				//Drehung wieder gerade zum Weiterfahren
				if(diagCom.EndTurnRight){
					fastPathTurn[fastPathIndex] = 4;
				}
				else{
					fastPathTurn[fastPathIndex] = 5;
				}
				fastPathIndex++;
				
				//die Richtung �ndert sich nicht
				if(diagCom.startTurnRight != diagCom.EndTurnRight){
					direction = !direction;
				}
				
				continue;
			}

			fastPathMove[fastPathIndex] = pathLength;
			
			//TURN
			if(IndexNew == 0){
				//kein turn mehr n�tig
			}
			//Turn einf�gen, 0 = rechts, 1 = unten, 2 = links, 3 = oben
			else if(shortestPath[IndexNew].x - shortestPath[IndexNew-1].x == 1){
				fastPathTurn[fastPathIndex] = 0;
			}

			else if(shortestPath[IndexNew].y - shortestPath[IndexNew-1].y == 1){
				fastPathTurn[fastPathIndex] = 1;
			}
			
			else if(shortestPath[IndexNew].x - shortestPath[IndexNew-1].x == -1){
				fastPathTurn[fastPathIndex] = 2;
			}

			else if(shortestPath[IndexNew].y - shortestPath[IndexNew-1].y == -1){
				fastPathTurn[fastPathIndex] = 3;
			}
			fastPathIndex++;
			
			
		}
		
	}
	
	//Den schnellsten Weg fahren
	int8_t countDiagEntries = 0;
	//Bei fastPathIndex "-1" eingef�gt weil sobald die Maus im Ziel ist immer noch ein Feld gefahren wird?
	for(int i = 0; i < fastPathIndex-1;i++){
		move(fastPathMove[i],750,2500,0);
		//falls ein Wert mit .5f kommt wird eine diagonale danach gefahren
		//bei der diagonalen sollen die Sensoren nicht verwendet werden
		//nach der diagonalen kommt wieder ein path mit 0.5f der den Sensor wieder aktiviert
		bool hatKommaFuenf = (fabs(fmod(fastPathMove[i], 1.0f) - 0.5f) < 0.0001f);
		if (hatKommaFuenf) {
				SENSOR_ENABLE = !SENSOR_ENABLE;   // <-- toggle
		}
		if(fastPathTurn[i] == 0){
			RotateMouse(*currentMouseDirection, leftt);
			*currentMouseDirection = leftt;
		}
		else if(fastPathTurn[i] == 1){
			RotateMouse(*currentMouseDirection, top);
			*currentMouseDirection = top;
		}
		else if(fastPathTurn[i] == 2){
			RotateMouse(*currentMouseDirection, rightt);
			*currentMouseDirection = rightt;
		}
		else if(fastPathTurn[i] == 3){
			RotateMouse(*currentMouseDirection, bottom);
			*currentMouseDirection = bottom;
		}
		//turnDiagRigth 45�
		else if(fastPathTurn[i] == 4){
			countDiagEntries++;
			if(countDiagEntries >= 2){
				//adjust currentMouseDirection
				switch (*currentMouseDirection) {
        case top:  *currentMouseDirection = rightt;
        case rightt:  *currentMouseDirection = bottom;
        case bottom:  *currentMouseDirection = leftt;
        case leftt:  *currentMouseDirection = top;
        default: *currentMouseDirection = none;
				}
			}
			turn(0.5f);
		}
		//turnDiagLeft 45�
		else if(fastPathTurn[i] == 5){
			countDiagEntries--;
			if (countDiagEntries <= -2){
				//adjust currentMouseDirection
				switch (*currentMouseDirection) {
        case top:  *currentMouseDirection = leftt;
        case rightt:  *currentMouseDirection = top;
        case bottom:  *currentMouseDirection = rightt;
        case leftt:  *currentMouseDirection = bottom;
        default: *currentMouseDirection = none;
				}
			}
			turn(-0.5f);
		}
	}
}

struct fastPathCommand {
    float distance;
    bool isMove;
};

void GetDriveCommandsSmooth(uint8_t * mouseX, uint8_t * mouseY, Point shortestPath[gSizeMouseMaze * gSizeMouseMaze], 
		enum directions * currentMouseDirection, uint8_t Index){
			
	printf("GetDriveCommandsSmooth\n");

	Board_LED_Off(0);
	Board_LED_Off(1);
	Board_LED_Off(2);
	
	//0 = y, 1 = x Richtung
	bool direction = 0;
	struct fastPathCommand pathCommands[255] = {0};
	uint8_t fastPathIndex = 0;
	int IndexNew = Index;
	bool doOnce = true;

	printf("IndexNew: %u\n", IndexNew);
	printf("mouseX : %u mouseY: %u \n", *mouseX, *mouseY);
	//Maus richtig ausrichten
	if(*mouseX == 0 && *mouseY == 0){
			
		//Ausrichtung nach rechts
		if(shortestPath[IndexNew - 1].x == 1){
			printf("Enter RotateMouse\n");
			RotateMouse(*currentMouseDirection,rightt);
			printf("Leave RotateMouse\n");
			direction = 0;
		}
		//Ausrichtung nach unten
		else 	if(shortestPath[IndexNew - 1].y == 1){
			RotateMouse(*currentMouseDirection,bottom);
			direction = 1;
		}

		printf("IndexNew: %u\n", IndexNew);
		//den k�rzesten Weg abwechselnd abspeichern, mehrere turns hintereinander möglich
		while(IndexNew >= 0){
			printf("IndexNew: %u\n", IndexNew);
			//EVALUATE CurveType
			diagCommand diagCom = CheckCurveType(shortestPath,IndexNew,direction);
			printf(0,"curveType: %u\n",diagCom.curveType);
			
			//MOVE
			//Erkennen von langen geraden in x Richtung
			float pathLength = 0;
			if(direction == 0){
				if(IndexNew != 0){				
					while(IndexNew > 0 && shortestPath[IndexNew].x != shortestPath[IndexNew-1].x){
						pathLength++;
						IndexNew--;
					}
					direction = 1;
				}
				else{
					pathLength = 1;
					direction = 1;
				}
			}
			//Erkennen von langen geraden in y Richtung
			else if (direction == 1){
				//RHA TODO pr�fen, ob && shortestPath[IndexNew].y != 7 ben�tigt wird falls ja bei x auch einf�gen
				if(IndexNew != 0){	
					while(IndexNew > 0 && shortestPath[IndexNew].y != shortestPath[IndexNew-1].y /*&& shortestPath[IndexNew].y != 7*/){
						pathLength++;
						IndexNew--;
					}
					direction = 0;
				}
				else{
					pathLength = 1;
					direction = 0;
				}
				
			}
			
			pathLength -= 1.0f;
			//Am Anfang richtig positionieren für Kurven
			if(doOnce){
				doOnce = false;
				pathLength += 0.5f;
			}
			
			
			//Erkennen der verschiedenen Kurvenarten
			if(diagCom.curveType == CURVE_LEFT_90_OPENFRONT ||
				diagCom.curveType == CURVE_RIGTH_90_OPENFRONT){
				pathCommands[fastPathIndex].distance = pathLength;
				pathCommands[fastPathIndex].isMove = true;
					
				float turnValue = 0.0f;
				if(diagCom.curveType == CURVE_LEFT_90_OPENFRONT){
					turnValue = 95.0f;
				}
				else{
					turnValue = -95.0f;
				}
				fastPathIndex++;
				pathCommands[fastPathIndex].distance = turnValue;
				pathCommands[fastPathIndex].isMove = false;
				fastPathIndex++;
				continue;
			}
			if(diagCom.curveType == CURVE_LEFT_180_OPENFRONT ||
				diagCom.curveType == CURVE_RIGTH_180_OPENFRONT){
				IndexNew--;
				pathCommands[fastPathIndex].distance = pathLength;
				pathCommands[fastPathIndex].isMove = true;
					
				float turnValue = 0.0f;
				if(diagCom.curveType == CURVE_LEFT_180_OPENFRONT){
					turnValue = 180.0f;
				}
				else{
					turnValue = -180.0f;
				}
				
				fastPathIndex++;
				pathCommands[fastPathIndex].distance = turnValue;
				pathCommands[fastPathIndex].isMove = false;
				fastPathIndex++;
				
				//gleiche Richtung!
				direction = !direction;
				continue;
			}
			else if(diagCom.curveType == CURVE_LEFT_180_OPENSIDE ||
				diagCom.curveType == CURVE_RIGTH_180_OPENSIDE){
				IndexNew--;
				pathCommands[fastPathIndex].distance = pathLength;
				pathCommands[fastPathIndex].isMove = true;
					
				float turnValue = 0.0f;
				if(diagCom.curveType == CURVE_LEFT_180_OPENSIDE){
					turnValue = 180.0f;
				}
				else{
					turnValue = -180.0f;
				}
				
				fastPathIndex++;
				pathCommands[fastPathIndex].distance = turnValue;
				pathCommands[fastPathIndex].isMove = false;
				fastPathIndex++;
				
				//gleiche Richtung!
				direction = !direction;
				continue;
			}
			else{
				pathCommands[fastPathIndex].distance = pathLength;
				pathCommands[fastPathIndex].isMove = true;
				fastPathIndex++;
				IndexNew--;
				printf("pathLength: %f\n",pathLength);
				Delay_ms(100);
			}
					
		}
		
	}
	
	printf("IndexNew: %u\n", IndexNew);
	printf("fastPathIndex: %u\n", fastPathIndex);
	
	Board_LED_On(0);
	Board_LED_On(1);
	Board_LED_On(2);
	
	//Den schnellsten Weg fahren
	int8_t countDiagEntries = 0;
	//Bei fastPathIndex "-1" eingef�gt weil sobald die Maus im Ziel ist immer noch ein Feld gefahren wird?
	Motion_Start();
	for(int i = 0; i < fastPathIndex;i++){
		if(pathCommands[i].isMove == true){
			printf("Motion_CommandForward\n");
			printf("pathCommands[i].distance: %f\n", pathCommands[i].distance);
			//Unterscheiden wie lange die geraden sind, je nachdem schneller fahren
			if(pathCommands[i].distance <= 2.1f){
				Motion_CommandForward(pathCommands[i].distance*180.0f, 1000.0f, 1000.0f, 300.0f, true);
			}
			else{
				Motion_CommandForward(pathCommands[i].distance*177.0f, 2000.0f, 2000.0f, 300.0f, true);
			}
		}
		else{
			printf("Motion_CommandCurveAngle\n");
			float final_Speed = 300.0f;
			if((i+1) < fastPathIndex){
				if(pathCommands[i+1].isMove){
					final_Speed = 1000.0f;
				}
			}
			
			if(pathCommands[i].distance < 100.0f){
				Motion_CommandCurveAngle(pathCommands[i].distance, 90.0f, 500.0f, 99999.0f, 500.0f);
			}
			else{
				Motion_CommandCurveAngle(pathCommands[i].distance, 90.0f, 300.0f, 99999.0f, 300.0f);
			}
		}
		
	}
	//Stop einen Kreis in der Mitte zum Jubeln fahren
	Motion_CommandForward(180.0f, 1000.0f, 1000.0f, 300.0f, true);
	Motion_CommandCurveAngle(720.0f, 0.0f, 300.0f, 99999.0f, 0.0f);
	Motion_Stop();
	
  printf("leave GetDriveCommandsSmooth\n");

}


void SolveMaze(TMaze maze, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze]){
	
	
//TEST! GetDriveCommands with shortestPath
//Point shortestPathTest[gSizeMouseMaze * gSizeMouseMaze];
//uint8_t testIndex = 11;
//Point testPoint = {0 ,0};
//shortestPathTest[11] = testPoint;
//Point testPoint0 = {1 ,0};
//shortestPathTest[10] = testPoint0;
//Point testPoint1 = {2 ,0};
//shortestPathTest[9] = testPoint1;
//Point testPoint2 = {3 ,0};
//shortestPathTest[8] = testPoint2;
//Point testPoint3 = {3 ,1};
//shortestPathTest[7] = testPoint3;
//Point testPoint4 = {2 ,1};
//shortestPathTest[6] = testPoint4;
//Point testPoint5 = {2 ,2};
//shortestPathTest[5] = testPoint5;
//Point testPoint6 = {3 ,2};
//shortestPathTest[4] = testPoint6;
//Point testPoint7 = {3 ,3};
//shortestPathTest[3] = testPoint7;
//Point testPoint8 = {3 ,4};
//shortestPathTest[2] = testPoint8;
//Point testPoint9 = {4 ,4};
//shortestPathTest[1] = testPoint9;
//Point testPoint10 = {4 ,3};
//shortestPathTest[0] = testPoint10;
    
uint8_t mouseX = 0;
uint8_t mouseY = 0; 
	
//Waylength Werte der umliegenden Felder
uint8_t tempValues[4] = { 0 };

//SOFTWARTE MAUS
enum directions mouseDriveDirection = rightt;

//TESTFUNCTION CALL
//GetDriveCommands(&mouseX, &mouseY, shortestPathTest, &mouseDriveDirection,testIndex);
    
//�NDERUNG HARDWARE MAUS
//Je nach Schalter Stellung wird die Richtung ausgew�hlt
//if(){
//    mouseDriveDirection = rightt;
//}
//else{
//    mouseDriveDirection = bottom;
//}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//			move(1, 500, 1500, 0);
//			Delay_ms(100);
//			turn(1);
//			Delay_ms(100);
//			move(1, 500, 1500, 0);
//			Delay_ms(100);
//			turn(-1);
//			Delay_ms(100);
//			move(1, 500, 1500, 0);
//			Delay_ms(100);
//			turn(1);
//			Delay_ms(100);
//			move(1, 500, 1500, 0);
//			Delay_ms(100);
//			turn(-1);
//			Delay_ms(100);
//			move(5, 500, 1500, 0);
//			Delay_ms(100);
//			turn(1);
//			move(1, 500, 1500, 0);
//			
//			
//			// mitte
//			move(1, 500, 1500, 0);
//			move(1, 500, 1500, 0);
//			turn(1);
//			move(1, 500, 1500, 0);
//			move(1, 500, 1500, 0);
//			turn(-1);
//			move(1, 500, 1500, 0);
//			turn(-1);
//			move(1, 500, 1500, 0);
//			move(1, 500, 1500, 0);
//			turn(1);
//			
//			// ziel
//			move(2, 500, 1500, 0);
//			turn(-1);
//			move(1, 500, 1500, 0);
//			turn(-1);
//			move(1, 500, 1500, 0);
//			turn(-1);
//			move(1, 500, 1500, 0);


//			turn(1);
//			turn(1);
//			turn(1);
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


//Schleife bis Ziel gefunden wird
while (FoundGoal(&mouseX, &mouseY)) { // mouseDriveDirection none gehoert entfernt!!!

  //Maus soll vor der Messung stehen
  //Delay_ms(100);
	//Waende von der aktuellen Position aktualisieren
	SetWalls(maze, mousemaze, mouseX, mouseY,mouseDriveDirection);

	//Bestimmen des kleinsten Wertes der umliegenden Felder
	uint8_t smallestTemp = GetSmallestValue(mousemaze, tempValues, mouseX, mouseY);

	//Pruefen in welche Richtung gefahren wird, ist eine Wand im Weg?
	//Zu erst rechts, unten, links und dann oben
	//noch nicht die sauberste Loesung smallesTemp wird einfach erhoeht bei einem Refresh
	mouseDriveDirection = DecideDriveDirection(tempValues, smallestTemp, &mouseX, &mouseY, mousemaze,mouseDriveDirection);
	//bei einem Refresh tempValues aktualisieren
	smallestTemp = GetSmallestValue(mousemaze, tempValues, mouseX, mouseY);   
    //Falls nicht zum kleinsten Wert gefahren werden kann wegen einer Wand wird der n�chst gr��erer Wert gepr�ft.
	while (mouseDriveDirection == none) {
		smallestTemp++;
		mouseDriveDirection = DecideDriveDirection(tempValues, smallestTemp, &mouseX, &mouseY, mousemaze,mouseDriveDirection);
	}
	Drive(mouseDriveDirection, &mouseX, &mouseY);

	printf("Print Labyrinth");
	//Delay_ms(10);
	Point p = {mouseX,mouseY};
	//PrintMouseLabyrinth(mousemaze, p);
	
}

Board_LED_Off(0);
Board_LED_Off(1);
Board_LED_On(2);

printf("ScanCenter");
ScanCenter(maze,mousemaze, tempValues,&mouseX,&mouseY, &mouseDriveDirection);

volatile Point shortestPath[gSizeMouseMaze * gSizeMouseMaze];
uint8_t Index = TryShortestPath(maze,mousemaze,shortestPath,&mouseX,&mouseY,&mouseDriveDirection);

printf("Index: %u\n",Index);

//RHA f�r Debugging
if(Index == 255){
	Board_LED_Off(0);
	Board_LED_Off(1);
	Board_LED_Off(2);
	return;
}


GetDriveCommandsSmooth(&mouseX, &mouseY, shortestPath, &mouseDriveDirection,Index);

Board_LED_On(0);
Board_LED_On(1);
Board_LED_On(2);

}

void InitMaze(TMaze maze) {
         
//    for(int i = 0; i < 33; i++){
//        for( int j = 0; j < 33; j++){
//            maze[i][j] = ' ';
//        }
//    }
//    
     strcpy(maze[0], "*********************************");
     strcpy(maze[1], "*                               *");
     strcpy(maze[2], "************* ***** *** ***** * *");
     strcpy(maze[3], "*             *   *   * * * * * *");
     strcpy(maze[4], "* * ******* *** * ***** * * * * *");
     strcpy(maze[5], "* * *   * *   * * *   *   *   * *");
     strcpy(maze[6], "* * * * * ***** * * * ********* *");
     strcpy(maze[7], "* * * * * *   * * * * *     *   *");
     strcpy(maze[8], "* * * * * * ***** * * * *** * * *");
     strcpy(maze[9], "*     * * *       * *   *   * * *");
    strcpy(maze[10], "* ***** * ******* * ***** *** * *");
    strcpy(maze[11], "*   * *         *   *   *     * *");
    strcpy(maze[12], "* * * * ******* ***** ******* ***");
    strcpy(maze[13], "* *   * *   *       *       *   *");
    strcpy(maze[14], "* ***** * * * ***** ***** * * * *");
    strcpy(maze[15], "* *   * * * * *   * *     * * * *");
    strcpy(maze[16], "* * * * * * * *   * * ***** * * *");
    strcpy(maze[17], "*   * * * * * *   * *   *     * *");
    strcpy(maze[18], "* ***** * * * *** * ***** ***** *");
    strcpy(maze[19], "* *   *     * *     *   * *     *");
    strcpy(maze[20], "* * * ********* ***** ***** * * *");
    strcpy(maze[21], "* * *               *     * * * *");
    strcpy(maze[22], "* * ********* *********** * * * *");
    strcpy(maze[23], "* *         * *         * * * * *");
    strcpy(maze[24], "* ********* * ******* * * * * * *");
    strcpy(maze[25], "* *   *   * *       * * *       *");
    strcpy(maze[26], "* * ***** * ********* * * *******");
    strcpy(maze[27], "*         * *   *   * *         *");
    strcpy(maze[28], "***** *** * * * * * * * *** *****");
    strcpy(maze[29], "*   *     *   *   *   *     *   *");
    strcpy(maze[30], "* ******* ************* *********");
    strcpy(maze[31], "*                               *");
    strcpy(maze[32], "*********************************");
	
}

enum directions DecideDirection(uint8_t const tempValues[gPossibleDirections],
	uint8_t* mouseX, uint8_t* mouseY, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze])
{	
    uint8_t INT_MAX = 255;
	uint8_t tmp = INT_MAX;
	enum directions getDirection = none;
	Point pos;

	////Code mit Wanderkennung
	if (tempValues[1] <= tmp && mousemaze[*mouseY][*mouseX].right == false) {
		tmp = tempValues[1];
		getDirection = rightt;
	}
	if (tempValues[2] <= tmp && mousemaze[*mouseY][*mouseX].bottom == false) {
		tmp = tempValues[2];
		getDirection = bottom;
	}
	if (tempValues[3] <= tmp && mousemaze[*mouseY][*mouseX].left == false) {
		tmp = tempValues[3];
		getDirection = leftt;
	}
	if (tempValues[0] <= tmp && mousemaze[*mouseY][*mouseX].top == false) {
		tmp = tempValues[0];
		getDirection = top;
	}

	return getDirection;
}

void AddNextFieldToShortestPath(Point fastestPath[gSizeMouseMaze * gSizeMouseMaze], uint8_t Index,enum directions mouseDriveDirection,
	uint8_t* mouseX, uint8_t* mouseY) {

	if (mouseDriveDirection == top) {
		fastestPath[Index].x = *mouseX;
		fastestPath[Index].y = *mouseY - 1;
		*mouseY = *mouseY -1;
	}
	else if (mouseDriveDirection == bottom) {
		fastestPath[Index].x = *mouseX;
		fastestPath[Index].y = *mouseY + 1;
		*mouseY = *mouseY + 1;
	}
	else if (mouseDriveDirection == leftt) {
		fastestPath[Index].x = *mouseX - 1;
		fastestPath[Index].y = *mouseY;
		*mouseX = *mouseX - 1;
	}
	else if (mouseDriveDirection == rightt) {
		fastestPath[Index].x = *mouseX + 1;
		fastestPath[Index].y = *mouseY;
		*mouseX = *mouseX + 1;
	}

}

void GetShortestPath(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], Point shortestPath[gSizeMouseMaze * gSizeMouseMaze]) {

	//Start bis Ziel auf welches Feld genau muss nochmal gut �berlegt werden!!!!!
	int startValue = mousemaze[0][0].wayLength;
	shortestPath[startValue].x = 0;
	shortestPath[startValue].y = 0;

	uint8_t mouseX = 0;
	uint8_t mouseY = 0;
	
	

	enum directions mouseDriveDirection;

	while (startValue != 0) {
		uint8_t tempValues[4] = { 255, 255, 255, 255 };
		startValue--;
		//Bestimmen des kleinsten Wertes der umliegenden Felder
		GetSmallestValue(mousemaze, tempValues, mouseX, mouseY);
		mouseDriveDirection = DecideDirection(tempValues, &mouseX, &mouseY, mousemaze);

		AddNextFieldToShortestPath(shortestPath, startValue, mouseDriveDirection, &mouseX, &mouseY);
	}
}

enum directions GetDirectionReverse(Point path[gSizeMouseMaze * gSizeMouseMaze],Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],
Point* currentPos, uint8_t* Index, enum directions const mouseDriveDirection) {

	Point nextPos = path[(*Index)];
	*Index = *Index +1;
	enum directions getDirection = none;
	
	volatile int DebugNextPosX = nextPos.x;
	volatile int DebugNextPosY = nextPos.y;
	
	if(nextPos.x == currentPos->x && nextPos.y == currentPos->y){
		nextPos = path[(*Index)];
		*Index = *Index+1;
		DebugNextPosX = nextPos.x;
	  DebugNextPosY = nextPos.y;
	}		

	if (currentPos->x - nextPos.x == 1 && mousemaze[currentPos->y][currentPos->x].left != true) {
		getDirection = leftt;
	}
	else if (currentPos->x - nextPos.x == -1 && mousemaze[currentPos->y][currentPos->x].right != true) {
		getDirection = rightt;
	}
	else if (currentPos->y - nextPos.y == 1 && mousemaze[currentPos->y][currentPos->x].top != true) {
		getDirection = top;
	}
	else if (currentPos->y - nextPos.y == -1 && mousemaze[currentPos->y][currentPos->x].bottom != true) {
		getDirection = bottom;
	}
	else {
		//HitWall
		getDirection = none;
	}
	
	RotateMouse(mouseDriveDirection,getDirection);
	//wird vielleicht nicht benötigt
	RefreshBehindWalls(mousemaze, *currentPos, &currentPos->x, &currentPos->y, getDirection);
	
	return getDirection;

}

bool IsCurrentPosOnShortestPath(Point* currentPos, uint8_t* Index,Point shortestPath[gSizeMouseMaze * gSizeMouseMaze]) {
	volatile Point position = shortestPath[0];
	*Index = 0;

	while (!(position.x == 0 && position.y == 0)) {
		*Index = *Index + 1;
		if (currentPos->x == position.x && currentPos->y == position.y) {
			*Index = *Index - 1;
			return true;
		}
		position = shortestPath[*Index];
	}
	//zur�ck zum Ausgangspunkt(Ziel) fahren und nochmal probieren
	*Index = 1;
	return false;
}

void CompensatePositon(Point *currentPos,Point shortestPath[gSizeMouseMaze * gSizeMouseMaze],enum directions * currentMouseDirection){
    
    if(currentPos->x < shortestPath[0].x){
        RotateMouse(*currentMouseDirection,rightt);
        Drive(rightt,&currentPos->x,&currentPos->y);
				*currentMouseDirection = rightt;
    }
    else if(currentPos->x > shortestPath[0].x){
        RotateMouse(*currentMouseDirection,leftt);
        Drive(leftt,&currentPos->x,&currentPos->y);
				*currentMouseDirection = leftt;
    }
    
    if(currentPos->y < shortestPath[0].y){
        RotateMouse(*currentMouseDirection,bottom);
        Drive(bottom,&currentPos->x,&currentPos->y);
				*currentMouseDirection = bottom;
    }
    else if(currentPos->y > shortestPath[0].y){
        RotateMouse(*currentMouseDirection,top);
        Drive(top,&currentPos->x,&currentPos->y);
				*currentMouseDirection = top;
    }
    
}
//This function evaluates the current shortest Path from start to goal. Then the function tries to drive this path from goal to start.
//If the mouse reaches the start the shortest Path possible is found.
uint8_t TryShortestPath(TMaze maze, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], Point shortestPath[gSizeMouseMaze * gSizeMouseMaze], 
                        uint8_t* mouseX, uint8_t* mouseY,enum directions * currentMouseDirection) {

	enum directions getNextDirection = *currentMouseDirection;
	enum directions getNextDirectionOld = none;
	uint8_t Index = 0;
	//Berechnet den kürzesten Weg, startend vom Start und geht dann immer zum nächsten Feld
	// bei dem wayLength 1 kleiner ist.
  GetShortestPath(mousemaze,shortestPath);
	Point currentPos = {*mouseX,*mouseY};
    
	if(currentPos.x != shortestPath[0].x || currentPos.y != shortestPath[0].y){
			//Falls die aktuelle Position nicht mit der Position 0 vom ShortestPath �bereinstimmt muss die Maus
			//an die Stelle vom shortestPath umpositioniert werden.
			CompensatePositon(&currentPos,shortestPath,&getNextDirection);
			*currentMouseDirection = getNextDirection;
		
			printf("CompensatePosition");
	}

	Point oldShortestPath[gSizeMouseMaze * gSizeMouseMaze];
	uint8_t oldIndex = 0;
	bool onShortestPath = false;

	do {
		getNextDirection = *currentMouseDirection;
		getNextDirection = GetDirectionReverse(shortestPath,mousemaze,&currentPos,&Index,getNextDirection);
		if (getNextDirection == none) {

			oldIndex = Index-1;
			for (uint8_t i = 0; i < oldIndex; i++) {
				oldShortestPath[oldIndex-1-i] = shortestPath[i];
			}

			//drive back to center and calculate new shortest Path
			//hier im Code wird currentPos einfach resetet
			GetShortestPath(mousemaze,shortestPath);
			//Pruefen, ob die aktuelle Position im ShortestPath vorkommt ansonsten zurueckfahren und vom Ziel neu beginnen
			if (IsCurrentPosOnShortestPath(&currentPos, &Index,shortestPath) == false) {
				//zur�ck zum Ausgangspunkt(Ziel) fahren und nochmal probieren
				for (uint8_t j = 1; j < oldIndex; j=j) {
					if(getNextDirectionOld == none){
						getNextDirectionOld = *currentMouseDirection;
					}
					getNextDirectionOld = GetDirectionReverse(oldShortestPath,mousemaze, &currentPos, &j,getNextDirectionOld);
					
					if (j <= oldIndex) {     // nur fahren, wenn noch nicht am Ende
						Drive(getNextDirectionOld, &currentPos.x, &currentPos.y);
						
						printf("Drive Back to Center");
						//Delay_ms(10);
						Point p = {currentPos.x,currentPos.y};
						//PrintMouseLabyrinth(mousemaze, p);
						
						//RHA falls die Maus in der Mitte ist und die Position nicht passt
						//Passt aktuell noch nicht weil shortestPath nicht korrekt ist?
						if((currentPos.x == 7 || currentPos.x ==8) && (currentPos.y == 7 || currentPos.y ==8)){
							if(currentPos.x != shortestPath[0].x || currentPos.y != shortestPath[0].y){
									//Falls die aktuelle Position nicht mit der Position 0 vom ShortestPath �bereinstimmt muss die Maus
									//an die Stelle vom shortestPath umpositioniert werden.
									CompensatePositon(&currentPos,shortestPath,&getNextDirectionOld);
									*currentMouseDirection = getNextDirectionOld;
							}
						}
						
						//nach jedem Fahren prüfen, ob man vielleicht auf dem shortestPath ist.
						if(IsCurrentPosOnShortestPath(&currentPos, &Index,shortestPath) == true){
							onShortestPath = true;
							break;
						}
					}
				}
				*currentMouseDirection = getNextDirectionOld;
				getNextDirectionOld = none;
				if(onShortestPath){
					onShortestPath = false;
					continue;
				}
                
			}
			else {
				continue;
			}

		}
		if(getNextDirection != none){
			*currentMouseDirection = getNextDirection;
		}
		Drive(getNextDirection, &currentPos.x, &currentPos.y);
		
		printf("Drive fastest Path");
		//Delay_ms(10);
		Point p = {currentPos.x,currentPos.y};
		//PrintMouseLabyrinth(mousemaze, p);
		
		SetWalls(maze, mousemaze, currentPos.x, currentPos.y, getNextDirection);
		RefreshBehindWalls(mousemaze, currentPos, &currentPos.x, &currentPos.y, getNextDirection);

	} 
	while (!(currentPos.x == 0 && currentPos.y == 0));
	
	*mouseX = 0;
	*mouseY = 0;
	
  return Index - 1;
	
}


