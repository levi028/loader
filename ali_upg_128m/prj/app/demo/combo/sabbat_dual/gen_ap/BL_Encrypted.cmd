[PARAMETER]
chip_name=3821
boot_total_area=boot_total_area.abs
encrypt_format=CLEAR_KEY
# BL universal key, customer can change this key
encrypt_key=0x1d, 0xd7, 0xbb, 0xc5, 0x53, 0x26, 0x0c, 0xdc, 0x54, 0x05, 0x5d, 0xc5, 0x00, 0xa8, 0x8b, 0xfa,
[CMD]
## bl encrypted with other partition, such as boot parameters area etc
bl_enc_wh
