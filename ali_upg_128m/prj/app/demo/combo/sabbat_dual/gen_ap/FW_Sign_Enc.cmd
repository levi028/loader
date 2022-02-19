[PARAMETER]
# input file name
swcode=product_sabbat_dual.abs
#RSASSA_PKCS or RSASSA_PSS
sig_format=RSASSA_PKCS
# This key is used to sign system software, STBM need change this key to their own key
sys1_key_format=ALI
sys1_rsa_key=sys_sw_pair.key
# signed chunk id
sw_chunkid=0x01FE0101
[CMD]
sw_chunk_sign
[PARAMETER]
# signed chunk id
sw_chunkid=0x06F90101
[CMD]
sw_chunk_sign
[PARAMETER]
# signed chunk id
sw_chunkid=0x20DF0100
[CMD]
sw_chunk_sign
[PARAMETER]
# signed chunk id
sw_chunkid=0x20DF0200
[CMD]
sw_chunk_sign
[PARAMETER]
# signed chunk id
sw_chunkid=0x02FD0100
[CMD]
sw_chunk_sign
[PARAMETER]
# signed chunk id
sw_chunkid=0x03FC0100
[CMD]
sw_chunk_sign
[PARAMETER]
# input file name
swcode=product_sabbat_dual.abs
# 16 bytes sw universal key, STBM need to change this key by themselves
sys_uk=decrypt_key.bin
# chunk id
sw_chunkid=0x01FE0101
[CMD]
sw_chunk_enc
[PARAMETER]
# chunk id
sw_chunkid=0x06F90101
[CMD]
sw_chunk_enc
