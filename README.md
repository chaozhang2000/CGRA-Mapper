# NOTE

## 初次运行
1. 根据README.md中，在build目录下的cmkae和make生成动态连接库文件
2. 在CGRA\-Mapper/test目录下运行compile.sh脚本，利用clang-12编译器来将测试程序kernel.cpp编译成kernel.bc,kernel.bc是一种格式的ir。ir是编译器前端处理后生成的文件，编译器后端会继续对其处理。这里利用了基于llvm的c编译器clang的前端对kernel.cpp源文件进行处理，生成ir，之后对ir进行处理的编译器的后端就是本工程CGRA-Mapper了。  
	```
	clang-12 -emit-llvm -fno-unroll-loops -O3 -o kernel.bc -c kernel.cpp
	```
3. 在CGRA\-Mapper/test目录下运行run.sh脚本
	```
	opt-12 -load ../build/src/libmapperPass.so -mapperPass kernel.bc
	```
	猜测是加载了动态链接库之后，利用自己编写的mapperPass来处理ir文件kernel.bc产生输出。于是出现了README中的演示效果

## 源码阅读 

### mapperPass.cpp
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

### DFG

TODO:了解DFG后补充DFG是什么  
猜测DFG的角色，DFG在mapperPass中依次出现的函数分别是下面几个，然后dfg对象被多次作为参数传递给mapper的函数，猜测此类应该是主要作为数据
```cpp
 DFG* dfg = new DFG(t_F, targetLoops, targetEntireFunction, precisionAware,heterogeneity, execLatency, pipelinedOpt);
 dfg->showOpcodeDistribution();
 dfg->generateDot(t_F, isTrimmedDemo);
 dfg->generateJSON();
```
DFG包含的文件有DFG.cpp，DFG.h,DFGEdge.cpp,DFGEdge.h,DFGNode.cpp,DFGNode.h六个文件，先看最顶层，先看数据再看方法。  
看DFG.h中，发现DFG类的私有和公有变量中很多都是和DFGEdge和DFGNode相关的数据结构，所以**DFG类应该是顶层的**。  
再看DFGNode和DFGEdge，可以发现DFGEdge中含有的数据较少，其实就是两个表示src和dst的指向DFGNode的指针。**所以DFGNode应该是最基础的**  
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

2. **DFGEdge** 这个类应该是基础。  


