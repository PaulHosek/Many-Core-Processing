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

# sort 10 strings of numbers with different seeds and of length up to 4000
# test if matches expected output
for ((i=1;i<nr_tests;i++)); do
  myrand=$((RANDOM % 4000 + 1))
  myrand2=$((RANDOM % 1000 + 1))
  sorted_numbers=$(prun -np 1 -v pipesort -l $myrand -s $((RANDOM)) -p -b $myrand | grep -oP '\d+') #match any digits
  if [[ $(echo $sorted_numbers | tr ' ' '\n' | sort -n) == $(echo $sorted_numbers | tr ' ' '\n') ]]; then
    echo -e "Length of sequence was ${BLUE} $myrand ${NC}, with bb size ${BLUE}$myrand2${NC}"
    echo -e "Test passed: the output is sorted ${GREEN}correctly.${NC}"
  else
    echo -e "${RED} Test failed: the output is not sorted correctly.${NC}"
    break
  fi
done
