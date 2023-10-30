mkdir -p output
filename="u16.txt"
while IFS= read -r line
do
./gen_param $line 4 4 > paramCGRA.json 2>./output/$line.txt
bash run.sh
done < "$filename"

#./gen_param $1 4 4 > paramCGRA.json 2>./output/$1.txt
