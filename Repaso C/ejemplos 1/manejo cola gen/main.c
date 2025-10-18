#include <stdio.h>
#include "Queue.h"

int main()
{
    short Res;
    QueueHandler_t Queue;
    QueueElement_t Element1;
    QueueElement_t Element2;
    QueueElement_t Element3;
    printf("Queue Test\n");
    Res = Queue_Init(&Queue);
    if(Res == QUEUE_OK)
    {
        void * Param;
        printf("Queue Init OK\n");
        printf("Queue Actual Elements: %d\n", Queue_GetCount(&Queue));
        printf("Queue Enqueue Elements: 0x%04X\n", Queue_Enqueue(&Queue, &Element1, (void *)5));
        printf("Queue Actual Elements: %d\n", Queue_GetCount(&Queue));
        printf("Queue Enqueue Elements: 0x%04X\n", Queue_Enqueue(&Queue, &Element2, (void *)10));
        printf("Queue Actual Elements: %d\n", Queue_GetCount(&Queue));
        printf("Queue Enqueue Elements: 0x%04X\n", Queue_Enqueue(&Queue, &Element3, (void *)15));
        printf("Queue Actual Elements: %d\n", Queue_GetCount(&Queue));
        
        
        printf("Queue Dequeue Elements: 0x%04X\n", Queue_Dequeue(&Queue, &Param));
        printf("Queue Actual Elements: %d\n", Queue_GetCount(&Queue));
        printf("Actual Element: %d\n", Param);
        printf("Queue Dequeue Elements: 0x%04X\n", Queue_Dequeue(&Queue, &Param));
        printf("Queue Actual Elements: %d\n", Queue_GetCount(&Queue));
        printf("Actual Element: %d\n", Param);
        printf("Queue Dequeue Elements: 0x%04X\n", Queue_Dequeue(&Queue, &Param));
        printf("Queue Actual Elements: %d\n", Queue_GetCount(&Queue));
        printf("Actual Element: %d\n", Param);
        printf("Queue Dequeue Elements: 0x%04X\n", Queue_Dequeue(&Queue, &Param));
        printf("Queue Actual Elements: %d\n", Queue_GetCount(&Queue));
        printf("Actual Element: %d\n", Param);
    }
    else
    {
        printf("Queue Init Fail, Error: 0x%04X\n", Res);
    }
    return 0;
}