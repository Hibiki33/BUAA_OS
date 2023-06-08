#!/bin/bash

cat $1 | head -n 8 | tail -n +8 > $2
cat $1 | head -n 32 | tail -n +32 >> $2
cat $1 | head -n 128 | tail -n +128 >> $2
cat $1 | head -n 512 | tail -n +512 >> $2
cat $1 | head -n 1024 | tail -n +1024 >> $2
