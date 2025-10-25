/*
 * Queue
 *
 *  Created on: Oct 3, 2025
 *      Author: jabel
 */

#ifndef INC_QUEUE_H_
#define INC_QUEUE_H_

/***********************************************
*      INCLUIR LIBRERIAS
* *********************************************/



/**********************************************
*     DEFINICIÓN DE CONSTANTES
* ********************************************/

#ifndef NULL
#define NULL ((void*)0)
#endif

#define QUEUE_BASE_ERR              0x0100

#define QUEUE_OK                    0
#define QUEUE_ERR_NULL_PARAM        QUEUE_BASE_ERR | 0x00FF
#define QUEUE_ERR_EMPTY             QUEUE_BASE_ERR | 0x00FE


/*********************************************
*    DEFINICIÓN DE TIPOS
* *******************************************/

typedef struct QElement
{
   void * Data;
   struct QElement * Next;
}QueueElement_t, * QueueElement_t_ptr;

typedef struct
{
   QueueElement_t_ptr Head;
   QueueElement_t_ptr Tail;
   unsigned int Count;
}QueueHandler_t, * QueueHandler_t_ptr;

/********************************************
*   DEFINICIÓN DE FUNCIONES
* ******************************************/

short Queue_Init(QueueHandler_t_ptr Queue);
unsigned int Queue_GetCount(QueueHandler_t_ptr Queue);
short Queue_Enqueue(QueueHandler_t_ptr Queue, QueueElement_t_ptr Element, void * Data);
short Queue_Dequeue(QueueHandler_t_ptr Queue, void ** Data);
short Queue_Copy(QueueHandler_t_ptr Or, QueueHandler_t_ptr De);


#endif /* INC_QUEUE_H_ */
