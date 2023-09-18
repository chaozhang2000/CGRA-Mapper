# README

**Ignore the README.md in ./workbench just read this README**

**For more information, please visit the author Tan's [Repositories](https://github.com/tancheng/CGRA-Mapper). I have not yet integrated the original README here**

## Quick start

Ubuntu22.04

1. install dependency
```
sudo apt-get install clang-12 make cmake build-essential llvm llvm-12-dev graphviz
```
2. install [doxyfile](https://www.doxygen.nl/download.html) If you'd like to generate the html doc of CGRA-Mapper source code 

3. build the mapper,compile the kernel.cpp and do mapping
```
cd CGRA-Mapper/workbench
make
```
4. run branchmarks
```
cd CGRA-Mapper/workbench/kernels
cd ???
bash compile.sh
bash run.sh
bash dot.sh
```
then you can see that placement.txt and link.txt are generated
link.txt is generated according the DFG.
placement.txt is generated according the config.json.
