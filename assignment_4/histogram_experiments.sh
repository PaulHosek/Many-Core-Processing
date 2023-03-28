module load prun
rm res_hist.csv

# histo_atomic histo_mutex histo_avoiding_mutual_ex
directories=(histogram_v1 histogram_v2_1_localTBhist_strided histogram_v2_2_warp_reduc histogram_v3_SIMD)

repetitions=1
#patterns=("areas" "plasma" "gradient" "uni" "pat1" "pat2")
#patterns=("areas")
sizes=(1000 3000 5000)
if [ ! -f res_hist.csv ]; then
  touch res_hist.csv
  echo "version,pattern,thread,n_rows,n_cols,time_elapsed" > res_hist.csv
fi



for dir in "${directories[@]}"; do
  cd "$dir"
  make clean
  make
#  time_elapsed=$(prun -np 1 -native '-C TitanX --gres=gpu:1' ./myhistogram -r -s 10234 -n 107 -m 97 | awk '/histogram \(kernel\):/{print $4}')
  for ((i=0;i<repetitions;i++)); do
    for size in "${sizes[@]}" ; do
#      for pattern in "${patterns[@]}";do
#            time_elapsed=$(prun -np 1 -native '-C TitanX --gres=gpu:1' ./myhistogram -n $size -m $size -i "../../../../images/${pattern}_${size}x${size}.pgm" | awk '/histogram \(kernel\):/{print $4}')
#            echo "$dir,$pattern,$thread,$size,$size,$time_elapsed" >> ../res_hist.csv
#      done
      time_elapsed=$(prun -np 1 -native '-C TitanX --gres=gpu:1' ./myhistogram -r -n $size -m $size -s $((RANDOM)) | awk '/histogram \(kernel\):/{print $4}')
      echo "$dir,random,$thread,$size,$size,$time_elapsed" >> ../res_hist.csv
    done
  done
  cd ..
done











##histo_seq
#dir=histo_seq
#cd "$dir"
#  make clean
#  make
#  ls
#  for ((i=0;i<repetitions;i++)); do
#  for size in "${sizes[@]}"
#    do
#	for pattern in "${patterns[@]}"
#    do
#        time_elapsed=$(prun -np 1 -v "$dir" -n $size -m $size -i "../../../../images/${pattern}_${size}x${size}.pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#        echo "$dir, $pattern,-, $size,$size,$time_elapsed" >> ../res_hist.csv
#
#        time_elapsed=$(prun -np 1 -v "$dir" -n $(( $size*10 )) -m $(( $size/10 )) -i "../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#        echo "$dir, $pattern,-,$(( $size*10 )),$(( $size/10 )),$time_elapsed" >> ../res_hist.csv
#
#        time_elapsed=$(prun -np 1 -v "$dir" -n $(( $size/10 )) -m $(( $size*10 )) -i "../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#        echo "$dir, $pattern,-,$(( $size/10 )),$(( $size*10 )),$time_elapsed" >> ../res_hist.csv
#    done
#
#    time_elapsed=$(prun -np 1 -v "$dir" -r -n $size -m $size -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#    echo "$dir, random,-, $size,$size,$time_elapsed" >> ../res_hist.csv
#
#    time_elapsed=$(prun -np 1 -v "$dir" -r -n $(( $size*10 )) -m $(( $size/10 )) -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#    echo "$dir, random,-,$(( $size*10 )),$(( $size/10 )),$time_elapsed" >> ../res_hist.csv
#
#    time_elapsed=$(prun -np 1 -v "$dir" -r -n $(( $size/10 )) -m $(( $size*10 )) -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#    echo "$dir, random,-,$(( $size/10 )),$(( $size*10 )),$time_elapsed" >> ../res_hist.csv
#done
#done
#
#cd ..

# rm -R histo_avoiding_mutual_ex
# rm -R histo_seq