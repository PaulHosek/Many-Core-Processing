#heat_omp
cd heat_omp
make clean
make heat_omp
cd ..

#heat_pth_v0
cd heat_pth
make clean
make heat_pth
cd ..

#heat_pth_v1
cd heat_pth_v1
make clean
make heat_pth_v1
cd ..

#heat_pth_v2
cd heat_pth_v2
make clean
make heat_pth_v2
cd ..

#heat_pth_v3
cd heat_pth_v3
make clean
make heat_pth_v3
cd ..

#heat_pth_v4
cd heat_pth_v4
make clean
make heat_pth_v4
cd ..

#heat_seq_new
cd heat_seq
make clean
make heat_seq
cd ..

#heat_seq_old
cd heat_seq_good
make clean
make heat_seq_good
cd ..

maxiter=100
e=0.0001
L=0
H=100
data_omp="data_omp.csv"
data_pth_v0="data_pth_v0.csv"
data_pth_v1="data_pth_v1.csv"
data_pth_v2="data_pth_v2.csv"
data_pth_v3="data_pth_v3.csv"
data_pth_v4="data_pth_v4.csv"
data_seq_old="data_seq_old.csv"
data_seq_good="data_seq_new.csv"
repetitions=5
start=1
experiment_dir=experiments_assignment2/*
END=16

#2.000.000 cell counts ... 1 iteration (20000 x 100)
#10.000 cell counts ... 1 iteration (100 x 100)
#25.000.000 cell counts ... 1 iteration (5000 x 5000)
#1.000.000 cell counts ... 1 iteration (1000 x 1000)

#500.000.000 ... 250 iterations (20000 x 100)
#500.000.000 ... 50000 iterations (100 x 100)
#500.000.000 ...  500 iterations (1000 x 1000)
#500.000.000 cell counts ... 50 iterations (5000 x 5000)
   
for m in ${experiment_dir}; do
  for ((it=${start}; it<=${repetitions}; it++)); do
    for i in $(seq 1 4 ${END}); do
      array=($(sed -n '2p' ${m}));
      if [ ${array[0]} == "1000" ]; then 
        maxiter=500;
      elif [ ${array[0]} == "5000" ]; then
        maxiter=50;
      elif [ ${array[0]} == "20000" ] || [ ${array[0]} == "20000" ]; then
        maxiter=250;
      else
        maxiter=50000;
      fi

      #Sequential old
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_seq_old};
      prun -np 1 -v heat_omp -n ${array[1]} -m ${array[0]} -i ${maxiter} -e ${e} -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};

      #Sequential new
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_seq_new};
      prun -np 1 -v heat_omp -n ${array[1]} -m ${array[0]} -i ${maxiter} -e ${e} -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};

      #OMP new 
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_seq_old};
      prun -np 1 -v heat_omp -n ${array[1]} -m ${array[0]} -i ${maxiter} -e ${e} -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      #PTH v0

      #PTH v1

      #PTH v2

      #PTH v3

      #PTH v4
    done
  done
done