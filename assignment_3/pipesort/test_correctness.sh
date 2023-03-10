#!/bin/bash
module load prun

make clean
make
nr_tests=10

# sort 10 strings of numbers with different seeds and of length up to 4000
# test if matches expected output
for ((i=1;i<nr_tests;i++)); do
  myrand=((RANDOM % 4000 + 1))
  sorted_numbers=$(prun -np 1 -v pipesort -l $myrand -s $((RANDOM)) -p | grep -oP '\d+') #match any digits
  if [[ $(echo $sorted_numbers | tr ' ' '\n' | sort -n) == $(echo $sorted_numbers | tr ' ' '\n') ]]; then
    eche "Length of sequence was $myrand"
    echo "Test passed: the output is sorted correctly."
  else
    echo "Test failed: the output is not sorted correctly."
    break
  fi
done
