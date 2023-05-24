# NOTE

## 初次运行
1. 根据README.md中，在build目录下的cmkae和make生成动态连接库文件
2. 在CGRA\-Mapper/test目录下运行compile.sh脚本，利用clang-12编译器来将测试程序kernel.cpp编译成kernel.bc
	```
	clang-12 -emit-llvm -fno-unroll-loops -O3 -o kernel.bc -c kernel.cpp
	```
3. 在CGRA\-Mapper/test目录下运行run.sh脚本
	```
	opt-12 -load ../build/src/libmapperPass.so -mapperPass kernel.bc
	```
	猜测是加载了动态链接库之后，利用自己编写的mapperPass来处理kernel.bc产生输出。于是出现了README中的演示效果

## 源码阅读 

首先应该阅读mapperPass相关的源码，查看src/mapperPass.cpp，可以发现其中有很多和llvm相关的内容比如include了很多头文件  
```c
#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Analysis/LoopIterator.h>
```
mapperPass类也继承自FunctionPass类所以有必要先学习以下llvm，有一个基础的认识  
LLVM的基础学习见[**LLVM学习笔记**](docs/LLVM.md)
