make clean
make heat_seq

maxiter=3000
e=0.0001
L=0
H=100
data="data_assignment2.csv"
repetitions=15
start=1
experiment_dir=../../test/experiments_assignment2
END = 20

for m in "${experiment_dir}/*"; do
  for ((it=${start}; it<=${repetitions}; it++)); do
    for i in $(seq 1 $END); do
      #Sequential
      echo -n "${m}, ${i}" >> ${data};
      prun -np 1 -v heat_seq -n ${m} -m ${m} -i ${maxiter} -e ${e} -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
    done
  done
done