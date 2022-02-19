cp product_sabbat_dual.abs product_sabbat_dual_temp.abs
lo_tool.exe -D product_sabbat_dual.abs ./2nd_level_public_key/public_key.der ./2nd_level_public_key/public_key_sig.bin X509 KEY_IN_BL
lo_tool_internal.exe -M to_be_signed_loader_ca_vendor-signature.abs product_sabbat_dual.abs to_be_signed_loader_ca_vendor.abs
Encrypt.exe -E AES_CBC orig_bl_key.bin product_sabbat_dual.abs M3515_public_key_by_ca_vendor.abs

cp product_sabbat_dual_temp.abs product_sabbat_dual.abs
lo_tool.exe -D product_sabbat_dual.abs ./2nd_level_public_key/public_key.der ./2nd_level_public_key/public_key_sig.bin X509 KEY_IN_BL 
lo_tool.exe -E product_sabbat_dual.abs see_bloader3_3503_s.abs
lo_tool.exe -S3 product_sabbat_dual.abs sys_sw_pair.key
redemo -S to_be_signed_loader.abs signed_loader_with_signature_by_ali.abs ./2nd_level_public_key/ali_2nd_level_pair.key
lo_tool.exe -M signed_loader_with_signature_by_ali.abs product_sabbat_dual.abs to_be_signed_loader.abs
cp -f product_sabbat_dual.abs lo_tool_M_test
cp -f product_sabbat_dual.abs clear.abs
Encrypt.exe -E AES_CBC orig_bl_key.bin product_sabbat_dual.abs M3515_public_key_by_ali.abs
cp -f M3515_public_key_by_ali.abs product_sabbat_dual.abs 

cp -f M3515_public_key_by_ca_vendor.abs ../product_sabbat_dual_ca_vendor.abs
cp -f M3515_public_key_by_ali.abs ../product_sabbat_dual.abs


