#!/bin/bash

mkdir mydir
chmod 777 mydir
touch myfile
echo 2023 > myfile
mv moveme ./mydir
cp copyme ./mydir
mv ./mydir/copyme ./mydir/copied
cat readme
gcc bad.c 2> err.txt
mkdir gen
if [ $# -eq 1 ]
then
	i=1
	while [ $i -le $1 ]
	do
		touch ./gen/${i}.txt
		let i+=1
	done	
else
	i=1
	while [ $i -le 10 ]
	do
		touch ./gen/${i}.txt
		let i+=1
	done
fi
