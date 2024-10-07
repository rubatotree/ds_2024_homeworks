#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

///	Cycle Queue
struct CustNode;
typedef struct CustNode DataType;
typedef unsigned int IndexType;

size_t CHUNKSIZE = 4;

typedef struct CyQueue
{
	size_t maxsize;
	IndexType head, tail;
	int full_flag;
	DataType* data;
} CyQueue;

CyQueue* CyQueue_Init()
{
	size_t size = CHUNKSIZE;
	CyQueue* q = (CyQueue*)malloc(sizeof(CyQueue));
	q->maxsize = size;
	q->head = 0;
	q->tail = 0;
	q->full_flag = 0;
	q->data = (DataType*)malloc(size * sizeof(DataType));
}

void CyQueue_Delete(CyQueue* q)
{
	free(q->data);
	free(q);
}

void CyQueue_Clear(CyQueue* q)
{
	q->head = 0;
	q->tail = 0;
	q->full_flag = 0;
}

size_t CyQueue_Size(CyQueue* q)
{
	if(q->tail == q->head && q->full_flag)
		return q->maxsize;
	return (q->tail - q->head + q->maxsize) % q->maxsize;
}

int CyQueue_Empty(CyQueue* q)
{
	return q->head == q->tail && !q->full_flag;
}

DataType CyQueue_At(CyQueue* q, IndexType ind)
{
	assert(ind < CyQueue_Size(q));
	return q->data[(q->head + ind) % q->maxsize];
}

void CyQueue_Resize(CyQueue* q, size_t newsize)
{
	// The overflowed data will be deleted.
	DataType* newdata = (DataType*)malloc(newsize * sizeof(DataType));
	size_t size = CyQueue_Size(q);
	size_t copy_size = newsize < size ? newsize : size;
	for(IndexType i = 0; i < copy_size; i++)
		newdata[i] = CyQueue_At(q, i);
	q->head = 0;
	q->maxsize = newsize;
	if(copy_size == newsize) 
	{
		// Queue Full
		q->tail = 0;
		q->full_flag = 1;
	}
	else
	{
		// Queue Not Full
		q->tail = copy_size;
		q->full_flag = 0;
	}
	free(q->data);
	q->data = newdata;
}

void CyQueue_Push(CyQueue* q, DataType data)
{
	size_t size = CyQueue_Size(q);
	if(size == q->maxsize)
	{
		// Resize First
		size += CHUNKSIZE;
		CyQueue_Resize(q, size);
	}
	q->data[q->tail] = data;
	q->tail++;
	q->tail %= q->maxsize;
	if(q->tail == q->head) q->full_flag = 1;
}

DataType CyQueue_Pop(CyQueue* q)
{
	assert(!CyQueue_Empty(q));
	DataType data = q->data[q->head];
	if(q->tail == q->head) q->full_flag = 0;
	q->head++;
	q->head %= q->maxsize;
	return data;
}

DataType CyQueue_Front(CyQueue* q)
{
	assert(!CyQueue_Empty(q));
	return q->data[q->head];
}

void CyQueue_PrintData(DataType data)
{
	printf("%d", data);
}

void CyQueue_Print(CyQueue* q)
{
	size_t size = CyQueue_Size(q);
	printf("[%u/%u] ", size, q->maxsize);
	for(int i = 0; i < size; i++)
	{
		CyQueue_PrintData(CyQueue_At(q, i));
		printf(" ");
	}
	printf("\n");
}

// Cust
typedef unsigned int TimeType;
typedef long long AmountType;
typedef struct CustNode
{
	TimeType arrtime, durtime, leavetime;
	AmountType amount; 
} CustNode;


int main()
{
	return 0;
}
