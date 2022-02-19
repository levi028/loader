[PARAMETER]
generation=GA4
chip_name=3702
boot_total_area=boot_total_area_encrypt.abs
aes_mode=AES_CBC
encrypt_format=CLEAR_KEY
# BL universal key, customer can change this key
encrypt_key=./keys/BL_Encrypted.bin
[CMD]
## bl encrypted with other partition, such as boot parameters area etc
bl_enc_wh