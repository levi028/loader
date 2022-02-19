#!/bin/sh
#IN_FILE=bl_out.abs
IN_FILE=$1
IN_FILE_SIZE=`du -b $IN_FILE | awk '{print $1}'`
echo "bl_out size:" $IN_FILE_SIZE

OUT_FILE0=ext_rsa_pubkey_area.bin
OUT_FILE1=ext_rsa_pubkey_area_sig.bin
OUT_FILE2=boot_params_area.bin
OUT_FILE3=boot_params_area_sig.bin
OUT_FILE4=boot_aux_code_area.bin
OUT_FILE5=boot_aux_code_area_sig.bin
OUT_FILE6=boot_unchecked_area.bin
OUT_FILE7=boot_code_area.bin
OUT_FILE8=boot_code_area_sig.bin

echo "0.extract ext_rsa_public_key data..."
./util.sh extract --output=$OUT_FILE0 --input_offset=0 $IN_FILE 336
echo "1.extract ext_rsa_public_key_sig data..."
./util.sh extract --output=$OUT_FILE1 --input_offset=336 $IN_FILE 256
echo "2.extract boot_params_area data..."
./util.sh extract --output=$OUT_FILE2 --input_offset=592 $IN_FILE 1200
echo "3.extract boot_params_area sig data..."
./util.sh extract --output=$OUT_FILE3 --input_offset=1792 $IN_FILE 256
echo "4.extract boot_aux_code_area data..."
./util.sh extract --output=$OUT_FILE4 --input_offset=2048 $IN_FILE 3840
echo "5.extract boot_aux_code_area_sig data..."
./util.sh extract --output=$OUT_FILE5 --input_offset=5888 $IN_FILE 256
echo "6.extract boot_unchecked_area data..."
./util.sh extract --output=$OUT_FILE6 --input_offset=6144 $IN_FILE 8192
echo "7.extract boot_code_area data..."
./util.sh extract --output=$OUT_FILE7 --input_offset=14336 $IN_FILE $(($IN_FILE_SIZE-14336-256))
echo "8.extract boot_code_area_sig data..."
./util.sh extract --output=$OUT_FILE8 --input_offset=$(($IN_FILE_SIZE-256)) $IN_FILE 256

