# Debug with GDB

## 编译带有调试信息的mapper

修改CGRA-Mapper/src/CMakeLists.txt如下所示

```
set(CMAKE_BUILD_TYPE Debug)
add_library(mapperPass MODULE
    # List your source files here.
    mapperPass.cpp
    DFGNode.h
    DFGNode.cpp
    DFGEdge.h
    DFGEdge.cpp
    DFG.h
    DFG.cpp
    CGRANode.h
    CGRANode.cpp
    CGRALink.h
    CGRALink.cpp
    CGRA.h
    CGRA.cpp
    Mapper.h
    Mapper.cpp
)

target_compile_options(mapperPass PUBLIC -g) 

set_target_properties(mapperPass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

if(APPLE)
    set_target_properties(mapperPass PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )   
endif(APPLE)
```
再删除CGRA-Mapper下的build文件夹，重新构建

```
rm -rf build
mkdir build
cd build
cmake ..
make
```

## 使用GDB进行调试

``` 
gdb opt-12

break mapperPass::runOnFunction #设置断点
run -load <path_to_your_pass_library> -<pass_name> <other_options> #<>内的内容参考CGRA-Mapper/test/run.sh中的内容进行修改
```

## GDB调试技巧
