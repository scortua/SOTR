/***********************************************
 *      INCLUIR LIBRERIAS
 * *********************************************/

#include "Queue.h"
#include "AppTypes.h"
#include "Portable.h"

/**********************************************
 *     DEFINICIÓN DE CONSTANTES INTERNAS
 * ********************************************/


/*********************************************
 *    DEFINICIÓN DE TIPOS INTERNOS
 * *******************************************/


/********************************************
 *   DEFINICIÓN DE FUNCIONES INTERNOS
 * ******************************************/


/********************************************
 *   DEFINICIÓN DE VARIABLES GLOBALES INTENRNAS
 * ******************************************/


/********************************************
 *   IMPLEMENTACIÓN DE FUNCIONES PUBLICAS
 * ******************************************/

short Queue_Init(QueueHandler_t_ptr Queue)
{
    short Res = QUEUE_ERR_NULL_PARAM;
    Port_DisableInterrupts();
    if(Queue != NULL)
    {
        Queue->Tail = NULL;
        Queue->Head = NULL;
        Queue->Count = 0;
        Res = QUEUE_OK;
    }
    Port_EnableInterrupts();
    return Res;
}

unsigned int Queue_GetCount(QueueHandler_t_ptr Queue)
{
    if(Queue != NULL)
    {
        return Queue->Count;
    }
    return 0;
}

short Queue_Enqueue(QueueHandler_t_ptr Queue, QueueElement_t_ptr Element, void * Data)
{
    short Res = QUEUE_ERR_NULL_PARAM;
    Port_DisableInterrupts();
    if((Queue != NULL) && (Element != NULL))
    {
        Element->Data = Data;
        Element->Next = NULL;
        if(Queue->Tail == NULL)
        {
            Queue->Head = Element;
            Queue->Tail = Element;
            Queue->Count = 1;
        }
        else
        {
            Queue->Tail->Next = Element;
            Queue->Tail = Element;
            Queue->Count++;
        }
        Res = QUEUE_OK;
    }
    Port_EnableInterrupts();
    return Res;
}

short Queue_Dequeue(QueueHandler_t_ptr Queue, void ** Data)
{
    short Res = QUEUE_ERR_NULL_PARAM;
    Port_DisableInterrupts();
    if((Queue != NULL) && (Data != NULL))
    {
        if(Queue->Head != NULL)
        {
            QueueElement_t_ptr Element;
            Element = Queue->Head;
            *Data = Element->Data;
            if(Element->Next != NULL)
            {
                Queue->Head = Queue->Head->Next;
                Queue->Count--;
            }
            else
            {
                Queue->Head = NULL;
                Queue->Tail = NULL;
                Queue->Count = 0;
            }
            Res = QUEUE_OK;
        }
        else
        {
            Res = QUEUE_ERR_EMPTY;
        }
    }
    Port_EnableInterrupts();
    return Res;
}

short Queue_Copy(QueueHandler_t_ptr Or, QueueHandler_t_ptr De)
{
	short Res = QUEUE_ERR_NULL_PARAM;
	Port_DisableInterrupts();
	if((Or != NULL) && (De != NULL))
	{
		De->Count = Or->Count;
		De->Head = Or->Head;
		De->Tail = Or->Tail;
		Res = QUEUE_OK;
	}
	Port_EnableInterrupts();
	return Res;
}

/********************************************
 *   IMPLEMENTACIÓN DE FUNCIONES PRIVADAS
 * ******************************************/
