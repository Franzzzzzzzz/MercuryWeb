#!/bin/bash

lst=`cat changefilelist`

for i in $lst ;
do
    cp ../Mercury_src/$i .
    file=${i##*/}
    diff -u ../../MercuryDPM/MercurySource/$i $file > ${file%.*}.patch
done
