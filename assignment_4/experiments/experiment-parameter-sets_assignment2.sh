make clean
make heat_omp

maxiter=100
e=0.0001
L=0
H=100
data="data_assignment4.csv"
repetitions=5
start=1
experiment_dir=experiments_assignment4/*
END=25

for m in ${experiment_dir}; do
  for ((it=${start}; it<=${repetitions}; it++)); do
    for i in $(seq 1 ${END}); do
      #Sequential
      array=($(sed -n '2p' ${m}));
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data};
      prun -np 1 -v heat_omp -n ${array[1]} -m ${array[0]} -i ${maxiter} -e ${e} -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
    done
  done
done