#!/usr/bin/bash

file=tests.csv

echo nthreads, size, time > $file

trials=100
size=(500 1000 2000)
nthread=(1 2 4 8 12)

for dim in ${size[@]}; do
    echo Multiplying $dim X $dim matrices...
    for t in ${nthread[@]}; do
        echo 'with ' $t ' threads'
        for i in $(seq 1 $trials); do
            echo -ne $i 'out of' $trials 'runs\r'
            ./mgen $dim $dim m1
            ./mgen $dim $dim m2
            ./mmult m1 m2 m3 $t >> $file
        done
        echo -ne '\n------------\n'
    done
    echo ''
    echo 'Done! Test results written to ' $file
done
