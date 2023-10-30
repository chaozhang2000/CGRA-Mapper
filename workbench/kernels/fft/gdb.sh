cd ../..
make cleanall
make mappergdb
cd kernels/fft
#./gen_param 0xffff 4 4 > paramCGRA.json
gdb opt-12 -x script.gdb

