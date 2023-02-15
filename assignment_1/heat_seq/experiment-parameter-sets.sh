make clean
make heat_seq

M=(50 75 110 200 500 800 1000 1300 1600 2000)
maxiter=3000
e=0.0001
L=0
H=100
data="data.csv"
repetitions=15
start=1

for m in ${M[@]}; do
  file_name_square="RectangularM=${m}N=${m}.pgm";
  file_name_thin_rect="RectangularM=${m}N=100.pgm";
  file_name_wide_rect="RectangularM=100N=${m}.pgm";

  for ((it=${start}; it<=${repetitions}; it++)); do
    #Squares k = 1, k = maxiter
    echo -n "${m}, ${m}, 1, " >> ${data};
    prun -np 1 -v heat_seq -n ${m} -m ${m} -i ${maxiter} -e ${e} -c ../../test/squares/${file_name_square} -t ../../test/squares/${file_name_square} -r 1 -k 1 -L ${L} -H ${H};
    echo -n "${m}, ${m}, ${maxiter}, " >> ${data};
    prun -np 1 -v heat_seq -n ${m} -m ${m} -i ${maxiter} -e ${e} -c ../../test/squares/${file_name_square} -t ../../test/squares/${file_name_square} -r 1 -k ${maxiter} -L ${L} -H ${H};

    #Thin rectangles k = 1, k = maxiter
    echo -n "100, ${m}, 1, " >> ${data};
    prun -np 1 -v heat_seq -n 100 -m ${m} -i ${maxiter} -e ${e} -c ../../test/thin_rect/${file_name_thin_rect} -t ../../test/thin_rect/${file_name_thin_rect} -r 1 -k 1 -L ${L} -H ${H};
    echo -n "100, ${m}, ${maxiter}, " >> ${data};
    prun -np 1 -v heat_seq -n 100 -m ${m} -i ${maxiter} -e ${e} -c ../../test/thin_rect/${file_name_thin_rect} -t ../../test/thin_rect/${file_name_thin_rect} -r 1 -k ${maxiter} -L ${L} -H ${H};

    #Wide rectangles k = 1, k = maxiter
    echo -n "${m}, 100, 1, " >> ${data};
    prun -np 1 -v heat_seq -n ${m} -m 100 -i ${maxiter} -e ${e} -c ../../test/wide_rect/${file_name_wide_rect} -t ../../test/wide_rect/${file_name_wide_rect} -r 1 -k 1 -L ${L} -H ${H};
    echo -n "${m}, 100, ${maxiter}, " >> ${data};
    prun -np 1 -v heat_seq -n ${m} -m 100 -i ${maxiter} -e ${e} -c ../../test/wide_rect/${file_name_wide_rect} -t ../../test/wide_rect/${file_name_wide_rect} -r 1 -k ${maxiter} -L ${L} -H ${H};
  done
done