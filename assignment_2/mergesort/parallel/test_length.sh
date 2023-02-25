#!/bin/bash

threads=16

module load prun
module load python

make -B

dir_asc="../../test/merge_parallel_length_asc"
dir_desc="../../test/merge_parallel_length_desc"
dir_rand="../../test/merge_parallel_length_rand"
output_file_asc="${dir_asc}/data.csv"
output_file_desc="${dir_desc}/data.csv"
output_file_rand="${dir_rand}/data.csv"
mkdir -p "$dir_asc"
mkdir -p "$dir_desc"
mkdir -p "$dir_rand"

rm ${output_file_asc} 2> /dev/null
rm ${output_file_desc} 2> /dev/null
rm ${output_file_rand} 2> /dev/null

echo "length,iteration,time" > $output_file_asc
echo "length,iteration,time" > $output_file_desc
echo "length,iteration,time" > $output_file_rand

for length in 1000 10000 100000 1000000 10000000 100000000 1000000000 10000000000
do
    for iteration in {1..50} 
    do
        echo -n "${length},${iteration}," >> $output_file_asc
        echo -n "${length},${iteration}," >> $output_file_desc
        echo -n "${length},${iteration}," >> $output_file_rand
        prun -np 1 -v parallel -p ${threads} -o ${output_file_asc} -l ${length} -a
        prun -np 1 -v parallel -p ${threads} -o ${output_file_desc} -l ${length} -d
        prun -np 1 -v parallel -p ${threads} -o ${output_file_rand} -l ${length} -r
    done
done