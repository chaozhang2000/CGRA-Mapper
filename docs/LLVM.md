# LLVM NOTE

## 可能有帮助的网址和博客
1. [LLVM官方文档](https://www.llvm.org/docs/UserGuides.html)  
2. [LLVM中的pass及其管理机制](https://blog.csdn.net/mamamama811/article/details/110165333)  

### Function Pass

会对IR中每个函数都进行一次处理，我们编写的Mapper本质上就是一个Function Pass,会对要进行map的目标函数(kernel)进行处理

### basic block

每个基本块是一段顺序执行的代码，包括其入口和出口，下面的例子可以帮助理解,这是fir.cpp中的kernel经过clong处理输出的llvm汇编代码。

```
; Function Attrs: nofree norecurse nounwind uwtable mustprogress                                     
define dso_local void @_Z6kernelPiS_S_(i32* nocapture readonly %0, i32* nocapture %1, i32* nocapture readonly %2) local_unnamed_addr #0 {
  br label %4

4:                                                ; preds = %3, %23
  %5 = phi i64 [ 0, %3 ], [ %24, %23 ]
  %6 = getelementptr inbounds i32, i32* %1, i64 %5
  br label %7

7:                                                ; preds = %4, %20
  %8 = phi i64 [ 0, %4 ], [ %21, %20 ]
  %9 = sub nsw i64 %5, %8
  %10 = icmp sgt i64 %9, -1
  br i1 %10, label %11, label %20

11:                                               ; preds = %7
  %12 = and i64 %9, 4294967295
  %13 = getelementptr inbounds i32, i32* %0, i64 %12
  %14 = load i32, i32* %13, align 4, !tbaa !2
  %15 = getelementptr inbounds i32, i32* %2, i64 %8
  %16 = load i32, i32* %15, align 4, !tbaa !2
  %17 = mul nsw i32 %16, %14
  %18 = load i32, i32* %6, align 4, !tbaa !2
  %19 = add nsw i32 %18, %17
  store i32 %19, i32* %6, align 4, !tbaa !2
  br label %20

20:                                               ; preds = %7, %11
  %21 = add nuw nsw i64 %8, 1
  %22 = icmp eq i64 %21, 32
  br i1 %22, label %23, label %7, !llvm.loop !6

23:                                               ; preds = %20
  %24 = add nuw nsw i64 %5, 1
  %25 = icmp eq i64 %24, 63
  br i1 %25, label %26, label %4, !llvm.loop !9

26:                                               ; preds = %23
  ret void
}
```
这个函数包括7个基本块，分别是br label %4, 4:, 7:,11:,20:,23:,26:。  
基本块还有successor basic block,是后继基本块，是当前基本块可能会跳转到的其他基本块  
例如基本块7,最后一行指令跳转到基本块11或者20,其successor basic block就是11和20

### Terminator Instruction

基本块（BasicBlock）中的终止指令是指控制流程在基本块内的最后一个指令，用于决定基本块的后继基本块。可以理解为跳转出基本块的那条指令。
