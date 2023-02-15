rm ../data.csv
(cd ../heat_seq && make autovec=1 heat_seq -B)
(cd ../heat_seq_original && make autovec=1 heat_seq_original -B)
(cd ../heat_simd && make autovec=1 heat_simd -B)

# Hand-vectorized vs original 
for m in 10 100 1000
do 
    for version in heat_simd heat_seq heat_seq_original
    do
        cd ../${version}
        pwd 
        for it in 1 2 3 4 5
        do
            let "i = 5000000/m"; 
            echo -n "${m}, 1, ${version}, " >> ../data.csv;
            prun -np 1 -v ${version} -n ${m} -m ${m} -i ${i} -e 0.0 -c ../../test/1676142089/test_m${m}_n${m}/conductivity.pgm -t ../../test/1676142089/test_m${m}_n${m}/temperature.pgm -r 1 -k 1 -L 0 -H 100;
            echo -n "${m}, ${i}, ${version}, " >> ../data.csv;
            prun -np 1 -v ${version} -n ${m} -m ${m} -i ${i} -e 0.0 -c ../../test/1676142089/test_m${m}_n${m}/conductivity.pgm -t ../../test/1676142089/test_m${m}_n${m}/temperature.pgm -r 1 -k ${i} -L 0 -H 100;    
        done
    done
done 