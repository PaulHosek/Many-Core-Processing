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

for m in "${experiment_dir}/*"; do
  for ((it=${start}; it<=${repetitions}; it++)); do
    #Sequential
    echo -n "${m}, ${m}, ${maxiter}, " >> ${data};
    prun -np 1 -v heat_seq -n ${m} -m ${m} -i ${maxiter} -e ${e} -c ../../test/squares/${file_name_square} -t ../../test/squares/${file_name_square} -r 1 -k ${maxiter} -L ${L} -H ${H};
  done
done