# FIR Example

首先我们的思路是通过一个例子来明白一个最简单的FIR算法是如何在CGRA硬件上运行的，再完成这个过程之后才能算是对CGRA有了最基础的认识，能帮助我们评估在CGRA上部署算法的难度。这个过程进行的顺序是
1. 明确指令的格式
2. 明确CGRA硬件对于指令的具体行为
3. 获取FIR例子的指令流
4. 推导FIR例子执行的过程
面对对CGRA陌生同学，先介绍CGRA的硬件

## 硬件介绍

**CGRA** 跳过CGRA的基本概念。首先是OpenCGRA所设计的CGRA结构，包括一些Tile和Date Memory。Tile可以理解为功能单元可以进行计算、将数据传输给周围Tile、以及访存等。Data Memory用来提供数据的存储器。我们要举例的CGRA结构如下图所示。此图为YF师兄所作。  
<p float="center">
	<img src="https://github.com/chaozhang2000/CGRA-Mapper/blob/test/docs/cgra.png" width="400" />
</p>
其中Data Memory位于上图左侧，未画出，与Tile0、4、8、12直接相连。图中的Tile的连接方式是每个Tile和(东南西北)上下左右四个Tile都有直接相连

**Tile** 每个Tile中主要包括Config Memory、Crossbar、channals、FU。在我们将要举的例子中，Tile大多是同质的，即每个Tile相同，都具有多种计算功能，例如加法、乘法等，但会根据配置每次只执行其中的一种计算。有些Tile具有访存(执行load和store指令)的功能，在将举的例子中，是最靠近Data Memory的四个Tile(0、4、8、12)具有这样的功能。  
Tile的结构如下图所示，图依旧来自YF师兄。  
<p float="center">
	<img src="https://github.com/chaozhang2000/CGRA-Mapper/blob/test/docs/cgra_tile.png" width="400" />
</p>
* **Config Memory**用于存放Tile的指令,Tile会每个周期执行一条指令，对于Tile的指令我们下面会详细介绍。
* **Crossbar** 主要作用是为八个Channel分配数据数据，所举的例子中，Crossbar有6个输入和8个输出，8个输出分别连接到8个Channel的输入，如果有某个Channel需要被分配数据的话，Crossbar会将某个由指令指定的输入数据分配到对应的Channel上。Crossbar的6个输入，前4按顺序分别来自北南西东四个相邻的Tile，后两个是本Tile中FU计算得到的两个结果。例如指令可以指定将第5个输入分配给第channel4，那么Crossbar会将FU的第一个结果输出，分配到channel4。
* **channel** 是数据传输到周围其他Tile以及传输到FU的通道，输入来自Crossbar，输出到本Tile的FU或者周围相连Tile的Crossbar输入。所举例子，Channel0到3输出到周围的四个Tile，对应的顺序是0、1、2、3分别对应北南西东。Channel4到7输出到FU。channel中有一个队列来暂存数据，在例子中是一个深度为1的队列，就相当于一个寄存器。指令控制crossbar将数据分配到channel时数据会被存到队列中，然后数据会在队列中保存，并输出给周围tile的crossbar，当周围Tile的crossbar根据指令，将数据分配到某个channel时，完成了数据从tile之间的传递，前一个tile的channel中数据将会出列。**所以channel在面对空指令时会暂存数据，这对理解空指令很重要**,我们在这篇笔记中记录了源码阅读过程。channel还有bypass功能。传输的数据还会带有一个bypass信号，当其为1时，数据将会绕过这个队列直接传输。bypass信号在Crossbar中产生，了解细节请阅读Crossbar源码。
* **FU** 则是真正执行计算的单元，包括加减乘除访存等等，根据指令来对channel4到7的数据进行计算，生成输出。
## 指令分析

本节的目的是了解指令的含义。直接看verilog代码中指令的格式
```verilog
typedef struct packed {
  logic [5:0] ctrl ;
  logic [0:0] predicate ;
  logic [3:0][2:0] fu_in ;
  logic [7:0][2:0] outport ;
  logic [5:0][0:0] predicate_in ;
} CGRAConfig_6_4_10_12;
```
可以看到一条指令包括5个部分:1个6位的ctrl，一个1位的predicate，4个3位的fu\_in，8个3位的outport，6个1位的predicate\_in。  
我们首先来根据刚才了解的CGRA硬件结构简单思考一下在CGRA中Tile的工作过程中，指令应该起哪些控制作用。  
1. 控制FU选择什么计算
2. 控制FU选择channel4到7中的哪几个数据进行计算
3. 控制Crossbar的8个输入分别来自Crossbar的6个输入中的哪一个  
实际上述3点分别对应指令中ctrl、fu\_in、outport三个部分。现在对这三部分再做一些说明。  
我们首先举一个具体的例子
```
CGRAConfig_6_4_6_8(Bits6(0x19), ctrl
Bits1(0x0), //predicate
[Bits3(0x1), Bits3(0x2), Bits3(0x0), Bits3(0x0)], //fu_in
[Bits3(0x0), Bits3(0x5), Bits3(0x5), Bits3(0x0), Bits3(0x0), Bits3(0x0), Bits3(0x0), Bits3(0x0)], //outport
[Bits1(0x0), Bits1(0x0), Bits1(0x0), Bits1(0x0), Bits1(0x0), Bits1(0x0)] //predicate_in
)
```
* **ctrl** 用来编码FU进行哪种计算，例如0x07表示乘法，0x0c表示load，上述例子中的0x19表示ADD\_CONST。~~TODO:这里应该有一张完整的表以后再补充~~
* **fu_in** 用来编码FU计算的操作数分别来自输入FU的四个Channel中的哪个。本段由4个3位表示，每个3位表示一个操作数，三位编码的大小表示操作数来源的Channel编号(输入FU的四个Channel中的第几个),0x1表示第一个Channel即Channel4，以此类推，0x0则表示本次FU计算没有此操作数。上述例子中的fu\_in部分表示本次FU有两个操作数，第一个操作数来自第一个channel即channel4，第二个操作数来自第二个channel即channel5
* **outport** 用来编码Crossbar的输出(即8个channel的输入)来自6个输入中的哪一个,outport部分由8个3位表示，每个3位控制一个输出，3位编码的大小表示本输出来自于6个输入中的哪个，3位编码0x1表示来自6个输入中的第一个,0x0表示本输出不传递数据。6个输入的前4个依次来自上下左右4个Tile，后两个依次来自FU的第一个和第二个输出。上述例子中的outport代码段表示第二个和第三个输出传递数据，都来自六个输入中的第5个即FU计算的第一个输出。第二个和第三个输出会输出给Channel1和Channel2，即会发送到南边和西边的两个相邻的Tile。
* **predicate** ~~TODO:还没有弄明白，等明白后再补充。~~

## FIR指令

我们下面给出一些翻译后更加可读的指令，由于时间关系，暂时不说明

```
the 0 inst
cycle=0	(x,y)=(1,1)	opt="OPT_NAH"
out_4="3"(E->fui1)	
predicate=0	predicate_in=null

the 1 inst
cycle=0	(x,y)=(2,1)	opt="OPT_ADD_CONST"
out_0="4"(fuo1->N)	
out_2="4"(fuo1->W)	
predicate=0	predicate_in=null

the 2 inst
cycle=0	(x,y)=(0,2)	opt="OPT_NAH"
out_4="3"(E->fui1)	
predicate=0	predicate_in=null

the 3 inst
cycle=0	(x,y)=(1,2)	opt="OPT_ADD_CONST"
out_2="4"(fuo1->W)	
predicate=0	predicate_in=null

the 4 inst
cycle=0	(x,y)=(2,2)	opt="OPT_NAH"
out_4="2"(W->fui1)	
out_5="1"(S->fui2)	
predicate=0	predicate_in=null

the 5 inst
cycle=0	(x,y)=(1,3)	opt="OPT_NAH"
out_2="3"(E->W)	
predicate=0	predicate_in=null

the 6 inst
cycle=0	(x,y)=(2,3)	opt="OPT_ADD_CONST"
out_2="4"(fuo1->W)	
predicate=0	predicate_in=null

the 7 inst
cycle=1	(x,y)=(1,1)	opt="OPT_EQ_CONST"
out_3="4"(fuo1->E)	
predicate=0	predicate_in=null

the 8 inst
cycle=1	(x,y)=(2,1)	opt="OPT_NAH"
out_4="2"(W->fui1)	
predicate=0	predicate_in=null

the 9 inst
cycle=1	(x,y)=(0,2)	opt="OPT_LD"
out_3="4"(fuo1->E)	
predicate=0	predicate_in=null

the 10 inst
cycle=1	(x,y)=(1,2)	opt="OPT_NAH"
out_0="2"(W->N)	
out_2="3"(E->W)	
out_4="3"(E->fui1)	
predicate=0	predicate_in=null

the 11 inst
cycle=1	(x,y)=(2,2)	opt="OPT_ADD"
out_2="4"(fuo1->W)	
predicate=0	predicate_in=null

the 12 inst
cycle=1	(x,y)=(0,3)	opt="OPT_NAH"
out_4="3"(E->fui1)	
predicate=0	predicate_in=null

the 13 inst
cycle=1	(x,y)=(1,3)	opt="OPT_NAH"
predicate=0	predicate_in=null

the 14 inst
cycle=2	(x,y)=(1,1)	opt="OPT_NAH"
out_0="3"(E->N)	
predicate=0	predicate_in=[3]

the 15 inst
cycle=2	(x,y)=(2,1)	opt="OPT_BRH"
out_0="4"(fuo1->N)	
out_2="4"(fuo1->W)	
predicate=0	predicate_in=null

the 16 inst
cycle=2	(x,y)=(0,2)	opt="OPT_NAH"
out_4="3"(E->fui1)	
predicate=0	predicate_in=null

the 17 inst
cycle=2	(x,y)=(1,2)	opt="OPT_PHI_CONST"
out_3="4"(fuo1->E)	
predicate=1	predicate_in=null

the 18 inst
cycle=2	(x,y)=(2,2)	opt="OPT_NAH"
out_4="1"(S->fui1)	
out_6="2"(W->fui3)	
predicate=0	predicate_in=[1]

the 19 inst
cycle=2	(x,y)=(0,3)	opt="OPT_LD"
out_3="4"(fuo1->E)	
predicate=0	predicate_in=null

the 20 inst
cycle=2	(x,y)=(1,3)	opt="OPT_NAH"
out_4="1"(S->fui1)	
out_5="2"(W->fui2)	
predicate=0	predicate_in=null

the 21 inst
cycle=3	(x,y)=(1,1)	opt="OPT_NAH"
predicate=0	predicate_in=null

the 22 inst
cycle=3	(x,y)=(2,1)	opt="OPT_NAH"
out_4="0"(N->fui1)	
predicate=0	predicate_in=null

the 23 inst
cycle=3	(x,y)=(0,2)	opt="OPT_STR"
predicate=0	predicate_in=null

the 24 inst
cycle=3	(x,y)=(1,2)	opt="OPT_NAH"
out_3="0"(N->E)	
out_4="3"(E->fui1)	
out_5="1"(S->fui2)	
predicate=0	predicate_in=[1]

the 25 inst
cycle=3	(x,y)=(2,2)	opt="OPT_PHI_CONST"
out_0="4"(fuo1->N)	
out_1="4"(fuo1->S)	
out_2="4"(fuo1->W)	
predicate=1	predicate_in=null

the 26 inst
cycle=3	(x,y)=(1,3)	opt="OPT_MUL"
out_1="4"(fuo1->S)	
predicate=0	predicate_in=null

the 27 inst
cycle=3	(x,y)=(2,3)	opt="OPT_NAH"
out_4="1"(S->fui1)	
predicate=0	predicate_in=null

the 28 inst
cycle=4	(x,y)=(1,1)	opt="OPT_NAH"
out_4="3"(E->fui1)	
predicate=0	predicate_in=null

the 29 inst
cycle=4	(x,y)=(2,1)	opt="OPT_ADD_CONST"
out_0="4"(fuo1->N)	
out_2="4"(fuo1->W)	
predicate=0	predicate_in=null

the 30 inst
cycle=4	(x,y)=(0,2)	opt="OPT_NAH"
out_4="3"(E->fui1)	
predicate=0	predicate_in=null

the 31 inst
cycle=4	(x,y)=(1,2)	opt="OPT_ADD_CONST"
out_2="4"(fuo1->W)	
predicate=0	predicate_in=null

the 32 inst
cycle=4	(x,y)=(2,2)	opt="OPT_NAH"
out_4="2"(W->fui1)	
out_5="1"(S->fui2)	
predicate=0	predicate_in=null

the 33 inst
cycle=4	(x,y)=(1,3)	opt="OPT_NAH"
out_2="3"(E->W)	
predicate=0	predicate_in=null

the 34 inst
cycle=4	(x,y)=(2,3)	opt="OPT_ADD_CONST"
out_2="4"(fuo1->W)	
predicate=0	predicate_in=null

```
接下来就按照各个时钟周期和指令的行为来模拟CGRA计算FIR的行为

## CGRA计算FIR的过程

### cycle0

当某个tile的fu将计算结果输出给周围的tile的crossbar信号时，会进行bypass，跳过本tile的channel中的寄存器。  
据个例子是，某个tile的指令是进行add操作结果要输出给北部的tile，在本上升沿接受到指令开始计算，由于bypass,所以计算结果在下个上升沿到来之前就已经施加到北部tile crossbar的第2个输入，若本上升沿北部tile接受的指令要使用南部的输入，则在下个时钟周期会将这个计算结果存入一个channel的寄存器中。
那么现在又有一个问题，如果上述例子中北部的tile没有及时的接受计算结构，那么数据会保存吗？阅读代码，应该会生成锁存器，应该在下次计算之前会保存
下面计算的过程由于时间问题先不解释

```
________    ____12___    ____13___    ____14___    ____15___  ^ Y  
|      |    |        | n |        |   |  add c |   |        | |  
|      |    |        |<--|        |<--|fuo1    |   |        | |  3
|      |    |________|   |________|   |________|   |________| |  
|      |                                                      |
|      |    ____8____    ____9____    ____10___    ____11___  |  
|      |    |        |   |  add c | l |        |   |        | |  
|      |    |    fui1|<--|fuo1    |-->|fui1    |   |        | |  2
| Date |    |________|   |________|   |___fui2_|   |________| |
|      |                                   ^                  |
| MEM  |    ____4____    ____5____    ____6|___    ____7____  |  
|      |    |        |   |        |   |  add c |   |        | |  
|      |    |        |   |    fui1|<--|  fuo1  |   |        | |  1
|      |    |________|   |________|   |__out___|   |________| |  
|      |                                                      |
|      |    ____0____    ____1____    ____2____    ____3____  |  
|      |    |        |   |        |   |        |   |        | |  
|      |    |        |   |        |   |        |   |        | |  0
|______|    |________|   |________|   |________|   |________| |  
----------------------------------------------------------------> X
								0						 1						2							3
```

```
________    ____12___    ____13___    ____14___    ____15___  ^ Y  
|      |    |        | l |        |   |        |   |        | |  
|      |    |    fui1|<--|        |   |        |   |        | |  3
|      |    |________|   |________|   |________|   |________| |  
|      |                      ^                               |
|      |    ____8____  1 ____9|n1_    ____10___    ____11___  |  
|      |    |LD fout1|-->|        |   |  add   |   |        | |  
|      |--->|        | n2|        | 2 |        |   |        | |  2
| Date |    |________|<--|____fui1|<--|fuo1____|   |________| |
|      |                                                      |
| MEM  |    ____4____    ____5____    ____6____    ____7____  |  
|      |    |        |   |  EQ c  |   |        |   |        | |  
|      |    |        |   |    fuo1|-->|fui1    |   |        | |  1
|      |    |________|   |________|   |________|   |________| |  
|      |                                                      |
|      |    ____0____    ____1____    ____2____    ____3____  |  
|      |    |        |   |        |   |        |   |        | |  
|      |    |        |   |        |   |        |   |        | |  0
|______|    |________|   |________|   |________|   |________| |  
----------------------------------------------------------------> X
								0						 1						2							3
```
```
________    ____12___    ____13___    ____14___    ____15___  ^ Y  
|      |    |  LD    |   |        |   |        |   |        | |  
|      |--->|    fuo1|-->|fui2    |   |        |   |        | |  3
|      |    |________|   |____fui1|   |________|   |________| |  
|      |                       ^                              |
|      |    ____8____    ____9_|l__    ____10___    ____11___  |  
|      |    |        | l |  PHI c |   |        |   |        | |  
|      |    |    fui1|<--|    fuo1|-->|fui3    |   |        | |  2
| Date |    |________|   |________|   |____fui1|   |________| |
|      |                      ^             ^                 |
| MEM  |    ____4____    ____5|n__    ____6_|__    ____7____  |  
|      |    |        |   |        |   |BRH fuo1|   |        | |  
|      |    |        |   |        |<--|fuo1    |   |        | |  1
|      |    |________|   |________|   |________|   |________| |  
|      |                                                      |
|      |    ____0____    ____1____    ____2____    ____3____  |  
|      |    |        |   |        |   |        |   |        | |  
|      |    |        |   |        |   |        |   |        | |  0
|______|    |________|   |________|   |________|   |________| |  
----------------------------------------------------------------> X
								0						 1						2							3
```
```
________    ____12___    ____13___    ____14___    ____15___  ^ Y  
|      |    |        |   |  MUL   |   |        |   |        | |  
|      |    |        |   |        |   |        |   |        | |  3
|      |    |________|   |__fuo1__|   |____fui1|   |________| |  
|      |                      |1            ^                 |
|      |    ____8____    ____9^___  n1____10|__    ____11___  |  
|      |    | ST     |   |        |-->|  PHI   |   |        | |  
|      |<---|        |   |        |   |  fuo1  |   |        | |  2
| Date |    |________|   |fui2fui1|<--|_to_nsw_|   |________| |
|      |                   ^               |                  |
| MEM  |    ____4____    _l|_5____    ____6^___    ____7____  |  
|      |    |        |   |        |   |   fui1 |   |        | |  
|      |    |        |   |        |   |        |   |        | |  1
|      |    |________|   |________|   |________|   |________| |  
|      |                                                      |
|      |    ____0____    ____1____    ____2____    ____3____  |  
|      |    |        |   |        |   |        |   |        | |  
|      |    |        |   |        |   |        |   |        | |  0
|______|    |________|   |________|   |________|   |________| |  
----------------------------------------------------------------> X
								0						 1						2							3
```
```
________    ____12___    ____13___    ____14___    ____15___  ^ Y  
|      |    |        | n |        |   | add c  |   |        | |  
|      |    |        |<--|        |<--|fuo1    |   |        | |  3
|      |    |________|   |________|   |________|   |________| |  
|      |                                                      |
|      |    ____8____    ____9____    ____10___    ____11___  |  
|      |    |        |   | add c  | l |        |   |        | |  
|      |    |    fui1|<--|fuo1    |-->|fui1    |   |        | |  2
| Date |    |________|   |________|   |____fui2|   |________| |
|      |                                    ^                 |
| MEM  |    ____4____    ____5____    ____6_|__    ____7____  |  
|      |    |        |   |        |   |addcfuo1|   |        | |  
|      |    |        |   |    fui1|<--|fuo1    |   |        | |  1
|      |    |________|   |________|   |________|   |________| |  
|      |                                                      |
|      |    ____0____    ____1____    ____2____    ____3____  |  
|      |    |        |   |        |   |        |   |        | |  
|      |    |        |   |        |   |        |   |        | |  0
|______|    |________|   |________|   |________|   |________| |  
----------------------------------------------------------------> X
								0						 1						2							3
```
