#if 0//new or change add by yuj
/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_otaupg.h
*
*    Description:   The realize of OTA upgrade
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_OTAUPG_H_
#define _WIN_OTAUPG_H_
#ifdef __cplusplus
extern "C"
{
#endif
extern OSAL_ID  g_ota_flg_id;
#include <api/libota/lib_ota.h>
extern UINT32 get_code_version(UINT8 *data_addr, UINT32 data_len);
extern TEXT_CONTENT otamsg_mtxt_content[];      /* */
extern TEXT_CONTENT otasw_mtxt_content[];      /* */
extern MULTI_TEXT   otaupg_mtxt_msg;      /* */
extern MULTI_TEXT   otaupg_mtxt_swinfo;      /* */
extern TEXT_FIELD   otaupg_txt_btn;      /* */
extern PROGRESS_BAR otaupg_progress_bar;       /* */
extern TEXT_FIELD   otaupg_progress_txt;//
typedef enum
{
    OTA_STEP_GET_INFO = 0,
    OTA_STEP_DOWNLOADING,
    OTA_STEP_BURNFLASH,
}OTA_STEP;

typedef enum E_APP_KEY
{
    E_KEY_MAIN,
    E_KEY_SEE,
}E_APP_KEY;

UINT16 bool_flash_mem(void);
INT32 burn_flash(void);

RET_CODE decrypt_universal_key_wrapped(UINT8 *pos,UINT32 app_key_idx);
BOOL get_chunk_header_in_flash(UINT32 ck_id,UINT8* buf);
UINT32 get_chunk_in_flash(UINT32 ck_id,UINT8 *buf);
UINT32 get_ram_chunk_version(UINT8* data,UINT8 key_pos,UINT8* tmp_buf);
UINT32 get_ram_chunk_version_ext(UINT8* data,UINT8 key_pos,UINT8* tmp_buf,UINT8 rsa_key_id);
UINT32 get_code_version(UINT8 *data_addr, UINT32 data_len);
UINT32 get_code_version_ext(UINT8 *data_addr, UINT32 data_len,UINT8 rsa_key_id);
UINT8 get_rsa_key_id_by_type(UINT8 key_type);

UINT32 get_see_code_version_f(UINT32 chk_id);
UINT32 get_see_code_version_m(UINT32 chk_id,UINT8* addr);

#ifdef _BC_CA_ENABLE_
#ifdef _SMI_ENABLE_
//
#else
UINT32 get_chunk_in_flash_ex(UINT32 ck_id,UINT8 *buf,UINT32 buf_size);
UINT32 get_chunk_data_in_flash(UINT32 ck_id,UINT8 *buf,UINT32 buf_size);
void get_mem_chunk_version(UINT8 *ck_maddr,UINT8 key_pos,UINT8 *decrypt_addr,UINT32 *pversion);
void get_see_version_in_mem(UINT8 *ck_maddr,UINT32 *pversion);
#endif
BOOL check_main_see_size(UINT32 size_in_upgrade);
#endif


#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif
#endif

