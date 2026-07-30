#include "Refresh.h"
#include <limits.h>
#include "FloodFill.h"


uint8_t Refresh(Point const pos, Field mouseMaze[gSizeMouseMaze][gSizeMouseMaze]) {
	//Queue* q = createQueue();
    Queue q;
    InitQueue(&q);
    push(&q,pos);
    
    uint8_t const INTMAX = 255;
	uint8_t tmp = INTMAX;

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


