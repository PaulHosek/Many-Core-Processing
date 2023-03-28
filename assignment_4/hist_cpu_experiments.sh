module load prun

# histo_atomic histo_mutex histo_avoiding_mutual_ex
directories=(histo_avoiding_mutual_ex)

repetitions=3
max_size=4097
if [ ! -f res_hist_cpu.csv ]; then
  touch res_hist_cpu.csv
  echo "version,pattern,n_rows,n_cols,time_elapsed" > res_hist_cpu.csv
fi



for dir in "${directories[@]}"; do
  cd "$dir"
  make clean
  make
  for ((i=0;i<repetitions;i++)); do

    for ((size=56;size<max_size;size = size+64)) ; do
      
      time_elapsed=$(prun -np 1 -v "$dir" -r -n $size -m $size -p 16 -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
      echo "$dir,random,$size,$size,$time_elapsed" >> ../res_hist_cpu.csv
      
      time_elapsed=$(prun -np 1 -v "$dir" -r -n $((size-1)) -m $((size-1)) -p 16 -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
      echo "$dir,random,$size,$size,$time_elapsed" >> ../res_hist_cpu.csv
    done
    for size in 32 128 256 512 1024 2048 4096; do
      
      time_elapsed=$(prun -np 1 -v "$dir" -r -n $size -m $size -p 16 -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
      echo "$dir,random,$size,$size,$time_elapsed" >> ../res_hist_cpu.csv
      
      time_elapsed=$(prun -np 1 -v "$dir" -r -n $((size-1)) -m $((size-1)) -p 16 -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
      echo "$dir,random,$size,$size,$time_elapsed" >> ../res_hist_cpu.csv

    done

  done
  cd ..
done


#histo_seq
dir=histo_seq
cd "$dir"
  make clean
  make
  ls
  for ((i=0;i<repetitions;i++)); do

    for ((size=56;size<max_size;size = size+64)) ; do

      time_elapsed=$(prun -np 1 -v "$dir" -r -n $((size-1)) -m $((size-1)) -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
      echo "$dir,random,-,$((size-1)),$((size-1)),$time_elapsed" >> ../res_hist_cpu.csv

      time_elapsed=$(prun -np 1 -v "$dir" -r -n $((size-1)) -m $((size-1))e -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
      echo "$dir,random,-,$((size-1)),$((size-1)),$time_elapsed" >> ../res_hist_cpu.csv
  done
  for size in 32 128 256 512 1024 2048 4096; do
          time_elapsed=$(prun -np 1 -v "$dir" -r -n $((size-1)) -m $((size-1)) -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
          echo "$dir,random,-,$((size-1)),$((size-1)),$time_elapsed" >> ../res_hist_cpu.csv

          time_elapsed=$(prun -np 1 -v "$dir" -r -n $((size-1)) -m $((size-1)) -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
          echo "$dir,random,-,$((size-1)),$((size-1)),$time_elapsed" >> ../res_hist_cpu.csv

        done
done

cd ..



