#!/bin/bash

for i in {1..8}; do
    echo "Testcase #$i:"
    echo "OUTPUT"
	./build/lab1_console -q < "../data/$i.in"
    echo "ANSWER"
    cat "../data/$i.ans"
    echo
done
