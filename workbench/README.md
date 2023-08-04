## File Tree
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
│       ├── script.gdb
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
    ├── gdb.sh
    └── run.sh
```

## Quick Start
1. Put kernels in ./kernels  
2. Put param.json in ./ 
3. Run make NAME=kernels\_name map to build the mapper,compile the kernels\_name.cpp and do mapping
4. You can also run make NAME=kernels\_name gdb to build the mapper with debug infomation,compile the kernels\name.cpp and debug with gdb, the gdb script can be modify in ./script/gdb.sh
5. You can see all output in ./build/kernels\_name

## Make
```
make NAME=kernels_name map	#build mapper,compile kernel_name.cpp and do map
make NAME=kernels_name gdb	#build mapper with debug information,compile kernel_name.cpp and run through gdb
make mapper	#build mapper
make mappergdb #build mapper with debug information
make NAME=kernels_name kernel # comile kernel_name.cpp
make clean NAME=kernels_name # rm .build/kernel_name
make cleanmapper #rm the mapper's build dir (../build)
make cleanall # rm both mapper's and kernel's build dir 
```
