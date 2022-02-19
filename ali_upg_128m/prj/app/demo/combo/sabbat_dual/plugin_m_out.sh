#!/bin/bash

SHL_ROOT_DIR=..
#/../../../..
#PLUGINDDK_DIR=${SHL_ROOT_DIR}/ddk/plugin
#PLUGIN_DIR=${SHL_ROOT_DIR}/ddkt2/plugin
OUT_FILE=${1}


function merge_plugin_dec_into_abs()
{

	local TEXT_END=`sde-readelf.exe -s ${SHL_ROOT_DIR}/obj/${OUT_FILE}.out | grep  _code_text_end_addr |awk '{print $2}'`
	#echo TEXT_END=${TEXT_END}

	local text_off_in_OUT=`sde-readelf.exe -S ${SHL_ROOT_DIR}/obj/${OUT_FILE}.out | grep  .rodata |awk '{print $6}'`
	echo text_off_in_OUT=${text_off_in_OUT}

	local OUT_MERGER_ADDR=$(awk 'BEGIN{printf("%#d",'0x$text_off_in_OUT'-'0x1000')}')
	echo OUT_MERGER_ADDR=${OUT_MERGER_ADDR}

	local TEXT_BASE=`sde-readelf.exe -s ${SHL_ROOT_DIR}/obj/${OUT_FILE}.out | grep  __RAM_BASE |awk '{print $2}'`
	#echo TEXT_BASE=${TEXT_BASE}
	local DATA_SIZE=`stat -c %s ./plugin_dec.abs`
	#echo DATA_SIZE=${DATA_SIZE}
	local MERGER_ADDR=$(awk 'BEGIN{printf("%#d",'0x$TEXT_END'-'0x$TEXT_BASE'-'0x1000')}')
	#echo MERGER_ADDR=${MERGER_ADDR}

	#echo ${OUT_FILE}
	if [ -f ${OUT_FILE}.out ]; then
	echo "abs mergering..."
	${SHL_ROOT_DIR}/prj/tools/bmerger.exe ${SHL_ROOT_DIR}/obj/${OUT_FILE}.abs ./plugin_dec.abs ${MERGER_ADDR} ${DATA_SIZE}
	echo "out Mergering!!!"
	echo ${OUT_FILE}.out
	echo ${OUT_MERGER_ADDR}
	echo ${DATA_SIZE}
	#cp ${SHL_ROOT_DIR}/obj/${OUT_FILE}.out ${SHL_ROOT_DIR}/obj/${OUT_FILE}-bak.out
	${SHL_ROOT_DIR}/prj/tools/bmerger.exe ${SHL_ROOT_DIR}/obj/${OUT_FILE}.out ./plugin_dec.abs ${OUT_MERGER_ADDR} ${DATA_SIZE}
	#echo $?
	else
	echo "no ${OUT_FILE}!!"
	fi
}

	merge_plugin_dec_into_abs
