#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX 200010

// Random functions
int random_int()
{
	return abs(rand() * RAND_MAX + rand());
}

int random_range(int l, int r)
{
	return l + random_int() % (r - l);
}

// DataTypes
typedef int TimeType;
typedef int AmountType;
typedef unsigned int IndexType;
typedef unsigned int SizeType;
typedef struct CustNode
{
	TimeType arrtime, durtime, leavetime;
	AmountType amount; 
	struct CustNode* next;
} CustNode, *DataType;

// Data to input
SizeType CHUNKSIZE = 4;
AmountType total_initial;
TimeType closetime;
TimeType  durtime_min,  durtime_max,
		 interval_min, interval_max;
AmountType amount_min,   amount_max;
SizeType day_number;

// Cycle Queue
typedef struct CyQueue
{
	SizeType maxsize;
	IndexType head, tail;
	int full_flag;
	DataType* data;
} CyQueue;

CyQueue* CyQueue_Init()
{
	SizeType size = CHUNKSIZE;
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

SizeType CyQueue_Size(CyQueue* q)
{
	if(q->tail == q->head && q->full_flag)
		return q->maxsize;
	// (int - int + uint) % uint != (int + uint - int) % uint
	// evil data type bug
	return ((int)q->tail + (int)q->maxsize - (int)q->head) % (int)q->maxsize;
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

void CyQueue_Resize(CyQueue* q, SizeType newsize)
{
	// The overflowed data will be deleted.
	DataType* newdata = (DataType*)malloc(newsize * sizeof(DataType));
	SizeType size = CyQueue_Size(q);
	SizeType copy_size = newsize < size ? newsize : size;
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
	SizeType size = CyQueue_Size(q);
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

// Event List
CustNode eventlist[MAX];
AmountType total_amount = 0;
CustNode* eventlist_head = NULL;
CustNode* eventlist_tail = NULL;

// INITIALIZE eventlist_tail BEFORE CALLING
void eventlist_Insert(CustNode node)
{
	if(eventlist_tail == NULL)
	{
		eventlist[0] = node;
		eventlist_head = &eventlist[0];
		eventlist_tail = &eventlist[0];
		eventlist_tail->next = NULL;
	}
	else if(eventlist_tail->arrtime <= node.arrtime)
	{
		// 贪心尾插优先
		assert((eventlist_tail - eventlist + 1) < MAX);
		eventlist_tail->next = eventlist_tail + 1;
		eventlist_tail += 1;
		(*eventlist_tail) = node;
		eventlist_tail->next = NULL;
	}
	else if(eventlist_head->arrtime >= node.arrtime && (int)(eventlist_head - eventlist) > 0)
	{
		// 次之先填充头部
		eventlist[0] = node;
		eventlist[0].next = eventlist_head;
		eventlist_head = &eventlist[0];
	}
	else
	{
		CustNode* prev = NULL;
		for(CustNode* cur = eventlist_head; cur != NULL; cur = cur->next)
		{
			if(cur->arrtime > node.arrtime)
			{
				// 插入 cur 前
				if(prev != NULL && (int)(cur - prev - 1) > 0)
				{
					IndexType idx = prev + 1 - eventlist;
					eventlist[idx] = node;
					eventlist[idx].next = cur;
					if(cur == eventlist_head) 
						eventlist_head = &eventlist[idx];
					else prev->next = &eventlist[idx];
					break;
				}
				else
				{
					// 把事件表中cur往后的元素都后移一位，直到遇到空闲空间
					CustNode* nearest_idle = cur;
					while(nearest_idle->next == nearest_idle + 1)
						nearest_idle = nearest_idle->next;
					nearest_idle += 1;	// 得到最近的空闲位置
					assert(nearest_idle - eventlist <= MAX);
					// 将 [cur, nearest_idle) 整体右移一位
					// 将 nearest_idle 移到 cur 的位置
					if((int)(eventlist_tail - nearest_idle + 1) <= 0)
						eventlist_tail += 1;
					while((int)(nearest_idle - cur) > 0)
					{
						*nearest_idle = *(nearest_idle - 1); // Copy
						
						// 如果被移动的 prev 也在这段连续空间里，那么需要移动 next
						// 但如果不在的话（也就是prev变量了），那么它对应的 next 值本来就会被填充，
						// 也就不需要改 next
						if((int)(nearest_idle - 2 - cur) >= 0)
							(nearest_idle - 2)->next += 1;
						nearest_idle -= 1;
					}
					(*cur) = node;
					cur->next = cur + 1;
					break;
				}
			}
			prev = cur;
		}
	}
}

void eventlist_Generate()
{
	eventlist_tail = eventlist_head;
	if(eventlist_tail != NULL)
	{
		while(eventlist_tail->next != NULL)
			eventlist_tail = eventlist_tail->next;
	}

	TimeType arrtime = 0;
	while(arrtime < closetime)
	{
		CustNode node;
		node.next = NULL;
		node.arrtime = arrtime;
		node.durtime = random_range(durtime_min, durtime_max + 1);
		node.amount = random_range(amount_min, amount_max + 1);
		node.leavetime = -1;
		
		printf("Random event: \tArrtime:%d \tDurtime:%d \tAmount:%d\n", node.arrtime, node.durtime, node.amount);
		eventlist_Insert(node);

		TimeType interval = random_range(interval_min, interval_max + 1);
		arrtime += interval;
	}
}

void eventlist_output(int print_avg_stay)
{
	double sum_stay = 0;
	printf("Event \t| Arrtime \t| Durtime \t| Amount \t| Leavetime\n");
	int eventn = 0;
	for(CustNode* i = eventlist_head; i != NULL; i = i->next)
	{
		printf("%4u \t| %8d\t| %8d\t| %+8d\t| %d\n", (int)(i - eventlist), i->arrtime, i->durtime, i->amount, i->leavetime);
		TimeType waittime = i->leavetime - i->arrtime;
		if(waittime < 0) waittime = 0; // arrtime > closetime
		sum_stay += waittime;
		eventn++;
	}
	printf("\n");
	if(print_avg_stay)
		printf("Average stay time: %lf (min).\n", sum_stay / eventn);
}

#define ECHO_EVENT \
		printf("Process Event #%d (%+d):\nArrive Time:%d\nTime:%d -> %d\nTotal Amount:%d -> %d\nMain Queue: ", (int)(current - eventlist), current->amount, current->arrtime, current_time - current->durtime, current_time, total_amount - current->amount, total_amount);\
		SizeType __qmain_size = CyQueue_Size(qmain), __qwait_size = CyQueue_Size(qwait);\
		if (__qmain_size == 0) printf("<EMPTY>");\
		for(IndexType __i = 0; __i < __qmain_size; __i++)\
			printf("%d ", (int)(CyQueue_At(qmain, __i) - eventlist));\
		printf("\nWait Queue: ");\
		if (__qwait_size == 0) printf("<EMPTY>");\
		for(IndexType __i = 0; __i < __qwait_size; __i++)\
			printf("%d ", (int)(CyQueue_At(qwait, __i) - eventlist));\
		printf("\n\n");
#define ECHO_FAIL \
		printf("Process Event #%d (%+d) but Fail (there's only %d)\nMain Queue: ", (int)(current - eventlist), current->amount, total_amount); \
		SizeType __qmain_size = CyQueue_Size(qmain), __qwait_size = CyQueue_Size(qwait); \
		if (__qmain_size == 0) printf("<EMPTY>");\
		for(IndexType __i = 0; __i < __qmain_size; __i++) \
			printf("%d ", (int)(CyQueue_At(qmain, __i) - eventlist)); \
		printf("\nWait Queue: "); \
		if (__qwait_size == 0) printf("<EMPTY>");\
		for(IndexType __i = 0; __i < __qwait_size; __i++) \
		printf("%d ", (int)(CyQueue_At(qwait, __i) - eventlist)); \
		printf("\n\n");

CyQueue* simulate()
{
	CyQueue *qmain = CyQueue_Init(), 
			*qwait = CyQueue_Init();

	for(CustNode* i = eventlist_head; i != NULL; i = i->next)
		CyQueue_Push(qmain, i);

	TimeType current_time = 0;
	while(!CyQueue_Empty(qmain))
	{
		CustNode *current = CyQueue_Pop(qmain);
		if(current->arrtime > current_time)
			current_time = current->arrtime;
		if(current_time >= closetime)
		{
			CyQueue_Push(qmain, current);
			break;
		}
		if(current->amount < 0 && total_amount + current->amount < 0)
		{
			// 银行存款不足，排入 wait 队列
			CyQueue_Push(qwait, current);
			ECHO_FAIL;
			continue;
		}
		// 正常办理业务
		AmountType prev_amount = total_amount;
		current_time += current->durtime;
		if(current_time >= closetime)
		{
			CyQueue_Push(qmain, current);
			break;
		}
		total_amount += current->amount;
		current->leavetime = current_time;
		
		ECHO_EVENT;

		if(current->amount > 0)
		{
			// 处理 wait 队列
			printf("[Process the wait queue]\n");
			SizeType wait_len = CyQueue_Size(qwait);
			int break_flag = 0;
			for(IndexType i = 0; i < wait_len; i++)
			{
				// 无需判断队空，因为至多处理 wait_len 个元素
				current = CyQueue_Pop(qwait);
				if(total_amount + current->amount >= 0)
				{
					current_time += current->durtime;
					if(current_time >= closetime)
					{
						CyQueue_Push(qwait, current);
						break_flag = 1;
						break;
					}
					total_amount += current->amount;
					current->leavetime = current_time;
					ECHO_EVENT;
					if(total_amount <= prev_amount)
					{
						// 不可能再有满足者
						break;
					}
				}
				else
				{
					CyQueue_Push(qwait, current);
					ECHO_FAIL;
				}
			}
			if(break_flag) break;
			printf("[Process the main queue]\n");
		}
	}
	// 银行营业结束后，所有客户立即离开银行
	CyQueue *qunfinished = CyQueue_Init();
	while(!CyQueue_Empty(qmain))
	{
		CustNode *current = CyQueue_Pop(qmain);
		current->leavetime = closetime;
		CyQueue_Push(qunfinished, current);
	}
	while(!CyQueue_Empty(qwait))
	{
		CustNode *current = CyQueue_Pop(qwait);
		current->leavetime = closetime;
		CyQueue_Push(qunfinished, current);
	}

	CyQueue_Delete(qmain);
	CyQueue_Delete(qwait);
	return qunfinished;
}

void process_unfinished(CyQueue *q)
{
	// 使用选择排序处理 next 表
	// O(n^2)
	eventlist_head = NULL;
	while(!CyQueue_Empty(q))
	{
		CustNode *node_max = NULL;
		SizeType size = CyQueue_Size(q);
		// 每次选择一个编号最大的（当然是到达最晚的）
		// 这里之前因为优先选择到达最晚的，导致相同到达时间的会出现
		// 编号偏后的 next 指向编号偏前的值，从而使得移位出错，造成死循环。
		// 这个 bug 调了非常久。惨痛的代价！
		for(IndexType i = 0; i < size; i++)
		{
			CustNode *current = CyQueue_Pop(q);
			if(node_max == NULL) node_max = current;
			else if((int)(current - eventlist) > (int)(node_max - eventlist))
			{
				CyQueue_Push(q, node_max);
				node_max = current;
			}
			else
			{
				CyQueue_Push(q, current);
			}
		}
		node_max->next = eventlist_head;
		eventlist_head = node_max;

		node_max->leavetime = -1;
	}
	printf("Unfinished events: ");
	for(CustNode* i = eventlist_head; i != NULL; i = i->next)
		printf("%d ", (int)(i - eventlist));
	printf("\n\n");
	CyQueue_Delete(q);
}

void process()
{
	total_amount = total_initial;
	for(SizeType i = 1; i <= day_number; i++)
	{
		printf("--- Day #%u ---\n", i);
		eventlist_Generate();
		eventlist_output(0);
		CyQueue *qunfinished = simulate();
		eventlist_output(1);

		process_unfinished(qunfinished);
	}
}

int main()
{
	srand(time(NULL));
	printf("Please input the total amount:\n");
	scanf("%d\n", &total_initial);
	printf("Please input the close time:\n");
	scanf("%d\n", &closetime);
	printf("Please input the min&max of durtime:\n");
	scanf("%d%d\n", &durtime_min, &durtime_max);
	printf("Please input the min&max of amount:\n");
	scanf("%d%d\n", &amount_min, &amount_max);
	printf("Please input the min&max of interval:\n");
	scanf("%d%d\n", &interval_min, &interval_max);
	printf("Please input the CHUNKSIZE:\n");
	scanf("%u\n", &CHUNKSIZE);
	printf("Please input the number days:\n");
	scanf("%u", &day_number);
	printf("\n=== START SIMULATION ===\n");
	process();
	return 0;
}
