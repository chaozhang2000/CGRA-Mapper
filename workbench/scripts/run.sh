SO=$1
BC=$2
OUT_DIR=$3
cp ./param.json $OUT_DIR
cd $OUT_DIR
opt-12 -load $SO -mapperPass $BC
#opt-12 -load ../build/src/libmapperPass.so -mapperPass kernel.bc
