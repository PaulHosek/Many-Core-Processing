module load prun
make clean
make


repetitions=20

if [ ! -f res_buffersize_14.csv ]; then
  touch res_buffersize_14.csv
  echo "n_nums,buffersize,runtime" > res_buffersize_14.csv
fi

for ((i=0;i<repetitions;i++)) do
  for n_nums in 10 50 100 500 1000 2000 3000 4000; do
    for buffersize in 1 2 3 4 5 6 7 8 9 10 12 14 16 18 20 25 30 35 40 45 50 100 200 300 400 500 600 700 800 900 1000; do
    time_elapsed=$(prun -np 1 -v pipesort -l $n_nums -s $((RANDOM)) | grep -oP 'Pipesort took:\s*\K[\d.]+')
    echo "$n_nums,$buffersize,$time_elapsed" >> res_buffersize_14.csv
    done
  done
done

#nohup ./exp_buffersize.sh > output.log 2>&1 && notify-send "Script completed" &
