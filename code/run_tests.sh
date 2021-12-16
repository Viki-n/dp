#!/bin/bash

while read test; do
    for program in test*; do
        echo $program $test
        echo $program $test >> $1
        ./data_source $test | ./$program >> $1 2>> $1
    done
done
