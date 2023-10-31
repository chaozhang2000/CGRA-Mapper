clang-12 -emit-llvm -O3 -fno-unroll-loops -o kernel.bc -c adpcm.c
#llvm-dis fir.bc -o fir.ll
