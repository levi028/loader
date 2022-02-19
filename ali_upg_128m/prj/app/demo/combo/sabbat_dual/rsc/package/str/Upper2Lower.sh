#!/bin/bash

for file in $(find . -name "str_*.h")
do
	echo "Processing $file "
#	ProcessFile "$file "
	sed -i '/unsigned char/s/.*/\L&/' $file 
	
done