make clean
make heat_seq

maxiter=100
e=0.0001
L=0
H=100
data="data_assignment2_seq.csv"
repetitions=10
start=1
experiment_dir=../../test/experiments_assignment2/*

for m in ${experiment_dir}; do
  for ((it=${start}; it<=${repetitions}; it++)); do
    #Sequential
    array=($(sed -n '2p' ${m}));
    echo -n "${array[0]}, ${array[1]}," >> ${data};
    prun -np 1 -v heat_seq -n ${array[1]} -m ${array[0]} -i ${maxiter} -e ${e} -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H};
  done
done