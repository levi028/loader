#!/bin/bash
# Illustration:
#This script is used to generate the boota_total_area.abs

# define path for tool and scripts
ROOT=$(pwd)
ALICRYPTOCLIENT=$ROOT/tool_linux/cryptoclient
ALICRYPTO=$ROOT/tool/ALiCrypto
BLTOOL=$ROOT/tool/bl_tool
TEMPDIR=$ROOT/../temporary
TO_BE_SIGN=$ROOT/to_be_sign
MKIMAGE=$ROOT/tool_linux/mkimage
curdir="$(dirname $0)"

export ALICRYPTOCLIENT
export ALICRYPTO
export BLTOOL
export TEMPDIR
export MKIMAGE
export ROOT

#mapping=$TEMPDIR/auto-generate/mapping.config
#[ ! -f $mapping ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
#. $mapping
#rm -rf $TEMPDIR
#mkdir -p $TEMPDIR

cp -rf $curdir/*  $TEMPDIR/

#generate the boot_total_area
#$BLTOOL BL_MERGE.cmd
#cd $TO_BE_SIGN
#rm -rf ./boot_*.bin
#rm -rf ./ext_rsa*.bin
#cp -rf $ROOT/bl.in $ROOT/to_be_sign/boot_total_area_sign.abs
#./bl_extract.sh boot_total_area_sign.abs
#./bl_merge.sh boot_total_area.abs
#cp -rf $TO_BE_SIGN/boot_total_area.abs $ROOT/bl.in
#cd ..
#cp -rf $ROOT/../decrypt_key.bin $ROOT/decrypt_key.bin
#cp -rf $ROOT/../sw_uk.bin $ROOT/sw_uk.bin
#insert uboot universal key to boot_code_area
#$ALICRYPTO Insert_Decrypt_UK.cmd
#insert system SW universal key to boot_code_area
#$ALICRYPTO Insert_Decrypt_UK2.cmd
#cp -rf $ROOT/boot_total_area.abs $TEMPDIR/boot_total_area.abs
#cp -rf $ROOT/boot_total_area.abs $TEMPDIR/boot_total_area_one.abs
#echo "insert UK done!"
#cp -rf $ROOT/boot_total_area.abs $ROOT/boot_total_area_clear.abs
#cp -rf $ROOT/boot_total_area.abs $ROOT/boot_total_area_encrypt.abs
#$BLTOOL BL_Encrypted_C3702.cmd
#cp -rf $ROOT/boot_total_area_encrypt.abs $TEMPDIR/boot_total_area.abs
#echo "encrypt BL done!"
cd $TEMPDIR
#sign and enc tee_aux & tee_param
#message "sign and enc tee_aux & tee_param : ====>"
$TEMPDIR/teeaux/gen_memory_space.sh $TEMPDIR/teeaux_memspace.bin
$TEMPDIR/teeaux/gen_trust_chain_addr.sh $TEMPDIR/teeaux_trustchain_addr.bin
#gen tee_param_area abs
cat $TEMPDIR/teeaux_trustchain_addr.bin \
		$TEMPDIR/teeaux_memspace.bin \
		$TEMPDIR/teeaux/tee_param_area.abs \
		> $TEMPDIR/tee_param_area.abs
echo "gen tee_param_area done!"
#gen tee_auxcode.abs.sigf.ecf.pkcs
$TEMPDIR/teeaux/teeaux_sign.sh \
		$TEMPDIR/tee_auxcode.abs \
		$TEMPDIR/tee_param_area.abs \
		$TEMPDIR/ctrdata/teeaux_ctrdata.bin \
		$TEMPDIR/tee_auxcode.abs.sigf.ecf.pkcs
echo "gen teeaux sign done!"
#insert tee_auxcode to boot_code_area
cp -f $TEMPDIR/tee_auxcode.abs.sigf.ecf.pkcs $TEMPDIR/../tee_total_area.abs
cat $TEMPDIR/boot_total_area.abs $TEMPDIR/tee_auxcode.abs.sigf.ecf.pkcs > $TEMPDIR/boot_total_area_ok.abs
echo "gen boot_total_area done!"

#do see bl and see
$ROOT/gen_seebl_see.sh
echo "gen all done!"




