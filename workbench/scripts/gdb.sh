SO=$1
BC=$2
OUT_DIR=$3
cp ./param.json $OUT_DIR
cd $OUT_DIR
cat > script.gdb << EOF
set breakpoint pending on
break mapperPass::runOnFunction
run -load $SO -mapperPass $BC
EOF
gdb opt-12 -x script.gdb
