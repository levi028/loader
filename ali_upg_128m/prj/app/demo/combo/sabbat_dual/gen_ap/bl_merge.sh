#!/bin/sh
OUT_FILE0=ext_rsa_pubkey_area.bin
OUT_FILE1=ext_rsa_pubkey_area_sig.bin
OUT_FILE2=boot_params_area.bin
OUT_FILE3=boot_params_area_sig.bin
OUT_FILE4=boot_aux_code_area.bin
OUT_FILE5=boot_aux_code_area_sig.bin
OUT_FILE6=boot_unchecked_area.bin
OUT_FILE7=boot_code_area.bin
OUT_FILE8=boot_code_area_sig.bin

OUT_TMP=$1
rm -rf $OUT_TMP
touch $OUT_TMP

echo "0.merge ext_rsa_pubkey_area data..."
./util.sh merge2 $OUT_TMP $OUT_FILE0

echo "0.merge ext_rsa_pubkey_area_sig data..."
./util.sh merge2 $OUT_TMP $OUT_FILE1

echo "1.extract boot_params_area sig data..."
./util.sh merge2 $OUT_TMP $OUT_FILE2

echo "2.extract boot_params_area_sig data..."
./util.sh merge2 $OUT_TMP $OUT_FILE3

echo "3.extract boot_aux_code_area data..."
./util.sh merge2 $OUT_TMP $OUT_FILE4

echo "4.extract boot_aux_code_area_sig data..."
./util.sh merge2 $OUT_TMP $OUT_FILE5

echo "5.extract boot_unchecked_area data..."
./util.sh merge2 $OUT_TMP $OUT_FILE6

echo "6.extract boot_code_area data..."
./util.sh merge2 $OUT_TMP $OUT_FILE7

echo "7.extract boot_code_area_sig data..."
./util.sh merge2 $OUT_TMP $OUT_FILE8

