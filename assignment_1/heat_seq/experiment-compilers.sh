make heat_seq

for nbodies in 500 1000 2000 
do
    for i in 1 2 3 4 5
    do 
        echo "Run sequential with $nbodies in iteration $i"
        prun -v 1 -np 1 -script $PRUN_ETC/prun-openmpi nbody-seq $nbodies 0 ../nbody.ppm 100 1 >> seq-data.csv;
    done
    for nnodes in 1 2 4 8
    do 
        for nprocs in 1 4 16
        do 
            for i in 1 2 3 4 5
            do
                echo "Running with $nbodies bodies, $nnodes nodes, $nprocs proc/node, it $i"
                echo -n "${nnodes}, " >> par-data.csv;           
                prun -v -$nnodes -np $nprocs -script $PRUN_ETC/prun-openmpi nbody-par $nbodies 0 ../nbody.ppm 100 1 >> par-data.csv;
            done
        done
    done
done
