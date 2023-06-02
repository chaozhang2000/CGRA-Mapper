# NOTE

# 初次运行
1. 根据README.md中，在build目录下的cmkae和make生成动态连接库文件
2. 在CGRA\-Mapper/test目录下运行compile.sh脚本，利用clang-12编译器来将测试程序kernel.cpp编译成kernel.bc,kernel.bc是一种格式的ir。ir是编译器前端处理后生成的文件，编译器后端会继续对其处理。这里利用了基于llvm的c编译器clang的前端对kernel.cpp源文件进行处理，生成ir，之后对ir进行处理的编译器的后端就是本工程CGRA-Mapper了。  
	```
	clang-12 -emit-llvm -fno-unroll-loops -O3 -o kernel.bc -c kernel.cpp
	```
3. 在CGRA\-Mapper/test目录下运行run.sh脚本
	```
	opt-12 -load ../build/src/libmapperPass.so -mapperPass kernel.bc
	```

猜测是加载了动态链接库之后，利用自己编写的mapperPass来处理ir文件kernel.bc产生输出。于是出现了README中的演示效果。  
实际上  
* opt\-12是LLVM的优化器和分析器，它可以对LLVM源文件进行各种优化或分析，并输出优化后的文件或分析结果
* \-load ../build/src/libmapperPass.so是一个选项，用于加载动态对象插件。这个插件应该注册了新的优化或分析通道,加载后，这个插件会添加新的命令行选项来启用各种优化或分析。
* \-mapperPass是一个选项，用于指定要运行的优化或分析通道的名称。这个通道是由上面加载的插件提供的。
* kernel.bc是一个文件名，用于指定输入文件。这个文件应该是LLVM的bitcode格式。

# 源码阅读 

## mapperPass.cpp
首先应该阅读mapperPass相关的源码，查看src/mapperPass.cpp，可以发现其中有很多和llvm相关的内容比如include了很多头文件  
```c
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/LoopIterator.h>
```
mapperPass类也继承自FunctionPass类所以有必要先学习以下llvm，有一个基础的认识  
LLVM的基础学习见[**LLVM学习笔记**](docs/LLVM.md)   
与本项目相关的C++基础知识见[**C++笔记**](docs/C++.md)  
llvm太过复杂，不宜直接一头扎进取，准备先接着看写mapperPass.cpp的代码，先有个更加全局的认识。

可以看到mapperPass类继承自FunctionPass类但是override了runOnFunction函数，这个函数占据了很大篇幅，应该比较关键  
这个函数所做的工作  
1. 从json文件中读取参数
2. 从functionWithLoop中获取目标循环列表,functionWithLoop是一个map\<string,list\<int\>\*\>\*,里面的数据全部来自json参数文件。这一步用一个函数实现。
```cpp
list<Loop*>* getTargetLoops(Function& t_F, map<string, list<int>*>* t_functionWithLoop, bool t_targetNested)
```
函数主要步骤：
* 初始化targetLoopID和targetLoops变量。
* 从t\_functionWithLoop中获取t\_F对应的目标循环ID列表，用一个while循环遍历它。
* 从一个分析通道中获取LoopInfoWrapperPass对象，然后调用它的getLoopInfo()方法，返回一个LoopInfo的引用。
* 用一个for循环遍历LoopInfo中的所有循环，用tempLoopID记录当前循环的ID。
* 如果tempLoopID和targetLoopID相等，说明找到了目标循环，将其存入targetLoops中。如果t\_targetNested为false，说明只要分析最内层的循环，那么就用一个while循环找到最内层的子循环，并将其存入targetLoops中。
* 如果targetLoops为空，说明没有找到目标循环，输出错误信息。

3. 初始化DFG CGRA MAPPER类,包括cgra的setRegConstraint,setCtrlMemConstraint,setBypassConstraint等。
4. DFG CGRA MAPPER类开始工作，从dfg-\>dfg-\>showOpcodeDistribution();开始

对于3,4需要阅读DFG CGRA MAPPER相关的源码，先跳过，回头再细化。

## DFG

DFG是**数据流图**，DFG类的作用是根据IR，来生成数据流图，作为进行接下来处理的一个数据基础。  
什么是数据流图，举个直观的例子，如下图，下图实际上就是本项目中生成的.dot文件，再用dot生成的图像,可见dot.sh。  
<p float="senter">
	<img src="https://github.com/chaozhang2000/CGRA-Mapper/blob/master/docs/fir_dfg.png" width="400" />
</p>
可以看到数据流图中有一些带有操作名称和ID的方框，还有红色箭头，和蓝色箭头，这些都会与DFG相关的类对应起来。  
实际上我们还可以将这个数据流图和原始的C语言代码联系起来。  

```cpp
for (i=0;i<NTAPS;i++) {
	output[j] += input[i] * coefficient[i];
}
```

你会发现在数据流图中的(3),(5)两个节点，代表的是load，实际上是要将input[i]和coefficient[i]load进来，然后红箭头指向了(6)fmul，浮点数乘法操作，这里和程序实际要做的事是相契合的，但不再是串行的程序流程图而是从中探索了并行性，比如两个load可以同时进行。这说明生成数据流图的过程对IR进行了处理，IR可以理解成是llvm的汇编，里面应该不具有并行性  

有了基本概念后接下来来尝试阅读源码当然在没了解DFG是什么之前也可以猜测DFG的角色，DFG在mapperPass中依次出现的函数分别是下面几个，然后dfg对象被多次作为参数传递给mapper的函数，猜测此类应该是主要作为数据
```cpp
 DFG* dfg = new DFG(t_F, targetLoops, targetEntireFunction, precisionAware,heterogeneity, execLatency, pipelinedOpt);
 dfg->showOpcodeDistribution();
 dfg->generateDot(t_F, isTrimmedDemo);//生成上面我们所说的数据流图.dot文件
 dfg->generateJSON();
```
DFG包含的文件有DFG.cpp，DFG.h,DFGEdge.cpp,DFGEdge.h,DFGNode.cpp,DFGNode.h六个文件，先看最顶层，先看数据再看方法。  
看DFG.h中，发现DFG类的私有和公有变量中很多都是和DFGEdge和DFGNode相关的数据结构，所以**DFG类应该是顶层的**。  
再看DFGNode和DFGEdge，可以发现DFGEdge中含有的数据较少，其实就是两个表示src和dst的指向DFGNode的指针。**所以DFGNode应该是最基础的**  
当然我们有了上面的基本介绍后很容易指导DFGEdge是箭头，DFGNode是方框，DFG是全部。  
1. **DFGEdge** 这个类是3个类中最简单的。看其主要部分,就可以看出其功能，师兄的笔记中说这是一个箭头。
```cpp
class DFGEdge
{
	private:
		DFGNode *m_src; //表示连接的起始节点
		DFGNode *m_dst; //表示连接的终止节点
	public:
		void connect(DFGNode*,DFGNode*); //将两个DFGNode* 赋值给m_src和m_dst，表示完成连接
}
```

2. **DFGNode** 这个类应该是基础。简单看其代码可以发现其有以下这些方法。  
```cpp
bool isLoad();
bool isStore();
bool isReturn();
bool isCall();
bool isBranch(); 
//……
```

可以认为每个DFGNode应该有这些属性，应该代表上述的某类操作(实际是某类指令)。

3. **DFG** 这个类完成对数据流图相关的操作。在mapperPass中首先实例化的就是DFG对象。  
先看DFG类中有什么内容举一些例子  
```cpp
private:
    list<DFGEdge*> m_DFGEdges; //DFG箭头
    list<DFGEdge*> m_ctrlEdges; //控制箭头
public:
		list<DFGNode*> nodes;
		void showOpcodeDistribution();
		void generateDot(Function&,bool);
		void generateJSON();
```

可以发现这里DFG类中有代表箭头的列表，有代表节点的列表，当然这些都是画出DFG所必须的。  
其中generateDot函数，就是生成.dot文件，帮助我们画出可视化数据流图的函数，但其并不重要。  
真正重要的是这个函数在DFG实例化后马上就执行了，说明DFG在实例化的时候就已经确定了，所以了解DFG时，其构造函数很重要。  
然后有可以发现构造函数中最主要的函数是  
```cpp
construct(t_f);//Function& t_F  Function是llvm中的类,我门的IR就可以被认为是这个Function
```

这个construct函数非常长，具体看DFG.cpp源代码，这里只介绍其大致工作逻辑。
* 第一个for循环:遍历Function中所有的basicblock，并遍历basicblock中的每条指令为每一条范围内的指令创建DFGnode，并创建控制箭头。
	* 这里需要做一些解释，Function可以理解成我们的IR
	* Function中含有很多基本块，基本块表示程序控制流中的最小单元，是一系列指令的序列，只有一个入口和一个出口。出口被称为是终结指令，是一种控制流指令，如分支跳转返回等，决定程序跳转到哪个基本块。
	* 什么是范围内的指令，我们并不是对所有的指令进行优化，所以我们在param.json文件中也设置了targetLoopsID，来表明需要优化的循环的ID，所以这可以理解，详见DFG.cpp中的shouldIgnore函数。
	* 指令的DFGnode节点是如何创建的，在遍历basicblock基本块的时候，会为第一次遇见的范围内指令创建node并加入nodes列表，然后会判断当前基本块的终结指令是否在范围内，如果在则会遍历其所有的后继节点的入口指令，如果也在范围内则为其也创建节点。
  * 如何创建控制箭头，控制箭头实际就是数据流图中的蓝箭头，根据上一点确定了当前基本块的终结指令和后继基本块的入口指令，就可以将两个节点进行链接，自然产生蓝色箭头。
* 第二个for循环:暂时还没看，但可以发现其主要是生成m\_DFGEdges，即DFG箭头，即数据流图中的红箭头。我们发现在上一个for循环中node实际上是顺序的，红箭头的如何连接实际上面临着并发性探索的问题，这个for循环暂时还不打算深究。  

现在我们知道DFG类可以根据IR生成数据流图了，结下来先进入CGRA和MAPPER类也对其进行初步了解。  

## new title
