clang-12 -emit-llvm -O3 -fno-unroll-loops -o kernel.bc -c bf_test.c
#llvm-dis bf.bc -o bf.ll
