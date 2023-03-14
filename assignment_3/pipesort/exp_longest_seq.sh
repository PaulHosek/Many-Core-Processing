#!/bin/bash
rm res_longest_seq_find.csv
module load prun
make clean
make


repetitions=1
max_length=300
if [ ! -f res_longest_seq_find.csv ]; then
  touch res_longest_seq_find.csv
  echo "n_nums,buffersize,runtime" > res_longest_seq_find.csv
fi

for ((i=0;i<repetitions;i++)) do
  for buffersize in 100; do
    for ((n_nums=1;n_nums<max_length;n_nums=n_nums+1)); do
    time_elapsed=$(prun -np 1 -v pipesort -l $n_nums -s $((RANDOM)) | grep -oP 'Pipesort took:\s*\K[\d.]+')
    echo "$n_nums,$buffersize,$time_elapsed" >> res_longest_seq_find.csv
    done
  done
done
