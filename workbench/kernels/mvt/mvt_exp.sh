N=$1
mkdir -p output
./gen_param $1 4 4 > paramCGRA.json 2>./output/$1.txt
bash run.sh
