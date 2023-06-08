#!/bin/bash

log=$1

if [ $# -eq 1 ];
then
    # Your code here. (1/4)
	egrep "WARN|ERROR" $log > bug.txt
else
    case $2 in
    "--latest")
        # Your code here. (2/4)
	cat $log | tail -n 5 
    ;;
    "--find")
        # Your code here. (3/4)
	grep $3 $log > ${3}.txt
    ;;
    "--diff")
        # Your code here. (4/4)
	diff $log $3 > /dev/null 
	if [ $? -eq 0 ]
	then
		echo same
	else
		echo different
	fi 
    ;;
    esac
fi
