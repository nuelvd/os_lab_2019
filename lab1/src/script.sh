#!/bin/sh
sum=0
count=150
while read -r num; do
    sum=$((sum+num))
done < numbers.txt
echo $sum