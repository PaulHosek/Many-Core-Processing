#!/bin/bash
module load prun

make clean
make
nr_tests=10
#!/bin/bash
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color
is_sorted=true
last_number=-1


# sort 10 strings of numbers with different seeds and of length up to 4000
# test if matches expected output
for ((i=1;i<nr_tests;i++)); do
  last_number=-1
  myrand=$((RANDOM % 4000 + 1))
  myrand2=$((RANDOM % 1000 + 1))
  seed=$((RANDOM % 10000 + 1))
  sorted_numbers=$(prun -np 1 -v pipesort -l $myrand -s $seed -p -b $myrand | grep -oP '\d+') #match any digits

  # test if is sorted
  for num in $sorted_numbers; do
    if (( $num < $last_number )); then
      is_sorted=false
      echo "not $last_number<=$num"
      break
    fi
    last_number=$num
  done

  # return
  if $is_sorted; then
    echo -e "Length of sequence was ${BLUE} $myrand ${NC}, with bb size ${BLUE}$myrand2${NC}, seed=${BLUE}$seed${NC}"
    echo -e "${GREEN}Passed.${NC}"
  else
    echo -e "Length of sequence was ${BLUE} $myrand ${NC}, with bb size ${BLUE}$myrand2${NC}, seed=${BLUE}$seed${NC}"
    echo -e "${RED}Failed${NC}"
    echo "$sorted_numbers"
    break
  fi
done
