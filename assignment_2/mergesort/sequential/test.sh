#!/bin/bash

module load prun
module load python

make -B

dir="../../test/merge_sequential"
output_file="${dir}/data.csv"
mkdir -p "$dir"
length=1000000

rm ${output_file} 2> /dev/null

echo "iteration,time" > $output_file

for iteration in {1..100} 
do
        echo -n "${iteration}," >> $output_file
        prun -np 1 -v sequential -o ${output_file} -l ${length} -r
done