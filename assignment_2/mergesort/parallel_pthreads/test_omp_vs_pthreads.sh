#!/bin/bash

module load prun
module load python

make clean
make

dir="../../test/merge_parallel_omp_vs_pthreads"
output_file="${dir}/data.csv"
mkdir -p "$dir"
length=1000000
threads=16
thread_min_input_size=1000

rm ${output_file} 2> /dev/null

echo "method,iterations,time" > $output_file

# p_threads
for iteration in {1..50} 
do
    echo -n "pthreads,${iteration}," >> $output_file
    prun -np 1 -v parallel_pthreads -p ${threads} -o ${output_file} -l ${length} -m ${thread_min_input_size} -r
done

cd "../parallel"

# openMP
for iteration in {1..50} 
do
    echo -n "omp,${iteration}," >> $output_file
    prun -np 1 -v parallel -p ${threads} -o ${output_file} -l ${length} -m ${thread_min_input_size} -r
done
