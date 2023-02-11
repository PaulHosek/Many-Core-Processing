
make clean 
rm data.out

for comp in gcc  
do
    for optl in O0 O1 O2 O3 
    do 
        make compiler=$comp optlevel=$optl heat_seq
        for i in 1 2 3 4 5
        do
            prun -np 1 -v heat_seq -n 150 -m 100 -i 30000 -e 0.0001 -c ../../images/pat3_100x150.pgm -t ../../images/pat3_100x150.pgm -r 1 -k 3000 -L 0 -H 100 >> data.out
        done 
        make clean 
    done 
done
