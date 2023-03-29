module load prun
module load cuda91/toolkit/9.1.85
rm res_hist_gpu.csv

# histo_atomic histo_mutex histo_avoiding_mutual_ex
directories=(histogram_v1 histogram_v2_1_localTBhist_strided histogram_v2_2_warp_reduc histogram_v3_SIMD)

repetitions=3
max_size=4097
if [ ! -f res_hist_gpu.csv ]; then
  touch res_hist_gpu.csv
  echo "version,pattern,n_rows,n_cols,time_elapsed" > res_hist_gpu.csv
fi


for dir in "${directories[@]}"; do
  cd "$dir"
  make clean
  make
  for ((i=0;i<repetitions;i++)); do

    for ((size=64;size<max_size;size = size+64)) ; do
      time_elapsed=$(prun -np 1 -native '-C TitanX --gres=gpu:1' ./myhistogram -r -n "$size" -m "$size" -s $((RANDOM)) | awk '/histogram \(kernel\):/{print $4}')
      echo "$dir,random,$size,$size,$time_elapsed" >> ../res_hist_gpu.csv
      echo "$time_elapsed"
      time_elapsed=$(prun -np 1 -native '-C TitanX --gres=gpu:1' ./myhistogram -r -n $((size+1)) -m $((size+1)) -s $((RANDOM)) | awk '/histogram \(kernel\):/{print $4}')
      echo "$dir,random,$((size+1)),$((size+1)),$time_elapsed" >> ../res_hist_gpu.csv
    done
    for size in 32 128 256 512 1024 2048 4096; do
      time_elapsed=$(prun -np 1 -native '-C TitanX --gres=gpu:1' ./myhistogram -r -n "$size" -m "$size" -s $((RANDOM)) | awk '/histogram \(kernel\):/{print $4}')
      echo "$dir,random,$size,$size,$time_elapsed" >> ../res_hist_gpu.csv

      time_elapsed=$(prun -np 1 -native '-C TitanX --gres=gpu:1' ./myhistogram -r -n $((size+1)) -m $((size+1)) -s $((RANDOM)) | awk '/histogram \(kernel\):/{print $4}')
      echo "$dir,random,$((size+1)),$((size+1)),$time_elapsed" >> ../res_hist_gpu.csv
    done

  done
  cd ..
done








#patterns=("areas" "plasma" "gradient" "uni" "pat1" "pat2")
#patterns=("areas")
#sizes=(64 128 256 512 1024 2048 4096)#    for size in "${sizes[@]}" ; do

#      for pattern in "${patterns[@]}";do
#            time_elapsed=$(prun -np 1 -native '-C TitanX --gres=gpu:1' ./myhistogram -n $size -m $size -i "../../../../images/${pattern}_${size}x${size}.pgm" | awk '/histogram \(kernel\):/{print $4}')
#            echo "$dir,$pattern,$size,$size,$time_elapsed" >> ../res_hist_gpu.csv
#      done

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
#        echo "$dir, $pattern,-, $size,$size,$time_elapsed" >> ../res_hist_gpu.csv
#
#        time_elapsed=$(prun -np 1 -v "$dir" -n $(( $size*10 )) -m $(( $size/10 )) -i "../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#        echo "$dir, $pattern,-,$(( $size*10 )),$(( $size/10 )),$time_elapsed" >> ../res_hist_gpu.csv
#
#        time_elapsed=$(prun -np 1 -v "$dir" -n $(( $size/10 )) -m $(( $size*10 )) -i "../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm" | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#        echo "$dir, $pattern,-,$(( $size/10 )),$(( $size*10 )),$time_elapsed" >> ../res_hist_gpu.csv
#    done
#
#    time_elapsed=$(prun -np 1 -v "$dir" -r -n $size -m $size -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#    echo "$dir, random,-, $size,$size,$time_elapsed" >> ../res_hist_gpu.csv
#
#    time_elapsed=$(prun -np 1 -v "$dir" -r -n $(( $size*10 )) -m $(( $size/10 )) -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#    echo "$dir, random,-,$(( $size*10 )),$(( $size/10 )),$time_elapsed" >> ../res_hist_gpu.csv
#
#    time_elapsed=$(prun -np 1 -v "$dir" -r -n $(( $size/10 )) -m $(( $size*10 )) -s $((RANDOM)) | grep -oP 'Histo took:\s*\K([0-9]+\.[0-9]*|\.?[0-9]+)([eE][+-][0-9]+)?')
#    echo "$dir, random,-,$(( $size/10 )),$(( $size*10 )),$time_elapsed" >> ../res_hist_gpu.csv
#done
#done
#
#cd ..

# rm -R histo_avoiding_mutual_ex
# rm -R histo_seq