rm data.csv
module load prun

directories=(sequential parallel_v1_onlynested parallel_v2_onlyouter parallel_v3_both parallel_v4_inline parallel_v5_o3)
repetitions=100

if [ ! -f data.csv ]; then
  touch data.csv
  echo "version, runtime" > data.csv
fi

for dir in "${directories[@]}"; do
  cd "$dir"
  make clean
  make
  for ((i=1;i<=repetitions;i++)); do

#  time=$(prun -np 1 -v "$dir" | grep -oP 'Vecsort took:\s*\K[\d.]+')
  time=$(prun -np 1 -v "$dir" -x 10000 -l 100000 | grep -oP 'Vecsort took:\s*\K[\d.]+')
  echo "$dir,$time" >> ../data.csv
  done
  cd ..

done
