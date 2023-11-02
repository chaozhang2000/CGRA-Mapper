mkdir -p output
filename=$1
while IFS= read -r code rows cols
do
./gen_param $code $rows $cols > paramCGRA.json
#bash gdb1.sh
opt-12 -load ../../../build/src/libmapperPass.so -mapperPass kernel.bc
done < $filename

#./gen_param $1 4 4 > paramCGRA.json 2>./output/$1.txt
