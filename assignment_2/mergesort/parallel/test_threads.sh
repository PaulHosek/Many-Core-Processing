#!/bin/bash

module load prun
module load python

make -B

dir="../../test/merge_parallel_threads"
output_file="${dir}/data.csv"
mkdir -p "$dir"
length=1000000

rm ${output_file} 2> /dev/null

echo "threads,iterations,time" > $output_file

for threads in 1 4 16
do
    for iteration in {1..50} 
    do
        echo -n "${threads},${iteration}," >> $output_file
        prun -np 1 -v parallel -p ${threads} -o ${output_file} -l ${length} -r
    done
done