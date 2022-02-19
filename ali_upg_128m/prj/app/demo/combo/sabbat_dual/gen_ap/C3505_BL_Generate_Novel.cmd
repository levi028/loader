;####################################################
;##################C3505 BL MERGE####################
;#################################################### 
[PARAMETER]
generation=GA3
chip_name=3505

############# RSASSA_PKCS or RSASSA_PSS #############
sig_format=RSASSA_PKCS
#####################################################

############# AES_CBC for CNX or AES_ECB for NGR ####
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

########## Default address is 0xa0000200 ############
loader_entry=0xa1000000
#####################################################

encrypt_format=CLEAR_KEY  
  
#################################### FlashProtectKey encrypt Boot code ##########################################
FlashProtectKey=BL_Encrypted_M3527.bin
#################################################################################################################  

########## Encypt Auxcode Insert to BLMG ############
aux_code_enc=AUX_Code_Enc_demo_M3527_gen.abs
aux_code_sign=AUX_Code_sign_demo_M3527_ali_gen.abs
#####################################################

############ public key level is 1 or 2 #############
pub_key_level=2
#####################################################

##### external public key, X509 with DER format #####
ext_pub_key=public_2nd_level.key
#####################################################

######### Root Key pair, ALI or DER format ##########
root_key_format=ALI
rsa_key=root_key_pair.key
#####################################################

####### External Pubic Key, ALI or DER format #######
ext_key_format=DER
ext_rsa_key=ext_key_pair.der
# the next stage software public key, if linux project, this key is used for uboot, if TDS, this key is used for system software
#####################################################

######### SYSTEM1 Pubic Key, ALI or DER format ######
sys1_key_format=ALI
sys1_rsa_key=sys_sw_pair.key
#####################################################

#####################################################
#system software universal key, customer can change this key
#sys_uk=customer_key.bin
#sys_uk=decrypt_key.bin
#####################################################
bl_name=bl_in.abs
bl_version=1
output=bl_out.abs
mem_init_file=C3505_Memory_Init.txt

[NAND CONFIG]
#block_perchip=1000
#pages_perblock=0x100
#bytes_perpage=0x800 
#read_timing=0x22 
#write_timing=0x22
############ MICRON #############
#block_perchip=0x800
#pages_perblock=0x100
#bytes_perpage=0x2000
#ecctype=40 
#nand_type=1

############ TOSHIBA #############
#block_perchip=0x800
#pages_perblock=0x100
#bytes_perpage=0x4000
#ecctype=40 
#nand_type=0

############ SK hyniux #############
#block_perchip=0x1000
#pages_perblock=0x40
#bytes_perpage=0x2000
#ecctype=40 
#nand_type=0

############ Default #############
block_perchip=0x800
pages_perblock=0x40
bytes_perpage=0x800
ecctype=16 
nand_type=0

#####################################################

read_timing=0x22 
write_timing=0x22
scramble_en=0
row_count=3

##eccsec_size=0x400
##eccredu_size=0x20
##rowaddr_cycle=0x3
##ecctype=0x0
##eccsec_perpage=0x2

[CMD]
ext_key_sign
params_sign
insert_len
insert_key
bl_version
bl_sign
merge
