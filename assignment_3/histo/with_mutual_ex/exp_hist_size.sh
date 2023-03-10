module load prun
rm res_hist_size.csv

directories=(histo_atomic histo_mutex histo_avoiding_mutex_ex histo_semaphores histo_sw_transactional)

repetitions=1
max_length=3000 # idk make 1 M

if [ ! -f res_hist_size.csv ]; then
  touch res_hist_size.csv
  echo "dir,thread,n_rows,n_cols,time_elapsed" > res_hist_size.csv
fi

#copy no mutex directory into no_mutex folder
rm -R histo_avoiding_mutual_ex
cd ..
cp -R histo_avoiding_mutual_ex with_mutual_ex
cd with_mutual_ex

for dir in "${directories[@]}"; do
  cd "$dir"
  make clean
  make


  for ((i=0;i<repetitions;i++)); do

    for thread in 1 4 8 16 32; do
      for ((n_rows=100;n_rows<max_length;n_rows=n_rows+1000)); do
        for ((n_cols=100;n_cols<max_length;n_cols=n_cols+1000)); do
          time_elapsed=$(prun -np 1 -v "$dir" -r -n $n_rows -m $n_cols -p $thread -s $((RANDOM)) | grep -oP 'Histo took:\s*\K[\d.]+')
          echo "$dir, $thread, $n_rows,$n_cols,$time_elapsed" >> ../res_hist_size.csv
        done
      done
    done

  done
  cd ..
done

rm -R histo_avoiding_mutual_ex