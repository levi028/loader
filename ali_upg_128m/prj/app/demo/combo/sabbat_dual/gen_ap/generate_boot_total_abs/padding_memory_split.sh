#!/bin/bash

. $BR2_CONFIG > /dev/null 2>&1
. $BR2_UPG_CONFIG > /dev/null 2>&1

TEMPDIR=../temporary

bootcfg=$1
dramfile=$2

output=dramTempFile.txt

echo "bootcfg is $bootcfg"
echo "dramfile is $dramfile"

rm -f ${output}

# delete deaddead of memory init files
sed -e '/0xdeaddead/d'  ${dramfile}  > ${output}

subscripts="$(ls $TEMPDIR/common/*.sh)"
for sub in $subscripts; do
	. $sub
done

#register format is for auxcode protocol
priv_area_start_reg=0x0f041010
priv_area_end_reg=0x0f041014
share_area_start_reg=0x0f041018


function fgetword()
{
   local bytes=`hexdump -x -s $2 -n $3 $1 | (read b1 b2 b3 b4; echo "$b4$b3$b2$b1" | tr "[] "  "[]" )`
   bytes=`expr substr $bytes 1 8`
   ret=`echo $((16#${bytes}))`
   return 0
}

#get private start address from boot_cfg
fgetword ${bootcfg} 48 4
priv_area_start_val=$(($ret&16#0FFFFFFF))
#get private end address from boot_cfg
fgetword ${bootcfg} 52 4
((priv_area_end_val=$priv_area_start_val+$ret))    
#get share memory start address from boot_cfg
fgetword ${bootcfg} 56 4
share_area_start_val=$(($ret&16#0FFFFFFF))

priv_area_start_val=`printf "0x%08x" ${priv_area_start_val}` \
priv_area_end_val=`printf "0x%08x" ${priv_area_end_val}` \
share_area_start_val=`printf "0x%08x" ${share_area_start_val}`

echo "${priv_area_start_reg}  ${priv_area_start_val}" >> ${output}
echo "${priv_area_end_reg}  ${priv_area_end_val}" >> ${output}
echo "${share_area_start_reg}  ${share_area_start_val}" >> ${output}
echo "0xdeaddead  0xdeaddead" >> ${output}

echo "priv_area_start_reg is $priv_area_start_reg"
echo "priv_area_start_val is $priv_area_start_val"
echo "priv_area_end_reg is $priv_area_end_reg"
echo "priv_area_end_val is $priv_area_end_val"
echo "share_area_start_reg is $share_area_start_reg"
echo "share_area_start_val is $share_area_start_val"

mv -f ${output} ${dramfile}

exit 0
