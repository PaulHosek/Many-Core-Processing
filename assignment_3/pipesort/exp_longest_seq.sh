#!/bin/bash
rm res_longest_seq.csv
module load prun
make clean
make


repetitions=5
max_length=5000
if [ ! -f res_longest_seq.csv ]; then
  touch res_longest_seq.csv
  echo "n_nums,buffersize,runtime" > res_longest_seq.csv
fi

for ((i=0;i<repetitions;i++)) do
  for buffersize in 100; do
    for ((n_nums=50;n_nums<max_length;n_nums=n_nums+50)); do
    time_elapsed=$(prun -np 1 -v pipesort -l $n_nums -s $((RANDOM)) | grep -oP 'Pipesort took:\s*\K[\d.]+')
    echo "$n_nums,$buffersize,$time_elapsed" >> res_longest_seq.csv
    done
  done
done
