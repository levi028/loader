
cd smi_ap

cp -f ../app_to_be_signed.bin app_to_be_signed.bin
cp -f ../app_see_to_be_signed.bin app_see_to_be_signed.bin

../../../../../tools/bl_tool.exe FW_OTA_Sign.cmd
../../../../../tools/bl_tool.exe FW_OTA_Enc.cmd

cp -f app_to_be_signed.bin_s_e ../app.bin
cp -f app_see_to_be_signed.bin_s_e ../app_see.bin

cd ..

../../../../tools/fidmerger_ca.exe blocks_script_7z_ota_s3505_dual.txt

cp product_sabbat_dual_ota.abs ../../../../../obj/product_sabbat_dual_ota.abs