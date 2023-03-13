#!/bin/bash
rm res_longest_seq_v2.csv
module load prun
make clean
make


repetitions=1
max_length=1000000
if [ ! -f res_longest_seq_v2.csv ]; then
  touch res_longest_seq_v2.csv
  echo "n_nums,buffersize,runtime" > res_longest_seq_v2.csv
fi

for ((i=0;i<repetitions;i++)) do
  for buffersize in 100; do
    for ((n_nums=50;n_nums<max_length;n_nums=n_nums+100)); do
    time_elapsed=$(prun -np 1 -v pipesort -l $n_nums -s $((RANDOM)) | grep -oP 'Pipesort took:\s*\K[\d.]+')
    echo "$n_nums,$buffersize,$time_elapsed" >> res_longest_seq_v2.csv
    done
  done
done
