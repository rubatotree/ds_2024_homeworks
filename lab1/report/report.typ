#set text(font: ("Noto Serif CJK SC"))
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
  *实验报告1：多项式计算器*
]
)
#align(center, 
text[
题目：实现一元稀疏多项式计算器\
朱雨田~~~~~PB23050888
]
)


#linebreak()

= 一、需求分析

本实验要求实现一元稀疏多项式简单计算器的基本功能：

1. 输入并建立多项式；
2. 输出多项式，输出形式为整数序列：$n,c_1,e_1,c_2,e_2,...,c_n,e_n$，其中 $n$ 为多项式项数，$c_i$ 和 $e_i$ 分别是第 $i$ 项的系数和指数，序数按指数降序排列；

= 二、概要设计

= 三、详细设计

= 四、调试分析

= 五、用户手册

== 1. 命令行

== 2. 图形界面

= 六、测试结果

= 七、附录