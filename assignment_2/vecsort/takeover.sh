rm takeover.csv
module load prun

directories=(sequential parallel_v1_onlynested parallel_v2_onlyouter)
repetitions=10

if [ ! -f takeover.csv ]; then
  touch takeover.csv
  echo "version,inner_max,len_outer,runtime" > takeover.csv
fi

for dir in "${directories[@]}"; do
  cd "$dir"
  make clean
  make
  for x in 200 1000 10000 50000 100000; do
    for l in 200 1000 10000 50000 100000; do
      for ((i=1;i<=repetitions;i++)); do

    #  time=$(prun -np 1 -v "$dir" | grep -oP 'Vecsort took:\s*\K[\d.]+')
      time=$(prun -np 1 -v "$dir" -x "$x" -l "$l" | grep -oP 'Vecsort took:\s*\K[\d.]+')
      echo "$dir,$x,$l, $time" >> ../takeover.csv
      done
    done
  done
  cd ..

done