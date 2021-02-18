#!/bin/bash

lst=`cat changefilelist`

for i in $lst ;
do
    file=${i##*/}
    patch -u ../Mercury_src/$i -i ${file%.*}.patch
done
