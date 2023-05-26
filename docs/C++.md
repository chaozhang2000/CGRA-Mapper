# C++基础
从mapperPass.c开始看源码

### STL

1. **map**

```cpp
map<string, int>* execLatency = new map<string, int>();
```  

map是一种关联容器，用于存储由建值和映射值组成的元素。键值必须是唯一的。  
这里string是按键，int是映射值  
对于map中值的操作  
```cpp
map.insert({key,data});	//插入新数据
a = map.at(key);	//读取数据
map[key] = new_data;	//更改数据
```

但需要注意的是上面定义的是一个map的指针，需要例如(\*map)[key]或者execLatency-\>at()来使用  
**new**是C++中的运算符用于在堆区中分配内存，并返回分配的内存的地址  

2. **list**

list是一个序列容器，它支持在容器中任何位置进行常数时间的插入和删除操作，以及双向迭代。list容器通常实现为双向链表。  
list的用法比较多，碰到再补充
```cpp
list<string>* pipelinedOpt    = new list<string>();	//创建
pipelinedOpt->push_back(pipeOpt[i]);	//从容器的末尾添加一个给定的元素 pop_front在头部去掉一个
```

3. **set**

set是C++标准模板库（STL）中的一种容器，用于存储按照一定顺序排列的唯一元素。set的元素不能被修改只能插入或删除。
```cpp
set<string> paramKeys;
paramKeys.insert("row");	//插入
paramKeys.erase("row");	//删除
//又一例子
for (auto it = s.begin(); it != s.end(); it++) {
	cout << *it << " ";
}       
```

### 其他

1. **ifstream**

创建一个ifstream对象，名为i，并打开param.json的文件。ifstream对象可以用>>和<<来进行输入和输出  
例如这里将读入的文件输出到json对象
```cpp
ifstream i("./param.json");
json param;
i >> param;
```
