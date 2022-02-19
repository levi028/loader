#!/bin/bash

curdir="$(dirname $0)"

subscripts="$(ls $TEMPDIR/common/*.sh)"
for sub in $subscripts; do
	. $sub
done

mapping=$TEMPDIR/auto-generate/mapping.config
[ ! -f $mapping ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

. $mapping

output=$TEMPDIR/common

#cp -vf $ROOT/see_bl.abs  $ROOT/common/see_bl.abs
#cp -vf $ROOT/see_bin.abs  $ROOT/common/see_bin.abs
#input=$TEMPDIR/common/see_bl.abs
input=./common/see_bl.abs
ctrdata=$TEMPDIR/ctrdata/seebl_ctrdata.bin
get_rsa_pk see_sw_rsa $TEMPDIR/see_sw_key_pair.der.pub c3702 ACS
append_pk $input $TEMPDIR/see_sw_key_pair.der.pub
do_ubo_c2000a seebl none ABL_RSA_PK_TEESW ABL_AES_OTP_KEY_10 ABL_RSA_PK_SEEROOT $input $output $ctrdata c3702 ACS
#		cp -f $TEMPDIR/$output $TEMPDIR/seebl_22222222222222222.bin
#		echo "vvvvvvvvvvvvvvvvvv"
echo "$output"
#		[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

echo "do see"

#see_orig=$output.orig
#see_output=$output

input=./common/see_bin.abs
ctrdata=$TEMPDIR/ctrdata/see_ctrdata.bin
#output=$see_orig
do_ubo_c2000a see lzma ABL_RSA_PK_TEESW ABL_AES_OTP_KEY_10 ABL_RSA_PK_SEESW $input $output $ctrdata c3702 ACS

#cat $see_orig > $see_output
cp -vf $TEMPDIR/common/see_bin.abs.ok  $ROOT/../see.ubo
cp -vf $TEMPDIR/common/see_bl.abs.ok $ROOT/../see_bl.ubo

	
