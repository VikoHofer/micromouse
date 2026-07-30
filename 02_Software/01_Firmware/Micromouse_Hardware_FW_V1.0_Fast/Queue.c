#include "Queue.h"

// Geht wahrscheinlich nicht weil Heap 0x10 ist im startup
// wird aber auch nicht benötigt
//Queue* createQueue(void) {
//    Queue* queue = (Queue*)malloc(sizeof(uint16_t)*3 + sizeof(Point)*MAX_SIZE);
//    queue->front = 0;
//    queue->rear = -1;
//    queue->size = 0;
//    return queue;
//}

void InitQueue (Queue* queue){
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
    
}

int isEmpty(Queue* queue) {
    if(queue->size != 0){
        return 0;
    }
    else{
        return 1;
    }
    
}

int isFull(Queue* queue) {
    return queue->size == MAX_SIZE;
}

void push(Queue* queue, Point item) {
    if (isFull(queue)) {
        //printf("Queue is full\n");
        return;
    }
    queue->rear = (queue->rear + 1) % MAX_SIZE;
    queue->Field[queue->rear] = item;
    queue->size++;
}

Point dequeue(Queue* queue) {
    if (isEmpty(queue)) {
        //printf("Queue is empty\n");
        Point p;
        p.x = 0;
        p.y = 0;
        return p;
    }
    Point dequeuedItem = queue->Field[queue->front];
    queue->front = (queue->front + 1) % MAX_SIZE;
    queue->size--;
    return dequeuedItem;
}



