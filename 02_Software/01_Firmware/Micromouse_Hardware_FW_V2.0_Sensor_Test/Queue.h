#ifndef QUEUE_H
#define QUEUE_H


#include <stdio.h>
#include <stdlib.h>
//MACHT EVENTUELL PROBLEME????
#include "FloodFill.h"

#define MAX_SIZE 1024

typedef struct {
    Point Field[MAX_SIZE];
    uint16_t front;
    uint16_t rear;
    uint16_t size;
} Queue;

//Queue* createQueue(void);
void InitQueue (Queue* queue);
int isEmpty(Queue* queue);
int isFull(Queue* queue);
void push(Queue* queue, Point pos);
Point dequeue(Queue* queue);

#endif
