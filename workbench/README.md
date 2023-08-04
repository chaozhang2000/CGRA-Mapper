```
.
├── build
│   └── kernel
│       ├── config.json
│       ├── dfg.json
│       ├── kernel.bc
│       ├── kernel.cpp
│       ├── kernel.ll
│       ├── param.json
│       └── _Z6kernelPiS_S_.dot
├── kernels
│   ├── fir.cpp
│   └── kernel.cpp
├── Makefile
├── param.json
├── README.md
└── scripts
    ├── compile.sh
    ├── dot.sh
    └── run.sh
```

Put kernels in ./kernels  
Put param.json in ./  
The kernel you want to process is specified by the parameter NAME,the default value is kernel,the NAME.cpp will be processed by the mapper.  
The result of compiling and mapping is output to ./build/NAME   

```
make NAME=kernels_name map	#build mapper,compile kernel_name.cpp and do map
make mapper	#build mapper
make NAME=kernels_name kernel # comile kernel_name.cpp
make clean NAME=kernels_name # rm .build/kernel_name
make cleanmapper #rm the mapper's build dir
make cleanall # rm both mapper's and kernel's build dir 
```
