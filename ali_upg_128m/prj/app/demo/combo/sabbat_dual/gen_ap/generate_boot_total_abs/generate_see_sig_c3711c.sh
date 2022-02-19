#!/bin/bash
# Illustration:
#This script is used to generate the boota_total_area.abs

# define path for tool and scripts
ROOT=$(pwd)

OSN=$(uname -s)
OS_IS_LINUX=$(echo ${OSN} | grep -i linux)

#echo "${OSN}"
#echo "${OS_IS_LINUX}"

if [ "${OS_IS_LINUX}"x = "Linux"x ]; then
	OSPX=
else 
	OSPX=".exe"
fi

if [ "${OS_IS_LINUX}"x = "Linux"x ]; then
	PATHPX="tool_linux"
else 
	PATHPX="tool"
fi

ALICRYPTOCLIENT=$ROOT/${PATHPX}/cryptoclient${OSPX}
ALICRYPTO=$ROOT/${PATHPX}/ALiCrypto${OSPX}
BLTOOL=$ROOT/${PATHPX}/bl_tool${OSPX}
TEMPDIR=$ROOT/../temporary
MKIMAGE=$ROOT/${PATHPX}/mkimage${OSPX}
curdir="$(dirname $0)"

export ALICRYPTOCLIENT
export ALICRYPTO
export BLTOOL
export TEMPDIR
export MKIMAGE
export ROOT

rm -rf $TEMPDIR
mkdir -p $TEMPDIR

cp -rf $curdir/*  $TEMPDIR/
cd $TEMPDIR

#do see sign
subscripts="$(ls $TEMPDIR/common/*.sh)"
for sub in $subscripts; do
	. $sub
done

mapping=$TEMPDIR/auto-generate/mapping.config
[ ! -f $mapping ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

. $mapping

output=$TEMPDIR/common

echo "do see"

#see_orig=$output.orig
#see_output=$output

input=./common/see_bin.abs
ctrdata=$TEMPDIR/ctrdata/see_ctrdata.bin
#output=$see_orig
do_tds_ca see lzma ABL_AES_OTP_KEY_10 ABL_RSA_PK_SEESW $input $output $ctrdata c3711c ACS

#cat $see_orig > $see_output
cp -vaf $TEMPDIR/common/see_bin.abs.ok  $ROOT/../../app_see.bin
echo "gen see all done!"




