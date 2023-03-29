module load prun
module load cuda91/toolkit/9.1.85
rm res_heat.csv

directories=(../../../assignment_3/heat_omp ../../../assignment_3/heat_pth_v3 ../../../assignment_3/heat_pth_v4 ../../heat_cuda ../../heat_cuda_v2)
versions=(heat_omp heat_pth_v3 heat_pth_v4)

versions_cpu=(heat_omp heat_pth_v3 heat_pth_v4)
versions_gpu=(heat_cuda heat_cuda_v2)

repetitions=10
#threads=(1 2 4 8 16)#
threads=(16)
patterns=("areas" "plasma" "gradient" "uni" "pat1" "pat2")
sizes=(1000 3000 5000)

if [ ! -f res_heat.csv ]; then
  touch res_heat.csv
  echo "version,pattern,thread,n_rows,n_cols,runtime,flops" > res_heat.csv
fi

for i in "${versions[@]}"
do
	rm -R "$i"
done

for i in "${directories[@]}"
do
	cp -r "$i" .
done

for version in "${versions_cpu[@]}"; do
  cd "$version"
  make clean
  make
  for ((i=0;i<repetitions;i++)); do
  for size in "${sizes[@]}"
    do
      iterations=$((10000000000 / (size * size)))
  for thread in "${threads[@]}"
  do
	for pattern in "${patterns[@]}"
    do
        echo "prun -np 1 -v ./$version -n $size -m $size -p $thread -t ../../../../images/${pattern}_${size}x${size}.pgm -c ../../../../images/${pattern}_${size}x${size}.pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100"
        output=($(prun -np 1 -v ./$version -n $size -m $size -p $thread -t ../../../../images/${pattern}_${size}x${size}.pgm -c ../../../../images/${pattern}_${size}x${size}.pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100 | tail -1))
        echo $output
        echo "$version, $pattern, $thread, $size,$size,${output[5]},${output[6]}" >> ../res_heat.csv
        
        echo "prun -np 1 -v ./$version -n $(( $size*10 )) -m $(( $size/10 )) -p $thread -t ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -c ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100"
        output=($(prun -np 1 -v ./$version -n $(( $size*10 )) -m $(( $size/10 )) -p $thread -t ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -c ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100 | tail -1))
        echo "$version, $pattern, $thread,$(( $size*10 )),$(( $size/10 )),${output[5]},${output[6]}" >> ../res_heat.csv

        echo "prun -np 1 -v ./$version -n $(( $size/10 )) -m $(( $size*10 )) -p $thread -t ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -c ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100"
        output=($(prun -np 1 -v ./$version -n $(( $size/10 )) -m $(( $size*10 )) -p $thread -t ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -c ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100 | tail -1))
        echo "$version, $pattern, $thread,$(( $size/10 )),$(( $size*10 )),${output[5]},${output[6]}" >> ../res_heat.csv
    done
done
done

done

cd ..
done

for version in "${versions_gpu[@]}"; do
  cd "$version"
  make clean
  make
  ls
  for ((i=0;i<repetitions;i++)); do
  for size in "${sizes[@]}"
    do
      iterations=$((10000000000 / (size * size)))
  for thread in "${threads[@]}"
  do
	for pattern in "${patterns[@]}"
    do
        echo "prun -np 1 -native '-C TitanX --gres=gpu:1' ./$version -n $size -m $size -p $thread -t ../../../../images/${pattern}_${size}x${size}.pgm -c ../../../../images/${pattern}_${size}x${size}.pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100"
        output=($(prun -np 1 -native '-C TitanX --gres=gpu:1' ./$version -n $size -m $size -p $thread -t ../../../../images/${pattern}_${size}x${size}.pgm -c ../../../../images/${pattern}_${size}x${size}.pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100 | tail -1))
        echo "$version, $pattern, $thread, $size,$size,${output[5]},${output[6]}" >> ../res_heat.csv

        echo "prun -np 1 -native '-C TitanX --gres=gpu:1' ./$version -n $(( $size*10 )) -m $(( $size/10 )) -p $thread -t ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -c ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100"
        output=($(prun -np 1 -native '-C TitanX --gres=gpu:1' ./$version -n $(( $size*10 )) -m $(( $size/10 )) -p $thread -t ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -c ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100 | tail -1))
        echo "$version, $pattern, $thread,$(( $size*10 )),$(( $size/10 )),${output[5]},${output[6]}" >> ../res_heat.csv

        echo "prun -np 1 -native '-C TitanX --gres=gpu:1' ./$version -n $(( $size/10 )) -m $(( $size*10 )) -p $thread -t ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -c ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100"
        output=($(prun -np 1 -native '-C TitanX --gres=gpu:1' ./$version -n $(( $size/10 )) -m $(( $size*10 )) -p $thread -t ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -c ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100 | tail -1))
        echo "$version, $pattern, $thread,$(( $size/10 )),$(( $size*10 )),${output[5]},${output[6]}" >> ../res_heat.csv
    done
done
done

done

cd ..
done

#histo_seq
version="histo_seq"
rm -R $version
cp "../../../assignment_3/${version}" .
cd "$version"
  make clean
  make
  for ((i=0;i<repetitions;i++)); do
  for size in "${sizes[@]}"
  
    do
    iterations=$((10000000000 / (size * size)))
	for pattern in "${patterns[@]}"
    do
        echo "prun -np 1 -v ./$version -n $size -m $size -t ../../../../images/${pattern}_${size}x${size}.pgm -c ../../../../images/${pattern}_${size}x${size}.pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100"
        output=($(prun -np 1 -v ./$version -n $size -m $size -t ../../../../images/${pattern}_${size}x${size}.pgm -c ../../../../images/${pattern}_${size}x${size}.pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100 | tail -1))
        echo "$version, $pattern, N/A, $size,$size,${output[5]},${output[6]}" >> ../res_heat.csv

        echo "prun -np 1 -v ./$version -n $(( $size*10 )) -m $(( $size/10 )) -t ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -c ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100"
        output=($(prun -np 1 -v ./$version -n $(( $size*10 )) -m $(( $size/10 )) -t ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -c ../../../../images/${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100 | tail -1))
        echo "$version, $pattern, N/A,$(( $size*10 )),$(( $size/10 )),${output[5]},${output[6]}" >> ../res_heat.csv

        echo "prun -np 1 -v ./$version -n $(( $size/10 )) -m $(( $size*10 )) -t ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -c ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100"
        output=($(prun -np 1 -v ./$version -n $(( $size/10 )) -m $(( $size*10 )) -t ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -c ../../../../images/${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm -i ${iterations} -e 0.000000001 -k 100 -L 0 -H 100 | tail -1))
        echo "$version, $pattern, N/A,$(( $size/10 )),$(( $size*10 )),${output[5]},${output[6]}" >> ../res_heat.csv
    done
done
done

cd ..