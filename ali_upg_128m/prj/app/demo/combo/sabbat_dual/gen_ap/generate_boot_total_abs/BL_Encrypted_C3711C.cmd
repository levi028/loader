[PARAMETER]
generation=GA5
boot_total_area=boot_total_area_encrypt.abs
encrypt_format=CLEAR_KEY
# BL universal key, customer can change this key
encrypt_key=./keys/flashprotectionkey.bin
[CMD]
## bl encrypted with other partition, such as boot parameters area etc
bl_enc_wh
