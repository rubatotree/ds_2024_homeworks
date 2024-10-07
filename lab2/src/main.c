#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#define MAX 200010

// Math functions
int random_int()
{
	return rand() * RAND_MAX + rand();
}

int random_range(int l, int r)
{
	return l + random_int() % (r - l);
}

// DataTypes
typedef int TimeType;
typedef int AmountType;
typedef unsigned int IndexType;
typedef struct CustNode
{
	TimeType arrtime, durtime, leavetime;
	AmountType amount; 
	IndexType next;
} CustNode, DataType;

// Data to input
size_t CHUNKSIZE = 4;
AmountType total;
TimeType closetime;
TimeType  durtime_min,  durtime_max,
		 interval_min, interval_max;
AmountType amount_min,   amount_max;
size_t event_number;

///	Cycle Queue
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

/*
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
*/

// Functions
CustNode eventlist[MAX];

void eventlist_GenerateDay1()
{
	TimeType arrtime = 0;
	for(IndexType i = 0; i < event_number; i++)
	{
		eventlist[i].next = i + 1;
		eventlist[i].arrtime = arrtime;
		eventlist[i].durtime = random_range(durtime_min, durtime_max + 1);
		eventlist[i].amount = random_range(amount_min, amount_max + 1);
		eventlist[i].leavetime = -1;

		TimeType interval = random_range(interval_min, interval_max + 1);
		arrtime += interval;
	}
}

// Main Program
void eventlist_output()
{
	printf("Event \t| Arrtime \t| Durtime \t| Amount \t| Leavetime\n");
	for(IndexType i = 0; i < event_number; i++)
	{
		printf("%u \t| %u\t| %u\t| %d\t| %u\t\n", i, eventlist[i].arrtime, eventlist[i].durtime, eventlist[i].amount, eventlist[i].leavetime);
	}
}

void process()
{
	eventlist_GenerateDay1();
	eventlist_output();
}

int main()
{
	printf("Please input the total amount:\n");
	scanf("%d\n", &total);
	printf("Please input the close time:\n");
	scanf("%d\n", &closetime);
	printf("Please input the min&max of durtime:\n");
	scanf("%d%d\n", &durtime_min, &durtime_max);
	printf("Please input the min&max of amount:\n");
	scanf("%d%d\n", &amount_min, &amount_max);
	printf("Please input the min&max of interval:\n");
	scanf("%d%d\n", &interval_min, &interval_max);
	printf("Please input the CHUNKSIZE:\n");
	scanf("%d\n", &CHUNKSIZE);
	printf("Please input the number of events in day 1\n");
	scanf("%u", &event_number);
	process();
	return 0;
}
