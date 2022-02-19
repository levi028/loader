#!/bin/bash

SHL_ROOT_DIR=..
#/../../../..
#PLUGINDDK_DIR=${SHL_ROOT_DIR}/ddk/plugin
#PLUGIN_DIR=${SHL_ROOT_DIR}/ddkt2/plugin
OUT_FILE=${1}
GCC_READELF=/opt/mips-sde-elf-4.7.3/bin/mips-sde-elf-readelf

function merge_plugin_dec_into_abs()
{

	local TEXT_END=`${GCC_READELF} -s ${SHL_ROOT_DIR}/obj/${OUT_FILE}.out | grep  _code_text_end_addr |awk '{print $2}'`
	#echo TEXT_END=${TEXT_END}

	local text_off_in_OUT=`${GCC_READELF} -S ${SHL_ROOT_DIR}/obj/${OUT_FILE}.out | grep  .rodata |awk '{print $6}'`
	echo text_off_in_OUT=${text_off_in_OUT}
    TEXT_TEMP=1000
	local OUT_MERGER_ADDR=$(awk 'BEGIN{printf("%#d",'$((16#${text_off_in_OUT}-16#${TEXT_TEMP}))')}')
	echo OUT_MERGER_ADDR=${OUT_MERGER_ADDR}

	local TEXT_BASE=`${GCC_READELF} -s ${SHL_ROOT_DIR}/obj/${OUT_FILE}.out | grep  __RAM_BASE |awk '{print $2}'`
	echo TEXT_BASE=${TEXT_BASE}
	local DATA_SIZE=`stat -c %s ./plugin_dec.abs`
	echo DATA_SIZE=${DATA_SIZE}
	local MERGER_ADDR=$(awk 'BEGIN{printf("%#d",'$((16#${TEXT_END}-16#${TEXT_BASE}-16#${TEXT_TEMP}))')}')
	echo MERGER_ADDR=${MERGER_ADDR}

	#echo ${OUT_FILE}
	if [ -f ${OUT_FILE} ]; then
	echo "abs mergering..."
	#${SHL_ROOT_DIR}/prj/tools/bmerger.exe ${SHL_ROOT_DIR}/obj/${OUT_FILE} ./plugin_dec.abs ${MERGER_ADDR} ${DATA_SIZE}
	#address=`printf %d ${MERGER_ADDR}`
	dd if=plugin_dec.abs of=${SHL_ROOT_DIR}/obj/${OUT_FILE} bs=1 seek=${MERGER_ADDR} conv=notrunc
	#${SHL_ROOT_DIR}/prj/tools/tools_linux/bmerger ${SHL_ROOT_DIR}/obj/${OUT_FILE} ./plugin_dec.abs ${MERGER_ADDR} ${DATA_SIZE}
	echo "out Mergering!!!"
	echo ${MERGER_ADDR}
	#${SHL_ROOT_DIR}/prj/tools/bmerger.exe ${SHL_ROOT_DIR}/obj/${OUT_FILE}.out ./plugin_dec.abs ${OUT_MERGER_ADDR} ${DATA_SIZE}
	#echo $?
	else
	echo "no ${OUT_FILE}!!"
	fi
}

	merge_plugin_dec_into_abs
