#include "FloodFill.h"
#include "Refresh.h"
#include <string.h>
#include "BOARD_LED.h"
#include <stdio.h>
#include "sensor.h"
#include "motion_control.h"
#include "Delay.h"


uint8_t const gSizeMouseMaze = 16;
uint8_t const gSizeLabyrinth = 33;
uint8_t const gPossibleDirections = 4;
Field mouseMaze[16][16];
TMaze maze;

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
            Board_LED_On();
			return false;
		}
	}
	return true;
}

void UpdateWallTop(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],uint8_t const mouseX, uint8_t const mouseY){
    mousemaze[mouseY][mouseX].top = true;
    if (mouseY != 0) {
        mousemaze[mouseY - 1][mouseX].bottom = true;
    }
}

void UpdateWallRight(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],uint8_t const mouseX, uint8_t const mouseY){
    mousemaze[mouseY][mouseX].right = true;
    if (mouseX != gSizeMouseMaze - 1) {
        mousemaze[mouseY][mouseX + 1].left = true;
    }
}

void UpdateWallBottom(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],uint8_t const mouseX, uint8_t const mouseY){
    mousemaze[mouseY][mouseX].bottom = true;
    if (mouseY != gSizeMouseMaze - 1) {
        mousemaze[mouseY + 1][mouseX].top = true;
    }
}

void UpdateWallLeft(Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],uint8_t const mouseX, uint8_t const mouseY){
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
    uint32_t const maxValue = 2500;
    
    //zweimal weil erster Aufruf Pfusch ist
    //GetValuesForMouse(&rightValue,&middleValue,&leftValue);
    
    // PA0
    // duration 5ms!!
    leftValue = readSensor_Left_45();

    // PA1
    // duration 5ms!!
    rightValue = readSensor_Right_45();

    // PB0
    middleValue = readSensor_Front();
    
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
        
        if( i == 0 && mousemaze[newY][newX].bottom){continue;}
        if( i == 1 && mousemaze[newY][newX].left){continue;}
        if( i == 2 && mousemaze[newY][newX].top){continue;}
        if( i == 3 && mousemaze[newY][newX].right){continue;}

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
			RefreshNew(pos, mousemaze);
		}
		if (mousemaze[*mouseY][*mouseX].bottom == true && (*mouseY + 1) < (gSizeMouseMaze - 1)) {
			pos.x = *mouseX;
			pos.y = *mouseY + 1;
			RefreshNew(pos, mousemaze);
		}
		if (mousemaze[*mouseY][*mouseX].left == true && (*mouseX) > 0) {
			pos.x = *mouseX - 1;
			pos.y = *mouseY;
			RefreshNew(pos, mousemaze);
		}
		if (mousemaze[*mouseY][*mouseX].top == true && (*mouseY) > 0) {
			pos.x = *mouseX;
			pos.y = *mouseY - 1;
			RefreshNew(pos, mousemaze);
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
    
    //�NDERUNG HARDWARE MAUS 
    //ROTATE MAUS
    //Funktion f�r Motor
    if(rotate == 1){
        //TODO auf anderes tunright �ndern
       turn(1);
    }
    else if (rotate == 2){
       turn(3);
    }
    else if (rotate == 3){
       turn(2);
    }
}

enum directions DecideDriveDirection(uint8_t const tempValues[gPossibleDirections], uint8_t smallestTemp,
	uint8_t * mouseX, uint8_t * mouseY, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], enum directions const mouseDriveDirection)
{
	enum directions getDirection = none;
    
	Point pos = { 0,0 };
    
    //Code mit Wanderkennung
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
    else {
        pos.x = *mouseX;
        pos.y = *mouseY;
        RefreshNew(pos, mousemaze);
        smallestTemp++;
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
    move(1);

}

//HARDWARE MAUS
void ScanCenter3Right(TMaze maze, uint8_t * mouseX, uint8_t * mouseY,Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],enum directions * currentMouseDirection){
    
    for(uint8_t i = 0; i < 4;i++){
        Drive(*currentMouseDirection,mouseX,mouseY);
        SetWalls(maze, mousemaze, *mouseX, *mouseY,*currentMouseDirection);
        Point p;
        p.x = *mouseX;
        p.y = *mouseY;
        RefreshBehindWalls(mousemaze, p, mouseX, mouseY, *currentMouseDirection);
        
        //Beim letzten mal nicht mehr updaten
        if(i != 3){
            enum directions updateDirection = *currentMouseDirection;
            if(updateDirection == leftt){
                updateDirection = top;
            }else{
                updateDirection++;
            }
            
            *currentMouseDirection = updateDirection;
        }
    }
    
}


//HARDWARE MAUS
void ScanCenter3Left(TMaze maze, uint8_t * mouseX, uint8_t * mouseY,Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],enum directions * currentMouseDirection){
    
    for(uint8_t i = 0; i < 4;i++){
        Drive(*currentMouseDirection,mouseX,mouseY);
        SetWalls(maze, mousemaze, *mouseX, *mouseY,*currentMouseDirection);
        Point p;
        p.x = *mouseX;
        p.y = *mouseY;
        RefreshBehindWalls(mousemaze, p, mouseX, mouseY, *currentMouseDirection);
        
        //Beim letzten mal nicht mehr updaten
        if(i != 3){
            enum directions updateDirection = *currentMouseDirection;
            if(updateDirection == top){
                updateDirection = leftt;
            }else{
                updateDirection--;
            }
            
            *currentMouseDirection = updateDirection;
        }
    }
    
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


void SolveMaze(TMaze maze, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze]){
    
uint8_t mouseX = 0;
uint8_t mouseY = 0;
    
//Waylength Werte der umliegenden Felder
uint8_t tempValues[4] = { 0 };

//SOFTWARTE MAUS
enum directions mouseDriveDirection = rightt;
    
//�NDERUNG HARDWARE MAUS
//Je nach Schalter Stellung wird die Richtung ausgew�hlt
//if(){
//    mouseDriveDirection = rightt;
//}
//else{
//    mouseDriveDirection = bottom;
//}
    

//Schleife bis Ziel gefunden wird
while (FoundGoal(&mouseX, &mouseY)) { // mouseDriveDirection none gehoert entfernt!!!

    //Maus soll vor der Messung stehen
    Delay_ms(10000);
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

}

////Die Mitte fertig abscannen ist noch nicht fertig
////FUNKTIONIERT nicht sicher in allen F�llen. Beispiel Ziel wird bei 8,8 gefunden und 
//bei Feld 7,7 sind keine W�nde Fehler k�nnen dann beim ShortestPath auftreten.
ScanCenter(maze,mousemaze, tempValues,&mouseX,&mouseY, &mouseDriveDirection);

//K�rzesten Weg finden und ausprobieren
Point shortestPath[gSizeMouseMaze * gSizeMouseMaze];
TryShortestPath(maze,mousemaze,shortestPath,&mouseX,&mouseY,&mouseDriveDirection);
    
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

	uint8_t tempValues[4] = { 0 };

	while (startValue != 0) {
		startValue--;
		//Bestimmen des kleinsten Wertes der umliegenden Felder
		GetSmallestValue(mousemaze, tempValues, mouseX, mouseY);
		mouseDriveDirection = DecideDirection(tempValues, &mouseX, &mouseY, mousemaze);

		AddNextFieldToShortestPath(shortestPath, startValue, mouseDriveDirection, &mouseX, &mouseY);
	}
}

enum directions GetDirectionReverse(Point path[gSizeMouseMaze * gSizeMouseMaze],Field mousemaze[gSizeMouseMaze][gSizeMouseMaze],
Point* currentPos, uint8_t* Index) {

	Point nextPos = path[*Index];
	*Index = *Index +1;

	if (currentPos->x - nextPos.x == 1 && mousemaze[currentPos->y][currentPos->x].left != true) {
		return leftt;
	}
	else if (currentPos->x - nextPos.x == -1 && mousemaze[currentPos->y][currentPos->x].right != true) {
		return rightt;
	}
	else if (currentPos->y - nextPos.y == 1 && mousemaze[currentPos->y][currentPos->x].top != true) {
		return top;
	}
	else if (currentPos->y - nextPos.y == -1 && mousemaze[currentPos->y][currentPos->x].bottom != true) {
		return bottom;
	}
	else {
		//HitWall
		return none;
	}

}

bool IsCurrentPosOnShortestPath(Point* currentPos, uint8_t* Index,Point shortestPath[gSizeMouseMaze * gSizeMouseMaze]) {
	Point position = shortestPath[0];

	while (!(position.x == 0 && position.y == 0)) {
		*Index = *Index + 1;
		if (currentPos->x == position.x && currentPos->y == position.y) {
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
    }
    else if(currentPos->x > shortestPath[0].x){
        RotateMouse(*currentMouseDirection,leftt);
        Drive(leftt,&currentPos->x,&currentPos->y);
    }
    
    if(currentPos->y < shortestPath[0].y){
        RotateMouse(*currentMouseDirection,bottom);
        Drive(bottom,&currentPos->x,&currentPos->y);
        
    }
    else if(currentPos->y > shortestPath[0].y){
        RotateMouse(*currentMouseDirection,top);
        Drive(top,&currentPos->x,&currentPos->y);
    }
    
}

void TryShortestPath(TMaze maze, Field mousemaze[gSizeMouseMaze][gSizeMouseMaze], Point shortestPath[gSizeMouseMaze * gSizeMouseMaze], 
                        uint8_t* mouseX, uint8_t* mouseY,enum directions * currentMouseDirection) {

	enum directions getNextDirection = *currentMouseDirection;
	enum directions getNextDirectionOld = none;
	uint8_t Index = 0;
    GetShortestPath(mousemaze,shortestPath);
	Point currentPos = {*mouseX,*mouseY};
    
    if(currentPos.x != shortestPath[0].x || currentPos.y != shortestPath[0].y){
        //Falls die aktuelle Position nicht mit der Position 0 vom ShortestPath �bereinstimmt muss die Maus
        //an die Stelle vom shortestPath umpositioniert werden.
        CompensatePositon(&currentPos,shortestPath,&getNextDirection);
    }

	Point oldShortestPath[gSizeMouseMaze * gSizeMouseMaze];
	uint8_t oldIndex = 0;

	do {
		getNextDirection = GetDirectionReverse(shortestPath,mousemaze,&currentPos,&Index);
		if (getNextDirection == none) {

			oldIndex = Index-1;
			for (uint8_t i = 0; i < oldIndex; i++) {
				oldShortestPath[oldIndex-1-i] = shortestPath[i];
			}

			//drive back to center and calculate new shortest Path
			//hier im Code wird currentPos einfach resetet
			GetShortestPath(mousemaze,shortestPath);
			//Pr�fen, ob die aktuelle Position im ShortestPath vorkommt ansonsten zur�ckfahren und vom Ziel neu beginnen
			if (IsCurrentPosOnShortestPath(&currentPos, &Index,shortestPath) == false) {
				//zur�ck zum Ausgangspunkt(Ziel) fahren und nochmal probieren
				for (uint8_t j = 1; j < oldIndex; j=j) {
					getNextDirectionOld = GetDirectionReverse(oldShortestPath,mousemaze, &currentPos, &j);
					Drive(getNextDirectionOld, &currentPos.x, &currentPos.y);
				}
                
			}
            if(currentPos.x != shortestPath[0].x || currentPos.y != shortestPath[0].y){
                //Falls die aktuelle Position nicht mit der Position 0 vom ShortestPath �bereinstimmt muss die Maus
                //an die Stelle vom shortestPath umpositioniert werden.
                CompensatePositon(&currentPos,shortestPath,&getNextDirection);
            }


		}
		Drive(getNextDirection, &currentPos.x, &currentPos.y);
		SetWalls(maze, mousemaze, currentPos.x, currentPos.y, getNextDirection);
		RefreshBehindWalls(mousemaze, currentPos, &currentPos.x, &currentPos.y, getNextDirection);

	} 
	while (!(currentPos.x == 0 && currentPos.y == 0));

    Toggle_Board_LED();
	
}


