#!/bin/bash

module load prun
module load python

now=$(date +%s)
dir="../../test/${now}"
summary_file="${dir}/summary.csv"
mkdir -p "$dir"

echo "M,iterations,time_not_precomputed,time_precomputed" > $summary_file

for m in 10 100 1000
do
        test_subdir="${dir}/test_m${m}_n${m}"
        mkdir $test_subdir

        prun -np 1 -v python ../../test/pgm_generator.py -m ${m} -n ${m} -v 65535 -o "${test_subdir}/temperature.pgm"
        prun -np 1 -v python ../../test/pgm_generator.py -m ${m} -n ${m} -v 65535 -o "${test_subdir}/conductivity.pgm"
        
        let iterations=500000/${m}
        for i in 1 2 3 4 5
        do
        # without precomputed sums
        prun -np 1 -v heat_seq -n ${m} -m ${m} -i ${iterations} -e 0.0 -c "${test_subdir}/conductivity.pgm" -t "${test_subdir}/temperature.pgm" -r 1 -k ${iterations} -L 0 -H 100 -o "${test_subdir}/out_not_precomputed_${i}.txt"
        prun -np 1 -v heat_seq -n ${m} -m ${m} -i ${iterations} -e 0.0 -c "${test_subdir}/conductivity.pgm" -t "${test_subdir}/temperature.pgm" -r 1 -k ${iterations} -L 0 -H 100 -s -o "${test_subdir}/out_precomputed_${i}.txt"
    
        table_not_precomputed=$(awk 'f;/^\s*Iterations/{f=1}' "${test_subdir}/out_not_precomputed_${i}.txt")
        table_precomputed=$(awk 'f;/^\s*Iterations/{f=1}' "${test_subdir}/out_precomputed_${i}.txt")
        
        readarray -t results_not_precomputed <<<"$table_not_precomputed"
        readarray -t results_precomputed <<<"$table_precomputed"

        length=${#results_not_precomputed[@]}

        for (( j=0; j<length; j++ ));
        do
            record_precomputed=(${results_precomputed[$j]})
            record_not_precomputed=(${results_not_precomputed[$j]})
            
            echo "${m},${record_not_precomputed[0]},${record_not_precomputed[5]},${record_precomputed[5]}" >> $summary_file
        done
        done
done