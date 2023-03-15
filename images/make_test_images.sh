#!/bin/bash

module load prun

patterns=("areas" "plasma" "gradient" "uni" "pat1" "pat2")
sizes=(1000 3000 5000)

for size in "${sizes[@]}"
do
	for pattern in "${patterns[@]}"
    do
        prun -np 1 -v make "${pattern}_${size}x${size}.pgm"
        prun -np 1 -v make "${pattern}_$(( $size/10 ))x$(( $size*10 )).pgm"
        prun -np 1 -v make "${pattern}_$(( $size*10 ))x$(( $size/10 )).pgm"
    done
done