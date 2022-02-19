#!/bin/bash

usage(){
	echo -e "Usage: $0 [cmd] [OPTIONS]"

	echo -e "Usage: pad [-d DATA] [--data=DATA] [--help] FILE SIZE"
	echo -e "Usage: align [-d DATA] [--data=DATA] [--help] FILE SIZE"
	echo -e "Usage: merge2 [--output_offset=SEEK]"
	echo -e "              [--input_offset=SKIP]"
	echo -e "              [-s SIZE] [--size=SIZE]"
	echo -e "              [--help] FILE SUB_FILE"
	echo -e "Usage: mergen [--output=FILE] [--help] FILE1 FILE2 .. FILEn"
	echo -e "Usage: extract [--output=FILE]"
	echo -e "               [--input_offset=SKIP]"
	echo -e "               [--help] FILE SIZE"
	echo -e "Usage: -h, --help:       Give this help list"
	exit 1
}

usage_pad() {
	echo -e "Usage: pad [-d DATA] [--data=DATA] [--help] FILE SIZE"
	echo -e ""
	echo -e "pad -- pad utility"
	echo -e "  FILE                     input file"
	echo -e "  SIZE                     pad data until the size is SIZE"
	echo -e ""
	echo -e "  -d, --data=DATA          pad with DATA, default DATA=0xff"
	echo -e "  --help                   Give this help list"
	exit 1
}

usage_align() {
	echo -e "Usage: align [-d DATA] [--data=DATA] [--help] FILE SIZE"
	echo -e ""
	echo -e "align -- align utility"
	echo -e "  FILE                     input file"
	echo -e "  SIZE                     pad data until the size is aligned by SIZE"
	echo -e ""
	echo -e "  -d, --data=DATA          align with DATA, default DATA=0xff"
	echo -e "  --help                   Give this help list"
	exit 1
}

usage_merge2() {
	echo -e "Usage: merge2 [--output_offset=SEEK]"
	echo -e "              [--input_offset=SKIP]"
	echo -e "              [-s SIZE] [--size=SIZE]"
	echo -e "              [--help] FILE SUB_FILE"
	echo -e ""
	echo -e "merge2 -- merge2 utility"
	echo -e "  FILE                     input file"
	echo -e "  SUB_FILE                 the file to be merged to FILE"
	echo -e ""
	echo -e "  --output_offset=SEEK     default SEEK is size of FILE"
	echo -e "  --input_offset=SKIP      default SKIP is 0"
	echo -e "  -s, --size=SIZE          default SIZE is size of SUB_FILE"
	echo -e "  --help                   Give this help list"
	exit 1
}

usage_mergen() {
	echo -e "Usage: mergen [--output=FILE] [--help] FILE1 FILE2 .. FILEn"
	echo -e ""
	echo -e "mergen -- mergen utility"
	echo -e "  FILE1 .. FILEn           input files"
	echo -e ""
	echo -e "  --output=FILE            default FILE is output.bin"
	echo -e "  --help                   Give this help list"
	exit 1
}

usage_extract() {
	echo -e "Usage: extract [--output=FILE]"
	echo -e "               [--input_offset=SKIP]"
	echo -e "               [--help] FILE SIZE"
	echo -e ""
	echo -e "extract -- extract utility"
	echo -e "  FILE                     input file"
	echo -e "  SIZE                     size of data to be extracted,"
	echo -e "                           default SIZE is SIZE_OF_FILE - SKIP"
	echo -e ""
	echo -e "  --output=FILE            default FILE is output.bin"
	echo -e "  --input_offset=SKIP      default SKIP is 0"
	echo -e "  --help                   Give this help list"
	exit 1
}

while [ $# -gt 0 ]; do
	case $1 in
	-h|--help)
		usage
		shift 1
		;;
	pad)
		ARG_FUNC=pad
		shift 1
		ARG_DATA=0xff
		while [ $# -gt 0 ]; do
			case $1 in
			--help)
				usage_pad
				shift 1
				;;
			-d)
				shift 1
				ARG_DATA=$1
				shift 1
				;;
			--data=*)
				ARG_DATA=`echo $1 | awk -F '=' '{print $2}'`
				shift 1
				;;
			*)
				ARG_OTHER="$ARG_OTHER $1"
				shift 1
				;;
			esac
		done
		ARG_FILE=`echo $ARG_OTHER | awk '{print $1}'`
		ARG_SIZE=`echo $ARG_OTHER | awk '{print $2}'`
		if [ "$ARG_FILE" = "" ] || [ ! -f $ARG_FILE ] ; then
			echo "error: file $ARG_FILE not found!"
			exit 1
		fi

		if [ "$ARG_SIZE" = "" ] ; then
			echo "parameter error for SIZE"
			exit 1
		fi
		;;
	align)
		ARG_FUNC=align
		shift 1
		ARG_DATA=0xff
		while [ $# -gt 0 ]; do
			case $1 in
			--help)
				usage_align
				shift 1
				;;
			-d)
				shift 1
				ARG_DATA=$1
				shift 1
				;;
			--data=*)
				ARG_DATA=`echo $1 | awk -F '=' '{print $2}'`
				shift 1
				;;
			*)
				ARG_OTHER="$ARG_OTHER $1"
				shift 1
				;;
			esac
		done
		ARG_FILE=`echo $ARG_OTHER | awk '{print $1}'`
		ARG_SIZE=`echo $ARG_OTHER | awk '{print $2}'`
		if [ "$ARG_FILE" = "" ] || [ ! -f $ARG_FILE ] ; then
			echo "error: file $ARG_FILE not found!"
			exit 1
		fi

		if [ "$ARG_SIZE" = "" ] ; then
			echo "parameter error for SIZE"
			exit 1
		fi
		;;
	merge2)
		ARG_FUNC=merge2
		shift 1
		while [ $# -gt 0 ]; do
			case $1 in
			--help)
				usage_merge2
				shift 1
				;;
			-s)
				shift 1
				ARG_SIZE=$1
				shift 1
				;;
			--size=*)
				ARG_SIZE=`echo $1 | awk -F '=' '{print $2}'`
				shift 1
				;;
			--output_offset=*)
				ARG_SEEK=`echo $1 | awk -F '=' '{print $2}'`
				shift 1
				;;
			--input_offset=*)
				ARG_SKIP=`echo $1 | awk -F '=' '{print $2}'`
				shift 1
				;;
			*)
				ARG_OTHER="$ARG_OTHER $1"
				shift 1
				;;
			esac
		done
		ARG_FILE=`echo $ARG_OTHER | awk '{print $1}'`
		ARG_SUB_FILE=`echo $ARG_OTHER | awk '{print $2}'`
		if [ "$ARG_FILE" = "" ] || [ ! -f $ARG_FILE ] ; then
			echo "error: file $ARG_FILE not found!"
			exit 1
		fi

		if [ "$ARG_SUB_FILE" = "" ] || [ ! -f $ARG_SUB_FILE ] ; then
			echo "error: file $ARG_SUB_FILE not found!"
			exit 1
		fi
		;;
	mergen)
		ARG_FUNC=mergen
		shift 1
		ARG_FILE=output.bin
		while [ $# -gt 0 ]; do
			case $1 in
			--help)
				usage_mergen
				shift 1
				;;
			--output=*)
				ARG_FILE=`echo $1 | awk -F '=' '{print $2}'`
				shift 1
				;;
			*)
				ARG_SUB_FILE="$ARG_SUB_FILE $1"
				shift 1
				;;
			esac
		done
		;;
	extract)
		ARG_FUNC=extract
		shift 1
		ARG_OUTPUT_FILE=output.bin
		while [ $# -gt 0 ]; do
			case $1 in
			--help)
				usage_extract
				shift 1
				;;
			--output=*)
				ARG_OUTPUT_FILE=`echo $1 | awk -F '=' '{print $2}'`
				shift 1
				;;
			--input_offset=*)
				ARG_SKIP=`echo $1 | awk -F '=' '{print $2}'`
				shift 1
				;;
			*)
				ARG_OTHER="$ARG_OTHER $1"
				shift 1
				;;
			esac
		done
		ARG_FILE=`echo $ARG_OTHER | awk '{print $1}'`
		ARG_SIZE=`echo $ARG_OTHER | awk '{print $2}'`
		if [ "$ARG_FILE" = "" ] || [ ! -f $ARG_FILE ] ; then
			echo "error: file $ARG_FILE not found!"
			exit 1
		fi
		;;
	*)
		usage
		shift 1
		;;
	esac
done

func_pad()
{
	local local_data=`printf %03o $ARG_DATA`
	local local_file_size=`wc -c $ARG_FILE | awk '$2="";1'`
	local local_pad_size=""

	echo "local_data is $local_data"

	((local_pad_size = ARG_SIZE - local_file_size))

	echo "backup $ARG_FILE to $ARG_FILE.origin"
	cp $ARG_FILE $ARG_FILE.origin

	tr '\000' "\\${local_data}" < /dev/zero | dd of=$ARG_FILE bs=$local_pad_size count=1 conv=notrunc,fdatasync oflag=append iflag=fullblock
	echo "pad finished!"
}

func_align()
{
	local local_data=`printf %03o $ARG_DATA`
	local local_file_size=`wc -c $ARG_FILE | awk '$2="";1'`
	local local_pad_size=""
	local local_mode_size=""

	((local_mode_size = local_file_size%ARG_SIZE))
	((local_pad_size = ARG_SIZE - local_mode_size))

	echo "backup $ARG_FILE to $ARG_FILE.origin"
	cp $ARG_FILE $ARG_FILE.origin

	tr '\000' "\\${local_data}" < /dev/zero | dd of=$ARG_FILE bs=$local_pad_size count=1 conv=notrunc,fdatasync oflag=append iflag=fullblock
	echo "align finished!"
}

func_merge2()
{
	local local_seek=`wc -c $ARG_FILE | awk '$2="";1'`
	local local_size=`wc -c $ARG_SUB_FILE | awk '$2="";1'`
	local local_skip=0

	if [ "$ARG_SEEK" != "" ] ; then
		local_seek=`printf %d $ARG_SEEK`
	fi
	if [ "$ARG_SIZE" != "" ] ; then
		local_size=`printf %d $ARG_SIZE`
	fi
	if [ "$ARG_SKIP" != "" ] ; then
		local_skip=`printf %d $ARG_SKIP`
	fi

	echo "backup $ARG_FILE to $ARG_FILE.origin"
	cp $ARG_FILE $ARG_FILE.origin

	dd if=$ARG_SUB_FILE of=$ARG_FILE bs=1 seek=$local_seek skip=$local_skip count=$local_size conv=notrunc iflag=fullblock
	echo "merge2 finished!"
}

func_mergen()
{
	rm -rf $ARG_FILE
	for FILE in $ARG_SUB_FILE; do
		dd if=$FILE of=$ARG_FILE conv=notrunc,fdatasync oflag=append
	done
	echo "mergen finished!"
}

func_extract()
{
	local local_file_size=`wc -c $ARG_FILE | awk '$2="";1'`
	local local_size=0
	local local_skip=0

	if [ "$ARG_SKIP" != "" ] ; then
		local_skip=`printf %d $ARG_SKIP`
	fi

	((local_size = local_file_size - local_skip))

	if [ "$ARG_SIZE" != "" ] ; then
		local_size=`printf %d $ARG_SIZE`
	fi

	dd if=$ARG_FILE of=$ARG_OUTPUT_FILE bs=1 skip=$local_skip count=$local_size conv=notrunc iflag=fullblock
	echo "extract finished!"
}

case $ARG_FUNC in
pad)
	func_pad
	exit 0
	;;
align)
	func_align
	exit 0
	;;
merge2)
	func_merge2
	exit 0
	;;
mergen)
	func_mergen
	exit 0
	;;
extract)
	func_extract
	exit 0
	;;
*)
	echo "function $ARG_FUNC not support"
	usage
	exit 0
	;;
esac

