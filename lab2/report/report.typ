#set text(font: ("Noto Serif CJK SC"))
#show raw: set text(font: ("Hack", "Noto Sans S Chinese"))
#set page(
  numbering: "1",
)
#set par(
  leading: 1.2em, 
  first-line-indent: 2em
)
#show heading: it => {it;text()[#v(0.3em, weak: true)];text()[#h(0em)]}

#align(center, 
text(17pt)[
  *实验报告：银行业务管理系统*
]
)
#align(center, 
text[
题目：模拟银行业务管理的离散事件\
]
)

#linebreak()
= 一、需求分析

+ 通过给定的范围，随机生成银行每天的事件表，即客户数据，包括到达时间、停留时间、金额。
+ 模拟银行的业务流程，采用队列模拟客户排队。队列分为普通队列和等待队列，以模拟银行资金总额充足与不足时的等待情况。我们需要算出每个客户的离开时间。
+ 当银行资金总额不足以处理普通队列的客户时，将客户加入等待队列中；当银行资金总额充足时，优先检查并处理等待队列中的客户。
+ 选做一要求使用循环队列作为银行中两个排队队列的实现形式，要求循环利用内存空间，且在内存空间不足时可以实现扩容。
+ 选做二要求维护多天的运行模拟，要求后一天的事件尽可能填充前一天未完成事件表中的“碎片”，以学习类似操作系统中的内存管理方式。

= 二、概要设计

拟采用循环队列的数据结构，编写一个模拟函数对银行事件进行模拟，并编写一系列函数来维护事件表插入的正确性。程序中将涉及下列两个抽象数据类型：

1. 定义“客户”类型 `CustNode`:

  ```cpp
  typedef struct CustNode
  {
      // 数据对象：
      TimeType arrtime, durtime, leavetime; // 客户的到达时间、业务处理时间、离开时间
      AmountType amount;                    // 客户的业务金额 
      struct CustNode* next;                // （选做二）指向下一个未完成客户的指针
  };
  ```

2. 定义循环队列 `CyQueue`：

  ```cpp
  typedef struct CyQueue
  {
      // 数据对象：
      SizeType maxsize;       // 队列当前的最大容量
      IndexType head, tail;   // 队首和队尾的下标
      int full_flag;          // 队列是否已满的标志
      DataType* data;         // 队列的数据数组

      // 基本操作：
      CyQueue* CyQueue_Init();
        // 操作结果：构造一个空队列
      void CyQueue_Delete(CyQueue* q);
        // 初始条件：队列 q 存在
        // 操作结果：销毁队列 q
      void CyQueue_Clear(CyQueue* q);
        // 操作结果：将队列 q 清空
      SizeType CyQueue_Size(CyQueue* q);
        // 操作结果：返回队列 q 的元素个数
      int CyQueue_Empty(CyQueue* q);
        // 操作结果：若队列 q 为空则返回 1，否则返回 0
      DataType CyQueue_At(CyQueue* q, IndexType ind);
        // 操作结果：返回队列 q 中（从队首数起）下标为 ind 的元素
      void CyQueue_Resize(CyQueue* q, SizeType newsize);
        // 操作结果：将队列 q 的容量调整为 newsize
      void CyQueue_Push(CyQueue* q, DataType data);
        // 操作结果：将元素 data 入队，若容量不足则自动扩容
      DataType CyQueue_Pop(CyQueue* q);
        // 操作结果：将队首元素出队并返回
      DataType CyQueue_Front(CyQueue* q);
        // 操作结果：返回队首元素
  };
  ```
3. 维护事件列表 `eventlist`
  ```cpp
  // 数据对象：
  CustNode eventlist[MAX];          // 事件列表
  CustNode* eventlist_head = NULL;  // 第一个未处理事件的指针
  CustNode* eventlist_tail = NULL;  // 最后一个未处理事件的指针
  // 基本操作：
  void eventlist_Insert(CustNode node);
    // 初始条件：eventlist_tail 已经被正确初始化
    // 操作结果：将客户 node 插入事件列表 eventlist
  void eventlist_Generate();
    // 操作结果：随机生成银行每天的事件表，并直接存入 eventlist
  void eventlist_output(int print_avg_stay);
    // 操作结果：输出事件列表，若 print_avg_stay 为真则输出平均逗留时间
  ```

4. 本程序包含三个模块：
  + 主程序模块。`main()` 函数负责处理命令行输入和调用 `process()` 函数；`process()` 函数负责循环模拟每天的银行业务，更新当天的事件表，调用 `simulate()` 函数以模拟当天的事件，输出当天的事件表，并整理当天未完成的事件以供下一天使用。`simulate()` 函数负责模拟单日的事件，并返回未完成的事件队列，待处理函数使用。`process_unfinished(CyQueue *q)` 函数接收未完成的事件队列，并将这些事件在事件表中妥善整理，供下一天使用。
  + 事件列表模块——生成、维护每天的事件列表。
  + 循环队列模块——实现银行的两个排队队列。
  除了三个主要模块外，还有部分辅助函数用于生成随机数等功能。

= 三、详细设计

1. 主程序中需要的全程量
  ```cpp
  #define MAX 200010                      // 事件表的最大容量

  // 定义时间、金额、下标、大小等数据类型
  typedef int TimeType;
  typedef int AmountType;
  typedef unsigned int IndexType;
  typedef unsigned int SizeType;

  // 待输命令行入的全局变量
  SizeType CHUNKSIZE = 4;                 // 事件表每次扩容的大小
  AmountType total_initial;               // 银行的初始资金总额
  TimeType closetime;                     // 银行的营业结束时间
  TimeType  durtime_min,  durtime_max,    // 客户的最短和最长业务处理时间
            interval_min, interval_max;   // 客户的最短和最长到达时间间隔
  AmountType amount_min,   amount_max;    // 客户的最小和最大业务金额
  SizeType day_number;                    // 模拟的天数

  // 模拟过程中维护的全局变量
  CustNode eventlist[MAX];                // 事件列表
  AmountType total_amount = 0;            // 银行当前的资金总额
  ```

= 四、调试分析

= 五、用户手册

= 六、测试结果

= 七、附录