[PARAMETER]
# input file name
swcode=app_see_clear.bin
#RSASSA_PKCS or RSASSA_PSS
sig_format=RSASSA_PKCS
# This key is used to sign system software, STBM need change this key to their own key
sys1_key_format=ALI
sys1_rsa_key=see_sw_pair.key
[CMD]
sw_sign