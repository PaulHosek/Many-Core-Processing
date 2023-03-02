make clean
make heat_omp

maxiter=100
e=0.0001
L=0
H=100
data="data_assignment2_test_schedule.csv"
repetitions=1
start=1
experiment_dir=../../test/experiments_assignment2/*
END=3

for m in ${experiment_dir}; do
  for ((it=${start}; it<=${repetitions}; it++)); do
    for i in $(seq 1 ${END}); do
      array=($(sed -n '2p' ${m}));
      echo -n "${array[0]}, ${array[1]}, ${i}, " >> ${data};
      prun -np 1 -v heat_omp -n ${array[1]} -m ${array[0]} -i ${maxiter} -e ${e} -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
    done
  done
done