cd ../..
make cleanall
make mappergdb
cd kernels/susan
#./gen_param 0xffff 4 4 > paramCGRA.json
gdb opt-12 -x script.gdb

