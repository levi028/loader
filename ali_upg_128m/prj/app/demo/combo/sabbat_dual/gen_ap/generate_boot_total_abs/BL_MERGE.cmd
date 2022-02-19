;####################################################
;###################### BL4 #########################
;#################################################### 
[PARAMETER]
generation=GA5

############# RSASSA_PKCS or RSASSA_PSS #############
sig_format=RSASSA_PKCS
#####################################################

############# AES_CBC for or AES_ECB ################
aes_mode=AES_CBC
#####################################################

################## MKT_ID & VER_ID ##################
market_id=0x00000000
market_mask=0xFFFFFFFF
version_id=0x00000001
version_mask=0xFFFFFFFF
#####################################################

##################### HW_MONITOR ####################
hw_monitor1=ffff0000
hw_monitor2=ffff0022
#####################################################

########## Default address is 0xA1000000 ############
loader_entry=0xA2800000
#####################################################

########## Encypt Auxcode Insert to BLMG ############
aux_code_enc=AUX_Code_Enc_demo.abs
aux_code_sign=AUX_Code_Hash_demo.abs.sig
#####################################################

####CLEAR FPK, bootloader will encrypt it.###########
encrypt_format=CLEAR_KEY
#####################################################

#### FlashProtectKey encrypt Boot code ##############
FlashProtectKey=./keys/flashprotectionkey.bin
#####################################################

############ public key level is 1 or 2 #############
pub_key_level=2
#####################################################

##### external public key, X509 with DER format #####
ext_pub_key=./keys/public_2nd_level.key
#####################################################

######### Root Key pair, ALI or DER format ##########
root_key_format=DER
rsa_key=./keys/root_key_pair.der
#####################################################

####### External Pubic Key, ALI or DER format #######
ext_key_format=DER
ext_rsa_key=./keys/ext_key_pair.der
# the next stage software public key, if linux project, this key is used for uboot, if TDS, this key is used for system software
#####################################################

######### SYSTEM1 Pubic Key, ALI or DER format ######
sys1_key_format=DER
sys1_rsa_key=./keys/sys_sw_pair.der
#####################################################

#####################################################
#system software universal key, customer can change this key
#sys_uk=./keys/decrypt_key.bin
#####################################################

bl_name=bl_in.abs
bl_version=1
output=boot_total_area.abs
mem_init_file=C3711C_Memory_Init.txt

[CMD]
ext_key_sign
params_sign
insert_len
insert_key
bl_sign
merge

[STOP]
