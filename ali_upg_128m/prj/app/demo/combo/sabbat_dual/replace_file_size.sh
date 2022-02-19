#!/bin/bash

see_bl_file=$1
see_sw_file=$2
orginal_script_file=$3


if [ ! -e "${see_bl_file}" ]; then
	echo "see_bl_file eror"
	exit 1
fi

if [ ! -e "${see_sw_file}" ]; then
	echo "see_sw_file eror"
	exit 1
fi

if [ ! -e "${orginal_script_file}" ]; then
	echo "orginal_script_file eror"
	exit 1
fi


temp_script_file=temp_${orginal_script_file}
output=replace_${orginal_script_file}

echo "see_bl_file is ${see_bl_file}"
echo "see_bl_file is ${see_sw_file}"
echo "orginal_script_file is ${orginal_script_file}"
echo "output is ${output}"

see_bl_size=`wc -c ${see_bl_file} | awk '{print $1}'`
see_sw_size=`wc -c ${see_sw_file} | awk '{print $1}'`

see_bl_size=`printf "0x%08x" ${see_bl_size}` \
see_sw_size=`printf "0x%08x" ${see_sw_size}` \

rm -f ${output}
rm -f ${temp_script_file}
 
cp -f ${orginal_script_file} ${temp_script_file}
sed -i 's/SEE_BL_SIZE/'"$see_bl_size"'/g' ${temp_script_file}
sed -i 's/SEE_SW_SIZE/'"$see_sw_size"'/g' ${temp_script_file}
cp -f ${temp_script_file} ${output}
rm -f ${temp_script_file}

