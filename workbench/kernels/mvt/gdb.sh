cd ../..
make cleanall
make mappergdb
cd kernels/mvt
gdb opt-12 -x script.gdb

