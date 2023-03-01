#!/bin/bash

threads=16
length=10000000

module load prun
module load python

make clean
make

dir="../../test/merge_parallel_min_thread_input"
output_file="${dir}/data.csv"
mkdir -p "$dir"

rm ${output_file} 2> /dev/null

echo "min_thread_input,iteration,time" > $output_file

for min_size in 1 10 100 1000 10000 100000 1000000 10000000
do
    for iteration in {1..50} 
    do
        echo -n "${min_size},${iteration}," >> $output_file
        prun -np 1 -v parallel -p ${threads} -o ${output_file} -l ${length} -m ${min_size} -r
    done
done