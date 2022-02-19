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
echo "gen tee_total_area done!"

subscripts="$(ls $TEMPDIR/common/*.sh)"
for sub in $subscripts; do
	. $sub
done

mapping=$TEMPDIR/auto-generate/mapping.config
[ ! -f $mapping ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

. $mapping

output=$TEMPDIR/common

input=./common/see_bl.abs
ctrdata=$TEMPDIR/ctrdata/seebl_ctrdata.bin
get_rsa_pk see_sw_rsa $TEMPDIR/see_sw_key_pair.der.pub c3702 ACS
append_pk $input $TEMPDIR/see_sw_key_pair.der.pub
do_ubo_c2000a seebl none ABL_RSA_PK_TEESW ABL_AES_OTP_KEY_10 ABL_RSA_PK_SEEROOT $input $output $ctrdata c3702 ACS

cp -vf $TEMPDIR/common/see_bl.abs.ok $ROOT/../see_bl.ubo
echo "gen tee seebl all done!"




