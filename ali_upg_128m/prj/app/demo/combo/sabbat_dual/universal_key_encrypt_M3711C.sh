#!/bin/bash

OSN=$(uname -s)
OS_IS_LINUX=$(echo ${OSN} | grep -i linux)

echo "${OS_IS_LINUX}"

KEY_DIR=$1
if [ -z ${KEY_DIR} ];then
	echo "cannot find the corrent dir"
	exit 1
fi

if [ "${OS_IS_LINUX}"x = "Linux"x ]; then
	tools_dir="tools/tools_linux"	
	uke_tools="universal_key_encrypt"
else 
	tools_dir="tools"	
	uke_tools="universal_key_encrypt.exe"
fi

echo "current dir:${KEY_DIR}"
cp -f ./${KEY_DIR}/decrypt_key.bin ./
cp -f ./${KEY_DIR}/BL_Encrypted.bin ./
cp -f ./${KEY_DIR}/see_sw_uk.bin ./
../../../../${tools_dir}/${uke_tools} BL_Encrypted.bin BL_Encrypted_enc.bin
../../../../${tools_dir}/${uke_tools} decrypt_key.bin decrypt_key_enc.bin
../../../../${tools_dir}/${uke_tools} see_sw_uk.bin see_sw_uk_enc.bin
rm decrypt_key.bin
rm BL_Encrypted.bin
rm see_sw_uk.bin
