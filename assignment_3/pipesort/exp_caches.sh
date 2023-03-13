#!/bin/bash
module load prun

rm -f res_caches.txt

make clean
make

repetitions=1


# Set the number of repetitions and the duration of each repetition
repetitions=1
duration_perf=10

# Create output file if it doesn't exist
if [ ! -f res_caches.txt ]; then
  touch res_caches.txt
  echo "n_nums,buffersize,task-clock,cycles,instructions,cache-references,cache-misses" > res_caches.txt
fi

# Set the events to measure with perf
#events="task-clock,cycles,instructions,cache-references,cache-misses"
events="cache-misses"
#perf stat -e $events -o /dev/null ./pipesort  >> res_caches.txt

timeout 10s perf stat -e $events -o /dev/null ./pipesort &


perf stat -e $events -o res_caches.txt ./pipesort

## Iterate over different values of n_nums and buffersize
#for n_nums in 10 100 1000 3000 4000; do
#  for buffersize in 1 5 10 100 500 700 1000; do
#    # Run the command in a loop for the specified duration
#    start_time=$(date +%s)
#    while (( $(date +%s) - start_time < duration_perf )); do
#      perf stat -e $events -o /dev/null ./pipesort -l $n_nums -s $((RANDOM)) -b $buffersize >/dev/null 2>&1
#    done
#
#    perf stat -e $events ./pipesort -l $n_nums -s $((RANDOM)) -b $buffersize 2>&1 | grep -oP '(\d+\.?\d*)' | tr '\n' ',' >> res_caches.txt
#  done
#done
