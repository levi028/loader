;####################################################
;###################### BL1 #########################
;#################################################### 
[PARAMETER]
generation=GR2
output=../../bloader3_3711c.abs

########## Default address is 0xa0000200 ############
loader_entry=0xa1000000
#####################################################

######### Root Key pair, ALI or DER format ##########
rsa_key_format=DER
rsa_key=../../root_key_pair.der
#####################################################
bl_name=../../bloader3_3711c_unify.abs
aux_code=aux_code.abs
mem_init_file=C3711C_Memory_Init.txt

[CMD]
aux_sign
bl_sign
merge      
       