#!/bin/bash

export _BUILD_OTA_E_=1

make path
make config
make new
make ddk
make product_7z

ret=$?
if [ $ret != 0 ]; then
	echo "$0 with error[$ret]"		
	exit 1
fi

cp product_s3602_ota.abs product_s3602_ota_3602.abs
