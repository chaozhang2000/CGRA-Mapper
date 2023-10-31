clang-12 -emit-llvm -O3 -fno-unroll-loops -o kernel.bc -c susan.c
llvm-dis-12 kernel.bc -o kernel.ll
#llvm-dis susan.bc -o susan.ll
