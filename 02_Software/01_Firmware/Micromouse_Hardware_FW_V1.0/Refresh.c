

#include "Refresh.h"
#include <limits.h>
#include "FloodFill.h"


uint8_t Refresh(Point const pos, Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze]) {
	//Queue* q = createQueue();
    Queue q;
    InitQueue(&q);
    push(&q,pos);
    
    uint8_t const INTMAX = 255;
	  volatile uint8_t tmp = INTMAX;

	while (!(isEmpty(&q))) {
		tmp = INTMAX;
        //ES WIRD DEQUEUED STATT NUR GELESEN UNTERSCHIED ZU C++
		Point const currentPos = dequeue(&q);

		uint8_t const x = currentPos.x;
		uint8_t const y = currentPos.y;

		if (mouseMaze[y][x].wayLength != 0) {

			// find smallest neighbor without wall inbetween
			if (y > 0 && mouseMaze[y - 1][x].wayLength < tmp && mouseMaze[y][x].top == false)
			{
				tmp = mouseMaze[y - 1][x].wayLength;
			}
			if (y < gSizeMouseMaze - 1 && mouseMaze[y + 1][x].wayLength < tmp && mouseMaze[y][x].bottom == false)
			{
				tmp = mouseMaze[y + 1][x].wayLength;
			}
			if (x > 0 && mouseMaze[y][x - 1].wayLength < tmp && mouseMaze[y][x].left == false)
			{
				tmp = mouseMaze[y][x - 1].wayLength;
			}
			if (x < gSizeMouseMaze - 1 && mouseMaze[y][x + 1].wayLength < tmp && mouseMaze[y][x].right == false)
			{
				tmp = mouseMaze[y][x + 1].wayLength;
			}

			// assign smallest possible value to current coordinates
			if (mouseMaze[y][x].wayLength != tmp + 1)
			{
				mouseMaze[y][x].wayLength = tmp + 1;
                Point p;
                p.x = 0;
                p.y = 0;


				if (x > 0 /*&& !mouseMaze[y][x].left*/) {
                    p.x = x-1;
                    p.y = y;
					push(&q,p);
				}
				if (x < gSizeMouseMaze - 1 /*&& !mouseMaze[y][x].right*/) {
					p.x = x+1;
                    p.y = y;
					push(&q,p);
				}
				if (y > 0 /*&& !mouseMaze[y][x].top*/) {
                    p.x = x;
                    p.y = y-1;
					push(&q,p);
				}
				if (y < gSizeMouseMaze - 1 /*&& !mouseMaze[y][x].bottom*/) {
					p.x = x;
                    p.y = y+1;
					push(&q,p);
				}
			}
		}
        //POP WIRD NICHT BENOETIGT WIRD OBEN SCHON ENTFERNT UNTERSCHIED ZU C++
		//q.pop();
		//PrintMouseLabyrinth(mouseMaze);
	}

    //free(q); q = NULL;
	return 0;
}


// Iterative Refresh-Funktion (ganz ohne Queue oder Rekursion)
uint8_t RefreshNew(Point const pos, Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze]) {
    uint8_t const INTMAX = 255;
    uint8_t tmp = INTMAX;
    
    // Initialisiere alle Zellen, um sie zu durchlaufen
    int updated = 1;  // Flag, um festzustellen, ob eine Zelle aktualisiert wurde

    // Wiederhole den Prozess, solange es �nderungen gibt
    while (updated) {
        updated = 0;  // Setze Flag zur�ck, bevor die Iteration beginnt

        // Durchlaufe das gesamte Labyrinth (Feld) iterativ
        for (uint8_t y = 0; y < gSizeMouseMaze; y++) {
            for (uint8_t x = 0; x < gSizeMouseMaze; x++) {
                if (mouseMaze[y][x].wayLength != 0) {  // Nur Zellen, die noch nicht bearbeitet sind
                    tmp = INTMAX;

                    // Benachbarte Zellen pr�fen und den kleinsten Wert finden
                    if (y > 0 && mouseMaze[y - 1][x].wayLength < tmp && !mouseMaze[y][x].top) {
                        tmp = mouseMaze[y - 1][x].wayLength;
                    }
                    if (y < gSizeMouseMaze - 1 && mouseMaze[y + 1][x].wayLength < tmp && !mouseMaze[y][x].bottom) {
                        tmp = mouseMaze[y + 1][x].wayLength;
                    }
                    if (x > 0 && mouseMaze[y][x - 1].wayLength < tmp && !mouseMaze[y][x].left) {
                        tmp = mouseMaze[y][x - 1].wayLength;
                    }
                    if (x < gSizeMouseMaze - 1 && mouseMaze[y][x + 1].wayLength < tmp && !mouseMaze[y][x].right) {
                        tmp = mouseMaze[y][x + 1].wayLength;
                    }

                    // Aktualisiere die wayLength der aktuellen Zelle, wenn der Wert ge�ndert wurde
                    if (mouseMaze[y][x].wayLength != tmp + 1) {
                        mouseMaze[y][x].wayLength = tmp + 1;
                        updated = 1;  // Es gab eine �nderung, also muss die Schleife weiterlaufen
                    }
                }
            }
        }
    }

    return 0;  // R�ckgabewert 0, wenn abgeschlossen
}

