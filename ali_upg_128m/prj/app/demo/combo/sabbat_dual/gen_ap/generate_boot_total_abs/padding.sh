#!/bin/bash

if [ $# != 2 ] ; then
	echo "Input Error,USAGE: $0 inuput_file output_file";
	exit 1;
fi


input_file=$1
output_file=$2

echo "input_file="$input_file
echo "output_file="$output_file

len=$(stat -c %s $input_file)
padding_len=$((16-($len%16)))
echo "padding_len="$padding_len
rm -f padding.bin
dd if=/dev/zero of=padding.bin bs=1 count=$padding_len oflag=append

rm -f $output_file
(dd if=$input_file ; dd if=padding.bin) > $output_file
rm -f padding.bin
