#!/bin/bash

mkdir ./build
gcc ./src/main.c -o ./build/main
for i in {1..8}; do
    echo "Testcase #$i:"
    echo "OUTPUT"
    ./build/main -q < "../data/$i.in"
    echo "ANSWER"
    cat "../data/$i.ans"
    echo
done
