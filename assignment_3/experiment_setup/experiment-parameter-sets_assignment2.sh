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
data_seq_new="data_seq_new.csv"
data_pth_simd="data_pth_simd.csv"
repetitions=5
start=1
experiment_dir="experiment_setup/experiments_assignment2/*"
END=16

cd ..

#heat_omp
cd heat_omp
make clean
make heat_omp
rm ${data_omp}
cd ..

#heat_pth_v0
cd heat_pth
make clean
make heat_pth
rm ${data_pth_v0}
cd ..

#heat_pth_v1
cd heat_pth_v1
make clean
make heat_pth_v1
rm ${data_pth_v1}
cd ..

#heat_pth_v2
cd heat_pth_v2
make clean
make heat_pth_v2
rm ${data_pth_v2}
cd ..

#heat_pth_v3
cd heat_pth_v3
make clean
make heat_pth_v3
rm ${data_pth_v3}
cd ..

#heat_pth_v4
cd heat_pth_v4
make clean
make heat_pth_v4
rm ${data_pth_v4}
cd ..

#heat_seq_new
cd heat_seq
make clean
make heat_seq
rm ${data_seq_new}
cd ..

#heat_seq_old
cd heat_seq_good
make clean
make heat_seq_good
rm ${data_seq_old}
cd ..

#heat_pth_simd
cd heat_pth_simd
make clean
make ${heat_pth_simd}
cd ..

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
    for i in $(seq 1 3 ${END}); do
      echo "../${e}"
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

      cd heat_seq_good
      #Sequential old
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_seq_old};
      prun -np 1 -v heat_seq_good -n ${array[1]} -m ${array[0]} -i ${maxiter} -e "../${m}" -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      cd ..
      
      cd heat_seq
      #Sequential new
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_seq_new};
      prun -np 1 -v heat_seq -n ${array[1]} -m ${array[0]} -i ${maxiter} -e "../${m}" -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      cd ..

      cd heat_omp
      #OMP
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_omp};
      prun -np 1 -v heat_omp -n ${array[1]} -m ${array[0]} -i ${maxiter} -e "../${m}" -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      cd ..

      cd heat_pth
      #PTH v0
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_pth_v0};
      prun -np 1 -v heat_pth -n ${array[1]} -m ${array[0]} -i ${maxiter} -e "../${m}" -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      cd ..

      cd heat_pth_v1
      #PTH v1 
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_pth_v1};
      prun -np 1 -v heat_pth_v1 -n ${array[1]} -m ${array[0]} -i ${maxiter} -e "../${m}" -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      cd ..   

      cd heat_pth_v2
      #PTH v2
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_pth_v2};
      prun -np 1 -v heat_pth_v2 -n ${array[1]} -m ${array[0]} -i ${maxiter} -e "../${m}" -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      cd ..

      cd heat_pth_v3
      #PTH v3
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_pth_v3};
      prun -np 1 -v heat_pth_v3 -n ${array[1]} -m ${array[0]} -i ${maxiter} -e "../${m}" -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      cd ..

      cd heat_pth_v4;
      #PTH v4
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_pth_v4};
      prun -np 1 -v heat_pth_v4 -n ${array[1]} -m ${array[0]} -i ${maxiter} -e "../${m}" -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      cd ..

      cd heat_pth_simd;
      #PTH SIMD
      echo -n "${array[0]}, ${array[1]}, ${i} " >> ${data_pth_simd};
      prun -np 1 -v heat_pth_simd -n ${array[1]} -m ${array[0]} -i ${maxiter} -e "../${m}" -c ${m} -t ${m} -r 1 -k ${maxiter} -L ${L} -H ${H} -p ${i};
      cd ..
    done
  done
done