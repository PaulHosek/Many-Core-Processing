module load prun

make clean
make
# sort 10 strings of numbers with different seeds and of length up to 4000
# test if matches expected output
for ((i=1;i<10;i++)); do
  sorted_numbers=$(prun -np 1 -v pipesort -l $((RANDOM % 4000 + 1)) -s $((RANDOM)) -p | grep -oP '\d+') #match any digits
  if [[ $(echo $sorted_numbers | tr ' ' '\n' | sort -n) == $(echo $sorted_numbers | tr ' ' '\n') ]]; then
    echo "Test passed: the output is sorted correctly."
  else
    echo "Test failed: the output is not sorted correctly."
    break
  fi
done
