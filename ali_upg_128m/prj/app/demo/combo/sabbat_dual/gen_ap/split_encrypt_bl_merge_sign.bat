cp product_sabbat_dual.abs product_sabbat_dual_temp.abs
lo_tool_internal.exe -M to_be_signed_loader_ca_vendor-signature.abs product_sabbat_dual.abs to_be_signed_loader_ca_vendor.abs
Encrypt.exe -E AES_CBC orig_bl_key.bin product_sabbat_dual.abs M3281_public_key_by_ca_vendor.abs

cp product_sabbat_dual_temp.abs product_sabbat_dual.abs
lo_tool_internal.exe -S product_sabbat_dual.abs sys_sw_pair.key
redemo -S to_be_signed_loader.abs signed_loader_with_signature_by_ali.abs sys_sw_pair.key ejtag.bin
lo_tool_internal.exe -M signed_loader_with_signature_by_ali.abs product_sabbat_dual.abs to_be_signed_loader.abs
Encrypt.exe -E AES_CBC orig_bl_key.bin product_sabbat_dual.abs M3281_public_key_by_ali.abs

cp -f M3281_public_key_by_ca_vendor.abs ../product_sabbat_dual_ca_vendor.abs
cp -f M3281_public_key_by_ali.abs ../product_sabbat_dual.abs


