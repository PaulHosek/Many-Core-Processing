rm exp_outer_thread.csv
module load prun

repetitions=10
max_threads=16

if [ ! -f exp_outer_thread.csv ]; then
  touch exp_outer_thread.csv
  echo "nr_outer_threads,runtime" > exp_outer_thread.csv
fi

for ((t=1;t<=max_threads;t++)); do
  make clean
  make
  for ((i=1;i<=repetitions;i++)); do

  time=$(prun -np 1 -v parallel_v3_both -t "$t" -x 10000 -l 100000  | grep -oP 'Vecsort took:\s*\K[\d.]+')
  echo "$t,$time" >> exp_outer_thread.csv
  done
done
