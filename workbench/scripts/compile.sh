SRC=$1
BC=$2
LL=$3
clang-12 -emit-llvm -fno-unroll-loops -O3 -o $BC -c $SRC
llvm-dis-12 $BC -o $LL
