/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: secure_boot_api.c
*
*    Description:
*          Approvel CPU/SEE secure boot-up comucation API
*
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
     PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <hld/sec_boot/sec_boot.h>
#include <osal/osal_cache.h>

#if (defined MAIN_CPU) && (defined BL_ENABLE)
#include <modules.h>
#include <api/libchunk/chunk.h>
#include <hld/sto/sto_dev.h>
#include <api/libc/string.h>
#include <hld/crypto/crypto.h>
#include <api/librsa/flash_cipher.h>
#include <api/librsa/rsa_verify.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>

/*APIs for bootloader*/

/*[API]: Bootloader main cpu trig bootloader_see from SEEROM by this API.
*        This funciton will get the bootloader_see code and its signature
*        from bootloader code segment.Then config these parameters to I/O .
*        At last notify SEE CPU by hardware reset trig to reboot SEE CPU.
*[Parameters]:
       c_addr : Bootloader_see code address in memory or flash
       c_len  : Bootloader_see code len
*/

RET_CODE bl_trig_bl_see(UINT32 c_addr, UINT32 c_len, UINT32 entry)
{
    RET_CODE ret = RET_FAILURE;
    SEE_INFO info;
    INT32 ret1 = 0;
    UINT32 otp_r_mask = 0 ;

    BOOT_DEBUG("----------Try to trig SEE_ROM boot-up --------\n");

    if(RET_SUCCESS != (ret = otp_init(NULL)))
    {
        BOOT_ERR("bl_boot_min_see: otp module init failed\n");
        goto ret_err;
    }


    ret1 = otp_read(SEE_ROM_OTP_R_PROTECT,&otp_r_mask,4);
    if(ret1 < 0x4)
    {
        BOOT_ERR("bl_boot_min_see: otp read failed\n");
        goto ret_err;
    }
    if(otp_r_mask & SEE_ROM_OTP_R_BITS)
    {
        BOOT_ERR("bl_boot_min_see: otp read protection error\n");
        goto ret_err;
    }

    MEMSET(&info, 0x0, sizeof(SEE_INFO));
    ret = cipher_get_see_from_bl(c_addr,c_len,&info);
    if(RET_FAILURE == ret)
    {
        BOOT_ERR("bl_boot_min_see: get see failed\n");
        goto ret_err;
    }
    set_see_code_info(info.see_addr,info.see_len,entry);
    set_see_sig_info(info.sig_addr, info.sig_len, RSASSA_PKCS1_V1_5);
    set_bl_head_info(0);

    ret = main_notify_see_boot(SEEROM_BOOT);
    if(ret != RET_SUCCESS)
    {
        BOOT_ERR("bootloader bl_see boot-up failed\n");
        goto ret_err;
    }
    BOOT_DEBUG("----------Trig SEE_ROM boot-up done --------\n");
    return ret ;
ret_err:
    BOOT_DEBUG("bl see failed, goto reboot ...");
    hw_watchdog_reboot();

}

/*
* [API]: Bootloader main cpu sync see idle status (for OTA)
*
*/
RET_CODE bl_sync_see_idle(UINT32 param)
{
    RET_CODE ret=RET_FAILURE;
    UINT32 tmo = 30 ;

    ret = main_sync_see_idle(tmo);
    if(ret != RET_SUCCESS)
    {
        BOOT_ERR("bootloader sw_see boot-up failed\n");
        return !RET_SUCCESS;
    }
    return RET_SUCCESS ;
}

/*
* [API]: Bootloader main cpu clean bootloader_SEE hardware status
*
*/
RET_CODE bl_clean_see_status(UINT32 param)
{
    RET_CODE ret=RET_FAILURE;

    ret = main_notify_see_boot( SEE_STATUS_RESET );
    if(ret != RET_SUCCESS)
    {
        BOOT_ERR("bootloader sw_see boot-up failed\n");
        return !RET_SUCCESS;
    }
    return RET_SUCCESS ;
}

/*[API]: Bootloader main cpu starts SEE software.
*        This funciton will notify SEE CPU jump from bootloader_SEE to software right now
*/
RET_CODE bl_run_sw_see(UINT32 param)
{
    RET_CODE ret = RET_FAILURE;

    BOOT_DEBUG("----------Try to run see software  --------\n");

    ret = main_notify_see_boot( SEE_SW_RUN);
    if(ret != RET_SUCCESS)
    {
        BOOT_ERR("bootloader bl_see boot-up failed\n");
        return !RET_SUCCESS;
    }
    BOOT_DEBUG("----------Run software boot-up work done --------\n");
    return RET_SUCCESS ;
}

/*[API]: Bootloader main cpu starts SEE software.
*        This funciton will notify SEE CPU jump from bootloader_SEE to software
*         But the SEE software shall really run after CPU set another flag in fireware
*         In TDS , this funcion is prefered.
*/
RET_CODE bl_run_sw_see_later(UINT32 param)
{
    RET_CODE ret = RET_FAILURE;

    BOOT_DEBUG("----------Try to run see software  --------\n");

    ret = main_notify_see_boot( SEE_SW_RUN_LATER);
    if(ret != RET_SUCCESS)
    {
        BOOT_ERR("bootloader bl_see boot-up failed\n");
        return !RET_SUCCESS;
    }
    BOOT_DEBUG("----------Run software boot-up work done --------\n");
    return RET_SUCCESS ;
}


/*[API]: Bootloader main cpu trig bootloader_SEE to decrypt/verify/decompress
         SEE software
*
*/
RET_CODE bl_verify_sw_see(UINT32 param)
{
    RET_CODE ret=RET_FAILURE;

    if(RET_SUCCESS != bl_clean_see_status(0))
    {
        FIXED_PRINTF( "second loader clean see status failed\n" );
        return !RET_SUCCESS;
    }

    ret = main_notify_see_boot( param|SEE_SW_VERIFY);
    if(ret != RET_SUCCESS)
    {
        BOOT_ERR("bootloader sw_see boot-up failed\n");
        return !RET_SUCCESS;
    }
    return RET_SUCCESS ;
}
/*
* Dummy for compiler
*/
RET_CODE remote_hld_notify_see_trig(UINT32 param)
{
    return RET_FAILURE;
}

#endif /*MAIN_CPU*/

UINT32 is_hw_ack_flag_true(UINT32 type)
{
    return  (*(volatile UINT32 *)(SEEROM_SEE_RUN_STATUS_ADDR)) & (type) ;
}

UINT32 set_main_trig_sw_run(void)
{
    return  ((*(volatile UINT32 *)(SEEROM_MAIN_RUN_STATUS_ADDR)) =  CPU_TRIG_SW_RUN);
}

UINT32 set_see_sw_run_flag()
{
    return ((*(volatile UINT32 *)(SEEROM_MAIN_RUN_STATUS_ADDR)) = SEE_SW_RUN_FLAG);
}

RET_CODE set_see_parameter(UINT32 run_addr, UINT32 code_addr,UINT32 len)
{
	*(volatile UINT32 *)(SEEROM_SEE_REAL_RUN_ADDR) = run_addr|0xa0000000 ;
	osal_cache_flush((void *)code_addr,len);
	*(volatile UINT32 *)(SEEROM_SEE_CODE_LEN_ADDR) = *(volatile UINT32 *)(0xa7000200 + 0x100) ;
	*(volatile UINT32 *)(SEEROM_SEE_CODE_LOCATION_ADDR) = code_addr|0xa0000000 ;      
	*(volatile UINT32 *)(SEEROM_SEE_SIGN_LOCATION_ADDR) = 0xa7000200 ;
	return RET_SUCCESS;
}

