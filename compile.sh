#!/bin/bash
cat $1
echo ''
./main $1
gcc a.s -o out.o
./out.o
echo $?
