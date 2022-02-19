/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sec_boot.h
*
*    Description:
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _SECURE_BOOT_H_
#define _SECURE_BOOT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BOOT_ERR                libc_printf
#define BOOT_DEBUG_EN           1
#define BOOT_DATA_DEBUG_EN      1

#if 1
#define BOOT_PRINTF               libc_printf
#else
#define BOOT_PRINTF(...)            do{} while(0)
#endif

#if 0
#define BOOT_DEBUG                libc_printf

#define BOOT_DUMP(data, len) { const int l = (len); int i;\
                for (i = 0; i < l; i++) {BOOT_DEBUG(" 0x%02x", *((data) + i)); \
                if((i+1)%16==0) BOOT_DEBUG("\n");}\
                           BOOT_DEBUG("\n");}
#else
#define BOOT_DUMP(data, len)        do{} while(0)
#define BOOT_DEBUG(...)           do{} while(0)
#endif

#define SEE_BOOT_ERR(...)    do{}while(0)
#define SEE_BOOT_DEGUG(...)    do{}while(0)
#define SEE_BOOT_DUMP(data,len)    do{}while(0)

/*For M3515 secure boot*/
#define RSASSA_PKCS1_V1_5 0
#define RSASSA_PSS        1
#define SIG_LEN           256

typedef enum
{
    COPY_DONE   = 0x1 ,
    VERIFY_DONE  ,
    VERIFY_FAIL  ,
    DECRYPT_FAIL ,
    UNZIP_FAIL,
    VERSIION_FAIL,
    OTHER_FAIL,
}ack_type;


typedef struct SIGN_FORMAT
{
    UINT32 length:   16;
    UINT32 format:   2;
    UINT32 reserved: 14;
}SIGN_FORMAT;

struct SEEROM_PARAM
{
    UINT32 see_run_addr;            //Reserved for branch instruction
    UINT32 see_code_addr;
    UINT32 see_code_len;              //Should be "HEAD" 0x44414548
    UINT32 see_sign_location;            //Change ip clock if no zero, now skip it
    SIGN_FORMAT see_sign_format;        //Boot device configuration, such as nand pmi,
    UINT32 debug_key_addr;
};

typedef struct SEE_KEY
{
    UINT32  see_sig_key;
    UINT32 uk_pos ;
}SEE_KEY;

typedef enum
{
   SEEROM_BOOT =  0x1,
   SEE_SW_VERIFY =  0x2,
   SEE_SW_RUN = 0x3,
   SEE_STATUS_RESET = 0x4,
   SEE_SW_RUN_LATER = 0x5,
}seeboot_type;

typedef struct SEE_INFO
{
    UINT32 see_addr;
    UINT32 see_len;
    UINT32 sig_addr;
    UINT32 sig_len;
}SEE_INFO;

#define SEE_ROM_OTP_R_PROTECT            (0x81 *4)
#define SEE_ROM_OTP_R_BITS                (0x7|(0x1ffff<<0x4)) // bit[0:2] and bit[4:20] should be 0

#define BOOTLOADER_ADDRESS_IN_DRAM       0xA0500000//BOOTLOADER_TEMP_ADDR
#define SYS_SEE_SIG_OFFSET               0xC00


#define SYS_SEE_NB_BASE_ADDR 0xb8040000
#define SEEROM_SEE_REAL_RUN_ADDR        (SYS_SEE_NB_BASE_ADDR+0x2b0)
#define SEEROM_SEE_CODE_LOCATION_ADDR   (SYS_SEE_NB_BASE_ADDR+0x2b4)
#define SEEROM_SEE_CODE_LEN_ADDR        (SYS_SEE_NB_BASE_ADDR+0x2b8)
#define SEEROM_SEE_SIGN_LOCATION_ADDR   (SYS_SEE_NB_BASE_ADDR+0x2bC)
#define SEEROM_DEBUG_KEY_ADDR           (SYS_SEE_NB_BASE_ADDR+0x2C0)

/*SEE status register*/
#define SEEROM_SEE_RUN_STATUS_ADDR      (SYS_SEE_NB_BASE_ADDR+0x2C4)
#define SEE_FAIL_BIT    (1<<9) /* see fail log measn if SEE_FAIL_BIT =1 & SEE_DECRYTP_BIT =1 , decrypt fail!!!*/
#define SEE_ACK_BIT     (1<<8) /*see verify pass =1 */
#define SEE_DECRYTP_BIT (1<<7) /*see decrypt pass =1*/
#define SEE_UZIP_BIT    (1<<6) /*see uzip pass =1*/
#define SEE_VERSION_BIT (1<<5) /*see version pass =1*/
#define SEE_COPY_BIT    (1)    /*see code copy to pirvate done =1*/
#define SEE_SYNC_REBOOT_BIT    (1<<16) /*see sync reboot info to main*/
#define SEE_ROM_RUN_BIT         (0xbea<<20)

/*Main status register*/
#define SEEROM_MAIN_RUN_STATUS_ADDR      (SYS_SEE_NB_BASE_ADDR+0x2C8)
#define CPU_GET_FAIL_BIT        (1<<9)
#define CPU_GET_ACK_BIT         (1<<8)  /*CPU get see ack bit status*/
#define CPU_GET_DECRYPT_BIT     (1<<7)  /*CPU get decrypt bit status*/
#define CPU_GET_UZIP_BIT        (1<<6)  /*CPU get uzip bit status*/
#define CPU_GET_VERSIO_BIT      (1<<5)  /*CPU get version check bit status*/
#define CPU_RUN_SW_BIT          (1)     /*CPU run system software*/
#define CPU_TRIG_SW_RUN        ((~SEE_ROM_RUN_BIT)&0xfff00000)
#define SEE_SW_RUN_FLAG      (~CPU_TRIG_SW_RUN )

#define _DECRYPT_SEE    0x1
#define _UZIP_SEE        0x2
#define _VERSION_SEE    0x3
#define _SIG_SEE        0x4

/*For TDS SW*/
#define DUVS_SEE    ((_DECRYPT_SEE <<28) | (_UZIP_SEE << 24)| \
                     (_VERSION_SEE <<20) | (_SIG_SEE <<16 ))

/*For Linux SW*/
#define DVS_SEE    ((_DECRYPT_SEE <<28) | (0xf<< 24)| \
                     (_VERSION_SEE <<20) | (_SIG_SEE <<16 ))
/*For USEE*/
#define S_SEE    ((0xf <<28) | (0xf<< 24)| \
                     (0xf <<20) | (_SIG_SEE <<16 ))

#define DRAM_SPLIT_CTRL_BASE 0xb8041000

#define LZMA_BASE_SIZE  1846
#define LZMA_LIT_SIZE   768
#define BUFFER_SIZE     ((LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (0 + 2))) * sizeof(UINT16))

#define MAX_SEE_LEN  1000000       //10M

/*API for bootloader*/
void set_see_code_info( UINT32 code_addr, UINT32 len, UINT32 run_addr);
void set_see_sig_info(UINT32 sig_location, UINT32 sig_len, UINT32 format );
RET_CODE set_see_key_info( UINT32 key_pos);

/*
* bl_verify_SW_see() Bootloader main cpu trig bootloader_see from SEEROM by this API.
* This funciton will get the bootloader_see code and its signature
* from bootloader code segment.Then config these parameters to I/O .
* At last notify SEE CPU by hardware reset trig to reboot SEE CPU.
*
* c_addr : Bootloader_SEE code address in memory or flash
* c_len  : Bootloader_SEE code len
* entry  : Bootloader_SEE code run address
*/
RET_CODE bl_trig_bl_see(UINT32 see_addr, UINT32 see_len, UINT32 entry);

/*
* bl_verify_SW_see() :Bootloader main cpu trig bootloader_SEE to decrypt/verify/decompress
* SEE software
*/
RET_CODE bl_verify_sw_see(UINT32 param);

/*bl_run_SW_see(): Bootloader main cpu starts SEE software.
* This funciton will notify SEE CPU jump from Bootloader_SEE to software right now
*/
RET_CODE bl_run_sw_see(UINT32);

/*
* bl_run_SW_see_later(): Bootloader main cpu starts SEE software.
* This funciton will notify SEE CPU jump from bootloader_SEE to software
* But the SEE software shall really run after CPU set another flag in fireware
* In TDS , this funcion is prefered.
*/
RET_CODE bl_run_sw_see_later(UINT32);

/*
* bl_sync_see_idle(): Bootloader main cpu sync see idle status (for OTA)
*/
RET_CODE bl_sync_see_idle(UINT32);

/*
* bl_clean_see_status(): Bootloader main cpu clean bootloader_SEE hardware status
*
*/
RET_CODE bl_clean_see_status(UINT32);

/*share global variable(Funciton)*/

/*
* cipher_get_see_from_bl(): The function used to get bootloader see code and signature from bootloader segment.
* For the clear bootloader case, the SEEROM cann't copy the bootloader_see code & sigature
* from the Nor flash directly but just from the main memory.
*
* code_addr   : The output start address of see bootloader code
* code_len :    The output see bootloader code length
* see_info   : The output start address of see signature
*/
RET_CODE cipher_get_see_from_bl(UINT32 code_addr, UINT32 code_len, SEE_INFO * see_info);

/*
* main_notify_see_boot(): The general interface from bootloder to SEE.
* The bootloader use the general interface to act control message to SEE
* param   : The control message parameter.
*/
RET_CODE main_notify_see_boot( UINT32 param);

/*
* set_bl_head_info(): Transfer the bootloader chunk head structure data to SEE
*/
RET_CODE set_bl_head_info(UINT32);

/*
* main_sync_see_idle(): Wait the SEE idle status.
*/
UINT32 main_sync_see_idle(UINT32 tmo);

/*
* is_hw_ack_flag_true():Check see status
*/
UINT32 is_hw_ack_flag_true(UINT32 type);

/*
* set_main_trig_sw_run():set see status to Bootloader_SEE
*/
UINT32 set_main_trig_sw_run(void);


#ifdef __cplusplus
 }
#endif

#endif
