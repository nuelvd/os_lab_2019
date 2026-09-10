#!/bin/sh
for i in $(seq 150); do
    od -An -N4 -tu4 /dev/random
done > numbers.txt