#!/bin/bash

#SEE_OUT=see_sabbat_s35x
SEE_OUT=$1
IS64M=$2
DOWNLOAD_SCRIPT=s3505_sabat
if [ ! -f ${SEE_OUT}.abs ] ; then
	echo "cannot find ${SEE_OUT}.abs"	
	exit 1
fi

cp see_sabbat_s3505.abs see_sabbat_s35x.abs
../../../../../tools/tools_linux/as_tool get_see_abs_sig.cmd
cp see_sabbat_s35x.abs.sig see_sabbat_s3505.sig

sed '/0xa3d001F0/'d ${DOWNLOAD_SCRIPT}.ini >${DOWNLOAD_SCRIPT}_tmp.ini
sed '/0xa70001F0/'d ${DOWNLOAD_SCRIPT}_tmp.ini >${DOWNLOAD_SCRIPT}_tmp2.ini
mv ${DOWNLOAD_SCRIPT}_tmp2.ini ${DOWNLOAD_SCRIPT}_tmp.ini

if [ ${IS64M}x = "64M"x ] ; then	
	sed -i 's/0xa7000200/0xa3d00200/g' ${DOWNLOAD_SCRIPT}_tmp.ini
	echo -n "AutoRun0=wm 0xa3d001F0 " >> ${DOWNLOAD_SCRIPT}_tmp.ini 
else	
	sed -i 's/0xa3d00200/0xa7000200/g' ${DOWNLOAD_SCRIPT}_tmp.ini
	echo -n "AutoRun0=wm 0xa70001F0 " >> ${DOWNLOAD_SCRIPT}_tmp.ini 
fi

du -b see_sabbat_s3505.abs | awk '{printf "0x%.8x\n",$1}' >>${DOWNLOAD_SCRIPT}_tmp.ini
rm -f ${DOWNLOAD_SCRIPT}.ini
mv -f ${DOWNLOAD_SCRIPT}_tmp.ini ${DOWNLOAD_SCRIPT}.ini