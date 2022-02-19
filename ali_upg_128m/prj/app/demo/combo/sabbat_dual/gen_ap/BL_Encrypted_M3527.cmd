[PARAMETER]
generation=GA3
chip_name=3505
boot_total_area=boot_total_area.abs
aes_mode=AES_CBC
encrypt_format=CLEAR_KEY
# BL universal key, customer can change this key
encrypt_key=BL_Encrypted_M3527.bin
[CMD]
## bl encrypted with other partition, such as boot parameters area etc
bl_enc_wh
