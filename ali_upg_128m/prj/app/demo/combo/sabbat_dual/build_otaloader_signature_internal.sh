## for ALi signature
len=`./ftell.exe app_to_be_signed.bin`
./create_signature.exe key.bin $len app_to_be_signed.bin app_signature.bin app_encrypt.bin parameter.bin 0
len=`./ftell.exe app_encrypt.bin`
./fmerge.exe app_encrypt.bin app_signature.bin $len app_signed.bin

#len=`./ftell.exe app_see_to_be_signed.bin`
#./create_signature.exe key.bin $len app_see_to_be_signed.bin app_see_signature.bin app_see_encrypt.bin parameter.bin 0
#len=`./ftell.exe app_see_encrypt.bin`
#./fmerge.exe app_see_encrypt.bin app_see_signature.bin $len app_see_signed.bin
cp app_see_to_be_signed.bin ./bc_ap
cd ./bc_ap
../../../../../tools/bl_tool.exe App_SEE_Sign_by_SeeRootPK.cmd
cp app_see_to_be_signed.bin_s ../app_see_signed.bin
cd ..

cp -f app_signed.bin app.bin
cp -f app_see_signed.bin app_see.bin

cp -f app.bin ota_app.bin
cp -f app_see.bin ota_app_see.bin

../../../../tools/fidmerger_ca.exe blocks_script_7z_ota_s3821_dual.txt

cp product_sabbat_dual_ota.abs ../../../../../obj/product_sabbat_dual_ota.abs