module load prun
rm res_hist.csv

# histo_atomic histo_mutex histo_avoiding_mutual_ex
directories=(histo_atomic histo_mutex histo_semaphores histo_sw_transactional histo_avoiding_mutual_ex)

repetitions=3
threads=(1 2 4 8 16)
patterns=("areas" "plasma" "gradient" "uni" "pat1" "pat2")
sizes=(1000 3000 5000)

if [ ! -f res_hist.csv ]; then
  touch res_hist.csv
  echo "version,pattern,thread,n_rows,n_cols,time_elapsed" > res_hist.csv
fi

#copy no mutex directory into no_mutex folder
rm -R histo_avoiding_mutual_ex
rm -R histo_seq
cp -R ../histo_avoiding_mutual_ex .
cp -R ../histo_seq .

for dir in "${directories[@]}"; do
  cd "$dir"
  make clean
  make
  ls
  for ((i=0;i<repetitions;i++)); do
  for size in "${sizes[@]}"
    do
  for thread in "${threads[@]}"
  do
	for pattern in "${patterns[@]}"
    do
        echo "prun -np 1 -v $dir -n $size -m $size -p $thread -i ../../../../images/${pattern}_${size}x${size}.pgm"
        time_elapsed=$(prun -np 1 -v "$dir" -n $size -m $size -p $thread -i "../../../../images/${pattern}_${size}x${size}.pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
        echo "$dir, $pattern, $thread, $size,$size,$time_elapsed" >> ../res_hist.csv

        echo "prun -np 1 -v $dir -n $(( $size*10 )) -m $(( $size/10 )) -p $thread -i ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm"
        time_elapsed=$(prun -np 1 -v "$dir" -n $(( $size*10 )) -m $(( $size/10 )) -p $thread -i "../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
        echo "$dir, $pattern, $thread,$(( $size*10 )),$(( $size/10 )),$time_elapsed" >> ../res_hist.csv

        echo "prun -np 1 -v $dir -n $(( $size/10 )) -m $(( $size*10 )) -p $thread -i ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm"
        time_elapsed=$(prun -np 1 -v "$dir" -n $(( $size/10 )) -m $(( $size*10 )) -p $thread -i "../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
        echo "$dir, $pattern, $thread,$(( $size/10 )),$(( $size*10 )),$time_elapsed" >> ../res_hist.csv
    done

    time_elapsed=$(prun -np 1 -v "$dir" -r -n $size -m $size -p $thread -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
    echo "$dir, random, $thread, $size,$size,$time_elapsed" >> ../res_hist.csv

    time_elapsed=$(prun -np 1 -v "$dir" -r -n $(( $size*10 )) -m $(( $size/10 )) -p $thread -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
    echo "$dir, random, $thread,$(( $size*10 )),$(( $size/10 )),$time_elapsed" >> ../res_hist.csv

    time_elapsed=$(prun -np 1 -v "$dir" -r -n $(( $size/10 )) -m $(( $size*10 )) -p $thread -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
    echo "$dir, random, $thread,$(( $size/10 )),$(( $size*10 )),$time_elapsed" >> ../res_hist.csv
done
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
  for size in "${sizes[@]}"
    do
	for pattern in "${patterns[@]}"
    do
        time_elapsed=$(prun -np 1 -v "$dir" -n $size -m $size -i "../../../../images/${pattern}_${size}x${size}.pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
        echo "$dir, $pattern,-, $size,$size,$time_elapsed" >> ../res_hist.csv

        time_elapsed=$(prun -np 1 -v "$dir" -n $(( $size*10 )) -m $(( $size/10 )) -i "../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
        echo "$dir, $pattern,-,$(( $size*10 )),$(( $size/10 )),$time_elapsed" >> ../res_hist.csv

        time_elapsed=$(prun -np 1 -v "$dir" -n $(( $size/10 )) -m $(( $size*10 )) -i "../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
        echo "$dir, $pattern,-,$(( $size/10 )),$(( $size*10 )),$time_elapsed" >> ../res_hist.csv
    done

    time_elapsed=$(prun -np 1 -v "$dir" -r -n $size -m $size -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
    echo "$dir, random,-, $size,$size,$time_elapsed" >> ../res_hist.csv

    time_elapsed=$(prun -np 1 -v "$dir" -r -n $(( $size*10 )) -m $(( $size/10 )) -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
    echo "$dir, random,-,$(( $size*10 )),$(( $size/10 )),$time_elapsed" >> ../res_hist.csv

    time_elapsed=$(prun -np 1 -v "$dir" -r -n $(( $size/10 )) -m $(( $size*10 )) -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
    echo "$dir, random,-,$(( $size/10 )),$(( $size*10 )),$time_elapsed" >> ../res_hist.csv
done
done

cd ..

# rm -R histo_avoiding_mutual_ex
# rm -R histo_seq