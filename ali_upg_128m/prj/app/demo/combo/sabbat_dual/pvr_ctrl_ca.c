/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: pvr_ctrl_ca.c
*    Description: All the basic function for ca pvr will be defined.
                  Such as message process, state return.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include "pvr_ctrl_ca.h"
#include "ctrl_key_proc.h"
#include "control.h"
#ifdef SUPPORT_CAS9
#include "conax_ap/win_ca_uri.h"
#include "conax_ap/win_mat_overide.h"
#endif

#ifdef SUPPORT_CAS7
#include "conax_ap7/win_ca_mmi.h"
#include "conax_ap7/win_mat_overide.h"
#endif

#if (defined(CI_PLUS_PVR_SUPPORT) || defined(CAS9_PVR_SUPPORT) || defined(CAS9_PVR_SCRAMBLED) \
||defined(CAS7_PVR_SUPPORT) ||defined(BC_STD_PVR_CRYPTO) ||defined(BC_PVR_SUPPORT)  \
|| defined(C0200A_PVR_SUPPORT) || defined(GEN_CA_PVR_SUPPORT))
#include <hld/trng/trng.h>
#endif

#include <hld/dmx/dmx.h>
#include <hld/dsc/dsc.h>
#include "win_com_popup.h"
#include <api/librsa/rsa_verify.h>
#include <hld/smc/smc.h>

#ifdef CI_PLUS_PVR_SUPPORT
#define CRYPTO_PARAM_TRANS_MODE_GENERAL_PARAM       0
#endif
#define REPLAY_PC_STATUS_PC_PROGRAM                 0x2

#if(  defined (SUPPORT_CAS9)  ||  defined (SUPPORT_CAS7) )
static BOOL pvr_mat_locked = FALSE;
static BOOL pvr_mat_pin_ok = FALSE;
static BOOL pvr_save_mat = FALSE;
#endif

#if(  defined (SUPPORT_CAS9)  ||  defined (SUPPORT_CAS7) )

BOOL api_pvr_get_mat_pin_status(void)
{
    return pvr_mat_pin_ok;
}

void api_pvr_set_mat_pin_status(BOOL new_status)
{
    pvr_mat_pin_ok = new_status;
}

void api_pvr_set_mat_lock_status(BOOL locked)
{
    pvr_mat_locked = locked;
}

BOOL api_pvr_get_mat_lock_status(void)
{
    return pvr_mat_locked;
}
void pvr_need_save_cur_mat(BOOL need_save)
{
    pvr_save_mat = need_save;
}

BOOL pvr_check_need_save_cur_mat(void)
{
    return pvr_save_mat;
}

// Set PVR mat lock playback state and not support preview mode
void api_pvr_set_mat_lock(void)
{
    PVR_STATE play_state = 0;
    UINT8 card_rat = 0;
    struct list_info rl_info;
    BOOL old_value = FALSE;
    BOOL ret = FALSE;
    pvr_play_rec_t  *pvr_info = NULL;
    UINT8 file_rat = 0;
    UINT32 pvr_file_rat __MAYBE_UNUSED__= 0;

#ifdef CAS9_V6
    BOOL file_check = FALSE;
    //UINT8 cache_pin[4]={0};
#else
    UINT32 ptm = 0;
#endif

    pvr_info  = api_get_pvr_info();

    if((FALSE == pvr_info->hdd_valid) || ( (PVR_HANDLE)NULL == pvr_info->play.play_handle ))
    {
        return;
    }
    if (VIEW_MODE_PREVIEW == hde_get_mode())
    {
        return;
    }
    play_state = pvr_p_get_state( pvr_info->play.play_handle);

    pvr_get_rl_info( pvr_info->play.play_index, &rl_info);

#ifndef CAS9_V6    //vicky130812#5
    if (rl_info.ca_mode == 0)
    {
        return;
    }
#endif

#ifndef CAS9_V6
    ptm = pvr_p_get_time( pvr_info->play.play_handle);
    if(pvr_get_mat_rating( pvr_info->play.play_handle, ptm, &pvr_file_rat)!=RET_SUCCESS)
    {
        //libc_printf("pvr_get_mat_rating fail\n");
        return;
    }
    file_rat = pvr_file_rat;
#else
    file_rat = api_cnx_uri_get_mat(); //play_apply_uri_mat
    file_check = api_cnx_uri_get_mat_check();
#endif

    get_cur_mat_value(&card_rat);

#ifndef CAS9_V6
    if ((file_rat > card_rat) && (pvr_mat_locked == FALSE) && (pvr_mat_pin_ok == FALSE))
    {
        // If Playback state is FB, FF, SLOW, B_SLOW and STEP state and will change to rating lock mode,
        // playback state will change to PLAY state.
        if (((NV_FF == play_state) || (NV_FB == play_state) || (NV_SLOW == play_state)
                || (NV_REVSLOW == play_state)  || (NV_STEP == play_state)))
        {
            pvr_p_play( pvr_info->play.play_handle);
        }
        //pvr_p_lock_switch( pvr_info->play.play_handle, TRUE);
        api_pvr_p_lock_switch(TRUE);
        pvr_mat_locked = TRUE;

        old_value = ap_enable_key_task_get_key(TRUE);
        ret = win_matpop_open(NULL, 0, MAT_RATING_FOR_PLAYBACK);
        ap_enable_key_task_get_key(old_value);

        if (ret)
        {
            pvr_mat_pin_ok = TRUE;
            //pvr_p_lock_switch( pvr_info->play.play_handle, FALSE);
            api_pvr_p_lock_switch(FALSE);
            pvr_mat_locked = FALSE;
        }
    }
    else if ((file_rat <= card_rat) && pvr_mat_locked)
    {
        //pvr_p_lock_switch( pvr_info->play.play_handle, FALSE);
        api_pvr_p_lock_switch(FALSE);
        pvr_mat_locked = FALSE;
    }
#else //play_apply_uri_mat
    if ((file_rat > card_rat) && (file_check == TRUE))
    {
        if (((NV_FF == play_state) || (NV_FB == play_state) || (NV_SLOW == play_state)
                || (NV_REVSLOW == play_state)  || (NV_STEP == play_state)))
        {
            pvr_p_play( pvr_info->play.play_handle);
        }
        //pvr_p_lock_switch( pvr_info->play.play_handle, TRUE);
        api_pvr_p_lock_switch(TRUE);

        //vicky140115#9
        #if 0//should always popup mat lock
        if((gmat_unlock) && (gmat_prog_level>=file_rat)
            && ((PVR_STATE_TMS==pvr_info->pvr_state)
            || (PVR_STATE_TMS_PLAY==pvr_info->pvr_state)            
            || (PVR_STATE_REC_TMS==pvr_info->pvr_state)
            || (PVR_STATE_REC_TMS_PLAY==pvr_info->pvr_state)))
        {
            sys_data_get_cached_ca_pin(cache_pin);
            ret = win_matpop_open(cache_pin, 0, MAT_RATING_FOR_PLAYBACK);
        }
        else
        #endif    
        {
            old_value = ap_enable_key_task_get_key(TRUE);
            ret = win_matpop_open(NULL, 0, MAT_RATING_FOR_PLAYBACK);
            #if 0
            if(TRUE == ret) //fix issue:play mat stream, not enter pin in live,enter tms then enter pin,stop to live,should not pop pin window 
            {
                //libc_printf("set mat value2 for live play check\n");
                gmat_unlock = TRUE;
                gmat_prog_level = file_rat;
            }
            #endif
            ap_enable_key_task_get_key(old_value);
        }

        if (ret)
        {
            //pvr_p_lock_switch( pvr_info->play.play_handle, FALSE);
            api_pvr_p_lock_switch(FALSE);
            api_cnx_uri_set_mat_check(FALSE);
        }
    }
#endif
}

#endif


#ifdef CI_PLUS_PVR_SUPPORT
BOOL api_pvr_update_ciplus_info(PVR_HANDLE handle)
{
    BOOL ret = FALSE;
    UINT8 enc_uri_msg[PVR_CIPLUS_URI_LEN] = {0};
    UINT32 msglen = 0;
    UINT8 emi = 0;
    UINT8 rl = 0;
    record_ciplus_uri_item ciplus_info;

    MEMSET(&ciplus_info, 0, sizeof(record_ciplus_uri_item));

    ciplus_offer_uri(enc_uri_msg, &msglen, &emi, &rl);
    if (PVR_CIPLUS_URI_LEN != msglen)
    {
        libc_printf("Error! URI length changed!\n");
    }
    MEMCPY(ciplus_info.u_detail, enc_uri_msg, PVR_CIPLUS_URI_LEN);
    ciplus_info.ptm = pvr_r_get_time(handle);

    if (PVR_COPY_NO_MORE == emi)
    {
        ciplus_info.copy_type = PVR_COPY_NEVER;
    }
    else if (PVR_COPY_ONE_GENR == emi)
    {
        ciplus_info.copy_type = PVR_COPY_NO_MORE;
    }
    else
    {
        ciplus_info.copy_type = emi;
    }

    ciplus_info.retention_limit = rl;
    libc_printf("got new URI: EMI = %d, RL = %d\n", ciplus_info.copy_type, ciplus_info.retention_limit);
    pvr_set_ciplus_uri(pvr_get_index(handle), &ciplus_info);
    ret = TRUE;

    return ret;
}
#endif


#ifdef CI_PLUS_PVR_SUPPORT
static int _pvr_ciplus_param_trans(void *dst_param, void *src_param, int trans_mode)
{
    ciplus_crypto_general_param *ciplus_param = NULL;
    pvr_crypto_general_param *pvr_param = NULL;

    if (CRYPTO_PARAM_TRANS_MODE_GENERAL_PARAM ==  trans_mode)
    {
        ciplus_param = (ciplus_crypto_general_param *)dst_param;
        pvr_param = (pvr_crypto_general_param *)src_param;
        ciplus_param->pvr_hnd = pvr_param->pvr_hnd;
        ciplus_param->pid_list = pvr_param->pid_list;
        ciplus_param->pid_num = pvr_param->pid_num;
        ciplus_param->dmx_id = pvr_param->dmx_id;
        ciplus_param->ca_mode = pvr_param->ca_mode;
        ciplus_param->key_num = pvr_param->key_num;
        ciplus_param->key_len = pvr_param->key_len;
        ciplus_param->keys_ptr = pvr_param->keys_ptr;
        ciplus_param->crypto_mode = pvr_param->crypto_mode;
        return 0;
    }
    return -1;
}

#endif /* CI_PLUS_PVR_SUPPORT */

#if (defined(CI_PLUS_PVR_SUPPORT) || defined(CAS9_PVR_SUPPORT) || defined(CAS9_PVR_SCRAMBLED) \
    || defined(C0200A_PVR_SUPPORT) || defined(CAS7_PVR_SUPPORT) || defined(BC_STD_PVR_CRYPTO) \
	|| defined(BC_PVR_SUPPORT) || defined(GEN_CA_PVR_SUPPORT))

/* Generate random numbers as encrypted keys */
static int _pvr_generate_keys(pvr_crypto_key_param *key_param)
{
    UINT32 i = 0;
    UINT32 key_bytes = 0;
    UINT32 loop_cnt1 = 0;
    UINT32 loop_cnt2 = 0;
    UINT8 *key_ptr = NULL;
    UINT32 group=8;

    key_bytes = (key_param->key_len + 7) / 8;         // bytes for one key
    key_bytes = key_bytes * key_param->key_num;        // total bytes for all keys
    loop_cnt1 = key_bytes / 8;                        // generate 64bits per loop
    loop_cnt2 = key_bytes % 8;                        // generate 1Byte per loop
    PVR_CRYPTO_DEBUG("generate keys for recording, loop_cnt: (%d, %d)\n", loop_cnt1, loop_cnt2);
    key_ptr = key_param->keys_ptr;
    for (i = 0; i < loop_cnt1/group; ++i)
    {
       if(RET_SUCCESS != trng_get_64bits(key_ptr,group))
        {
            //libc_printf("%s() Tng Error! in Loop1(%d)\n",__FUNCTION__,(UINT8)i);
            osal_task_sleep(1);
        }
        key_ptr += group*8;
    }

    for (i = 0; i < loop_cnt2; ++i)
    {
        if(RET_SUCCESS != trng_generate_byte(key_ptr))
        {
            //libc_printf("%s() Tng Error! in Loop2(%d)\n",__FUNCTION__,(UINT8)i);
            osal_task_sleep(1);
        }
         key_ptr++;
    }
    return 0;
}


/* remove the invalid or duplicate pids in @pid_list
 *    @pid_list [in/out]: pids to be checked
 *  @pid_num  [in]    : the pid number in @pid_list
 *    @return : the valid pid number in @pid_list
 */
static UINT16 _pvr_check_reencrypt_pids(UINT16 *pid_list, UINT16 pid_num)
{
    UINT16 valid_pid_num = 0;
    UINT16 i = 0;
    UINT16 j = 0;

    // remove the invalid and duplicate pids
#if PVR_CRYPTO_DEBUG_ENABLE
    PVR_CRYPTO_DEBUG("%s() pid_list 1: ", __FUNCTION__);
    for (i = 0; i < pid_num; i++)
    {
        PVR_CRYPTO_DEBUG("0x%X ", pid_list[i]);
    }
    PVR_CRYPTO_DEBUG("\n");
#endif

    for (i = 0; i < pid_num; i++)
    {
        if ((0 == pid_list[i]) || (INVALID_PID == (pid_list[i] & INVALID_PID)))
        {
            continue;
        }
        for (j = 0; j < valid_pid_num; j++)
        {
            if (pid_list[i] == pid_list[j])
            {
                pid_list[i] = INVALID_PID;
                break;
            }
        }

        if (j >= valid_pid_num)
        {
            pid_list[valid_pid_num++] = pid_list[i];
        }
    }

#if PVR_CRYPTO_DEBUG_ENABLE
    PVR_CRYPTO_DEBUG("%s() pid_list 2: ", __FUNCTION__);
    for (i = 0; i < valid_pid_num; i++)
    {
        PVR_CRYPTO_DEBUG("0x%X ", pid_list[i]);
    }
    PVR_CRYPTO_DEBUG("\n");
#endif

    return valid_pid_num;
}


/* set re-encrypt pids to @pid_list
 *    @pid_info        [in] : pids info
 *    @pid_list        [out]: pids to be re-encrypted
 *  @pid_list_size    [in] : max size of @pid_list
 *    @return : the valid pid number in @pid_list
 */
static UINT16 _pvr_set_reencrypt_pids(struct pvr_pid_info *pid_info,
                                      UINT16 *pid_list, UINT16 pid_list_size)
{
    UINT16 pid_num = 0;
    UINT16 i=0;
    //pvr_play_rec_t  *pvr_info = NULL;

    //pvr_info  = api_get_pvr_info();

    if ((pid_info->video_pid != INVALID_PID) && (pid_num < pid_list_size))
    {
        PVR_CRYPTO_DEBUG("re-encrypt video pid: 0x%X\n", pid_info->video_pid);
        pid_list[pid_num++] = pid_info->video_pid;
    }

    for (i = 0; (i < pid_info->audio_count) && (pid_num < pid_list_size); i++)
    {
        PVR_CRYPTO_DEBUG("re-encrypt audio pid %d: 0x%X\n", i, pid_info->audio_pid[i]);
        pid_list[pid_num++] = pid_info->audio_pid[i];
    }

    for (i = 0; (i < pid_info->ttx_pid_count) && (pid_num < pid_list_size); i++)
    {
        PVR_CRYPTO_DEBUG("re-encrypt ttx pid %d: 0x%X\n", i, pid_info->ttx_pids[i]);
        pid_list[pid_num++] = pid_info->ttx_pids[i];
    }

    for (i = 0; (i < pid_info->ttx_subt_pid_count) && (pid_num < pid_list_size); i++)
    {
        PVR_CRYPTO_DEBUG("re-encrypt ttx_subt pid %d: 0x%X\n", i, pid_info->ttx_subt_pids[i]);
        pid_list[pid_num++] = pid_info->ttx_subt_pids[i];
    }

    for (i = 0; (i < pid_info->subt_pid_count) && (pid_num < pid_list_size); i++)
    {
        PVR_CRYPTO_DEBUG("re-encrypt subt pid %d: 0x%X\n", i, pid_info->subt_pids[i]);
        pid_list[pid_num++] = pid_info->subt_pids[i];
    }

    return _pvr_check_reencrypt_pids(pid_list, pid_num);
}

#endif /* (defined(CI_PLUS_PVR_SUPPORT) || defined(CAS9_PVR_SUPPORT)) */

#if defined(BC_PVR_SUPPORT)
INT8 pvr3_evnt_callback_for_bc(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    UINT16 pid_num = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_crypto_pids_param *pids_param = NULL;
    pvr_crypto_general_param *pvr_param = NULL;
    pvr_block_decrypt_param *decrypt_param = NULL;
    P_NODE p_node;
    INT32 ret = 0;
    pvr_info  = api_get_pvr_info();
    switch (msg_type)
    {
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pid_num = pids_param->pid_num;
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pid_num);
            return 0;
        }
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pids_param->pid_num);
            return 0;
        }
        case PVR_MSG_REC_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting record\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr,
                             pvr_param->crypto_mode, pvr_param->qn_per_key);

            if (bc_pvr_rec_config(pvr_param) != 0)
            {
                PVR_CRYPTO_ERROR("bc_pvr_rec_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting record, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            return (INT8)bc_pvr_rec_stop(pvr_param);
        }
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting play\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr,
                             pvr_param->crypto_mode, pvr_param->qn_per_key);

            if (bc_pvr_playback_config(pvr_param, (0 == pvr_info->tms_r_handle) ? 0 : 1) != 0)
            {
                PVR_CRYPTO_ERROR("bc_pvr_playback_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting play, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            ret = (INT8)bc_pvr_playback_stop(pvr_param);
            if (P_STOP_AND_REOPEN == api_pvr_get_stop_play_attr())
            {
                sys_data_get_curprog_info(&p_node);
#ifdef _BC_AUI_
                bc_mcas_set_dsc_live_path(p_node.prog_number);
#else
                bc_set_dsc_live_play(bc_get_viewing_bc_serviceidx());
#endif
            }
            return ret;
        }
        case PVR_MSG_BLOCK_MODE_DECRYPT:
        {
            decrypt_param = (pvr_block_decrypt_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr block mode decrypt, pvr_hnd: 0x%X, input: 0x%X, length: %d, block index: %d\n",
                             decrypt_param->pvr_hnd, decrypt_param->input, decrypt_param->length, decrypt_param->block_idx);
            return bc_pvr_block_decrypt(decrypt_param->pvr_hnd, decrypt_param->input, 
                decrypt_param->length, decrypt_param->block_idx);
        }
    }
}
#endif /* BC_PVR_SUPPORT */

#if (defined(CAS7_PVR_SUPPORT) || defined(BC_PVR_STD_SUPPORT))
INT8 pvr3_evnt_callback_for_cas7(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    int ret = -1;
    pvr_crypto_pids_param *pids_param = NULL;
    pvr_crypto_general_param *pvr_param = NULL;
    dsc_parse_resource dpr;

    switch (msg_type)
    {
        case PVR_MSG_CRYPTO_DATA:
        {
            ret = pvr_crypto_data((pvr_crypto_data_param *)msg_code);
            return ret ;
        }
        case PVR_MSG_REC_GET_KREC:
        {
            ret = (INT8)pvr_crypto_generate_keys((pvr_crypto_key_param *)msg_code);
            return ret ;
        }
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pids_param->pid_num);
            return 0;
        }
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pids_param->pid_num);
            return 0;
        }
        case PVR_MSG_REC_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting record\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr, pvr_param->crypto_mode);

            MEMSET(&dpr, 0, sizeof(dsc_parse_resource));
            dpr.decrypt_stream_id = 0;
            dpr.decrypt_mode = CSA;
            dpr.decrypt_dev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);


            if (pvr_crypto_rec_config(pvr_param, &dpr) != 0)
            {
                PVR_CRYPTO_ERROR("ciplus_pvr_rec_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting record, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            return (INT8)pvr_crypto_rec_stop(pvr_param);
        }
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting play\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr, pvr_param->crypto_mode);

            if (pvr_crypto_playback_config(pvr_param) != 0)
            {
                PVR_CRYPTO_ERROR("cas9_pvr_playback_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting play, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            ret = (INT8)pvr_crypto_playback_stop(pvr_param);

            MEMSET(&dpr, 0, sizeof(dsc_parse_resource));
            dpr.decrypt_stream_id = 0;
            dpr.decrypt_mode = CSA;
            dpr.decrypt_dev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);

            pvr_crypto_set_dsc_parse(&dpr);

            return ret;
        }
    }
    return ret;
}
#endif /*CAS7_PVR_SUPPORT*//* BC_STD_PVR_CRYPTO*/

#ifdef  C0200A_PVR_SUPPORT
static INT8 pvr3_evnt_callback_for_c0200a(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    UINT16 pid_num = 0;
    INT8    ret = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_crypto_pids_param *pids_param  = NULL;
    pvr_crypto_general_param *pvr_param = NULL;
    pvr_crypto_data_param *cp_data = NULL;

#ifdef MULTI_DESCRAMBLE
    UINT32 prog_id = 0;
    P_NODE p_node;
#endif

    pvr_info  = api_get_pvr_info();
    switch (msg_type)
    {
        case PVR_MSG_REC_GET_RP_INFO:
        case PVR_MSG_REC_GET_FP_KEY_INFO:
        case PVR_MSG_REC_GET_URI_KEY_INFO:
        case PVR_MSG_REC_GET_KREC:
        {
            return (INT8)_pvr_generate_keys((pvr_crypto_key_param *)msg_code);
        }
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pid_num = pids_param->pid_num;
#ifdef C200A_PVR_REENCRYPT_PIDS_FROM_CA
#ifdef MULTI_DESCRAMBLE
            prog_id = pvr_r_get_channel_id(handle);
            pid_num = ca_get_dsc_pid_multi_des(pids_param->pid_list, pid_num, prog_id);
            //libc_printf("%s,prog_id=%d\n",__FUNCTION__,prog_id);
#else
            pid_num = ca_get_dsc_pid((UINT8 *)pids_param->pid_list, pid_num);
#endif
            pids_param->pid_num = _pvr_check_reencrypt_pids(pids_param->pid_list, pid_num);
#else
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pid_num);
#endif
            return 0;
        }
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pids_param->pid_num);
            return 0;
        }
        case PVR_MSG_REC_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting record\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr,
                             pvr_param->crypto_mode, pvr_param->qn_per_key);

            if (c200a_pvr_rec_config(pvr_param) != 0)
            {
                PVR_CRYPTO_ERROR("c200a_pvr_rec_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting record, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            return (INT8)c200a_pvr_rec_stop(pvr_param);
        }
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting play\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr,
                             pvr_param->crypto_mode, pvr_param->qn_per_key);

            if (c200a_pvr_playback_config(pvr_param, (0 == pvr_info->tms_r_handle) ? 0 : 1) != 0)
            {
                PVR_CRYPTO_ERROR("c200a_pvr_playback_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting play, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            ret = (INT8)c200a_pvr_playback_stop(pvr_param);
            if (P_STOP_AND_REOPEN == api_pvr_get_stop_play_attr())
            {
#ifdef MULTI_DESCRAMBLE
                MEMSET(&p_node, 0, sizeof(P_NODE));
                sys_data_get_curprog_info(&p_node);
                prog_id = p_node.prog_id;
                //libc_printf("%s,handle=0x%x,prog_id=%d\n",__FUNCTION__,handle,prog_id);
                if ((prog_id != 0) && ((PVR_STATE_TMS_PLAY ==  pvr_info->pvr_state)
                                       || (PVR_STATE_REC_PLAY == pvr_info->pvr_state)))
                {
                    //timeshift to living need to config crypto
                    c200a_set_dsc_for_live_play(api_get_ca_living_dmx(),
                                               (UINT32)ca_get_ts_stream_id_multi_des(prog_id));
                }

#endif
                c200a_set_dsc_for_live_play(0, (UINT32)0);
            }
            return ret;
        }
        case PVR_MSG_CRYPTO_DATA:
        {
            return (INT8)c200a_crypto_data((pvr_crypto_data_param *)msg_code);
        }
        case PVR_MSG_CAL_CHUNK_HASH:
        {
            cp_data = (pvr_crypto_data_param *)msg_code;
            //return  create_sha_ramdrv(cp->input, cp->data_len, cp->output);
            return calculate_hmac(cp_data->input, cp_data->data_len, cp_data->output, cp_data->key_ptr);
        }
        default:
            break;
    }
    return 0;
}
#endif
#ifdef CAS9_PVR_SUPPORT
static INT8 pvr3_evnt_callback_for_cas9(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    UINT16 pid_num = 0;
    INT8    ret = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_crypto_pids_param *pids_param  = NULL;
    pvr_crypto_general_param *pvr_param = NULL;
    pvr_crypto_data_param *cp_data = NULL;

#ifdef MULTI_DESCRAMBLE
    UINT32 prog_id = 0;
    P_NODE p_node;
#endif

#ifdef NEW_TIMER_REC
   UINT32 prog_id_m=0;  //prog_id for main play 
#endif

    pvr_info  = api_get_pvr_info();
    switch (msg_type)
    {
        case PVR_MSG_REC_GET_RP_INFO:
        case PVR_MSG_REC_GET_FP_KEY_INFO:
        case PVR_MSG_REC_GET_URI_KEY_INFO:
        case PVR_MSG_REC_GET_KREC:
        {
            return (INT8)_pvr_generate_keys((pvr_crypto_key_param *)msg_code);
        }
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pid_num = pids_param->pid_num;
#ifdef CAS9_PVR_REENCRYPT_PIDS_FROM_CA
#ifdef MULTI_DESCRAMBLE
            prog_id = pvr_r_get_channel_id(handle);
            #ifdef NEW_TIMER_REC                        
            if(RET_SUCCESS==cas9_pvr_get_liveplay_prog_id(&prog_id_m))
            {                    
                if(prog_id !=prog_id_m)    //Other Chan Rec
                {
                    pid_num=ca_get_pvr_pid_info(pids_param->pid_list, pid_num, prog_id);
                }
                else    //Same Chan Rec
                {
            pid_num = ca_get_dsc_pid_multi_des(pids_param->pid_list, pid_num, prog_id);
                }
            }
            else
            {
                 //libc_printf("%s-Line(%d)-Fail to run cas9_pvr_get_liveplay_prog_id()\n",__FUNCTION__,__LINE__);
            }
            #else    
            pid_num = ca_get_dsc_pid_multi_des(pids_param->pid_list, pid_num, prog_id);
            #endif
            //libc_printf("%s,prog_id=%d\n",__FUNCTION__,prog_id);
#else
            pid_num = ca_get_dsc_pid((UINT8 *)pids_param->pid_list, pid_num);
#endif
            pids_param->pid_num = _pvr_check_reencrypt_pids(pids_param->pid_list, pid_num);
#else
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pid_num);
#endif
            return 0;
        }
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pids_param->pid_num);
            return 0;
        }
        case PVR_MSG_REC_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting record\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr,
                             pvr_param->crypto_mode, pvr_param->qn_per_key);

            if (cas9_pvr_rec_config(pvr_param) != 0)
            {
                PVR_CRYPTO_ERROR("cas9_pvr_rec_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting record, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            return (INT8)cas9_pvr_rec_stop(pvr_param);
        }
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting play\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr,
                             pvr_param->crypto_mode, pvr_param->qn_per_key);

            if (cas9_pvr_playback_config(pvr_param, (0 == pvr_info->tms_r_handle) ? 0 : 1) != 0)
            {
                PVR_CRYPTO_ERROR("cas9_pvr_playback_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting play, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            ret = (INT8)cas9_pvr_playback_stop(pvr_param);
            if (P_STOP_AND_REOPEN == api_pvr_get_stop_play_attr())
            {
#ifdef MULTI_DESCRAMBLE
                MEMSET(&p_node, 0, sizeof(P_NODE));
                sys_data_get_curprog_info(&p_node);
                prog_id = p_node.prog_id;
                //libc_printf("%s,handle=0x%x,prog_id=%d\n",__FUNCTION__,handle,prog_id);
                if ((prog_id != 0) && ((PVR_STATE_TMS_PLAY ==  pvr_info->pvr_state)
                                       || (PVR_STATE_REC_PLAY == pvr_info->pvr_state)))
                {
                    //timeshift to living need to config crypto
                    cas9_set_dsc_for_live_play(api_get_ca_living_dmx(),
                                               (UINT32)ca_get_ts_stream_id_multi_des(prog_id));
                }
#elif (defined(CAS9_PVR_SCRAMBLED))
                cas9_set_dsc_for_live_play(api_get_ca_living_dmx(), (UINT32)ca_get_ts_stream_id());
#else
                cas9_set_dsc_for_live_play(0, (UINT32)ca_get_ts_stream_id());
#endif
            }
            return ret;
        }
        case PVR_MSG_CRYPTO_DATA:
        {
            return (INT8)cas9_crypto_data((pvr_crypto_data_param *)msg_code);
        }
        case PVR_MSG_CAL_CHUNK_HASH:
        {
            cp_data = (pvr_crypto_data_param *)msg_code;
            //return  create_sha_ramdrv(cp->input, cp->data_len, cp->output);
            return calculate_hmac(cp_data->input, cp_data->data_len, cp_data->output, cp_data->key_ptr);
        }
        case PVR_MSG_PLAY_FILE_DEL:
	      {
	         ap_send_msg(CTRL_MSG_SUBTYPE_CMD_DEL_FILE,0,FALSE);
			     return 0;
        }
        default:
            break;
    }
    return 0;
}
#endif /* CAS9_PVR_SUPPORT */

#ifdef GEN_CA_PVR_SUPPORT

#ifdef _C0700A_VSC_ENABLE_
/* Generate random numbers as encrypted keys */
static int _casA_pvr_generate_keys(pvr_crypto_key_param *key_param)
{
    UINT32 i = 0;
    UINT32 key_bytes = 0;
    UINT32 loop_cnt1 = 0;
    UINT32 loop_cnt2 = 0;
    UINT8 *key_ptr = NULL;
    UINT32 group=8;
    UINT8 rand_data[64];
    UINT8 rand_byte =0;

    //UINT32 tick1= osal_get_tick();
    key_bytes = (key_param->key_len + 7) / 8;         // bytes for one key
    key_bytes = key_bytes * key_param->key_num;        // total bytes for all keys
    loop_cnt1 = key_bytes / 8;                        // generate 64bits per loop
    loop_cnt2 = key_bytes % 8;                        // generate 1Byte per loop
    PVR_CRYPTO_DEBUG("generate keys for recording, loop_cnt: (%d, %d)\n", loop_cnt1, loop_cnt2);
    key_ptr = key_param->keys_ptr;

    trng_get_64bits(rand_data,group);
    
    for (i = 0; i < loop_cnt1/group; ++i)
    {
        MEMCPY(key_ptr, rand_data, 64);
        key_ptr += group*8;
    }

    trng_generate_byte(&rand_byte);
    for (i = 0; i < loop_cnt2; ++i)
    {
        *key_ptr = rand_byte;
        key_ptr++;
    }
    //UINT32 tick2=osal_get_tick();
    //libc_printf("_casA_pvr_generate_keys: time %d\n", tick2-tick1);
    return 0;
}
#endif
static INT8 pvr3_evnt_callback_for_genCA(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    UINT16 pid_num = 0;
    UINT32 prog_id = 0;
    INT8    ret = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_crypto_pids_param *pids_param  = NULL;
    pvr_crypto_general_param *pvr_param = NULL;
    __MAYBE_UNUSED__ pvr_crypto_data_param *cp_data = NULL;    

    pvr_info  = api_get_pvr_info();
    switch (msg_type)
    {
        case PVR_MSG_REC_GET_KREC:
        case PVR_MSG_REC_GET_RP_INFO:
        
        {
            #ifdef _C0700A_VSC_ENABLE_
                return (INT8)_casA_pvr_generate_keys((pvr_crypto_key_param *)msg_code);
            #else
                return (INT8)_pvr_generate_keys((pvr_crypto_key_param *)msg_code);
            #endif
        }
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pid_num = pids_param->pid_num;

            prog_id = pvr_r_get_channel_id(handle);
            pid_num = get_dsc_pid_multi_des(pids_param->pid_list, pid_num, prog_id);
            pids_param->pid_num = _pvr_check_reencrypt_pids(pids_param->pid_list, pid_num);

            return 0;
        }
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pids_param->pid_num);
            return 0;
        }
        case PVR_MSG_REC_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting record\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr,
                             pvr_param->crypto_mode, pvr_param->qn_per_key);

            if (gen_ca_pvr_rec_config(pvr_param) != 0)
            {
                PVR_CRYPTO_ERROR("gen_ca_pvr_rec_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting record, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            return (INT8)gen_ca_pvr_rec_stop(pvr_param);
        }
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr start re-encrypting play\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr,
                             pvr_param->crypto_mode, pvr_param->qn_per_key);

            if (gen_ca_pvr_playback_config(pvr_param, (0 == pvr_info->tms_r_handle) ? 0 : 1) != 0)
            {
                PVR_CRYPTO_ERROR("gen_ca_pvr_playback_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        {
            pvr_param = (pvr_crypto_general_param *)msg_code;
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting play, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            ret = (INT8)gen_ca_pvr_playback_stop(pvr_param);
            if (P_STOP_AND_REOPEN == api_pvr_get_stop_play_attr())
            {
            //live_stream_id , live_dmx_id should ask CA library to get value
               UINT32 live_stream_id = 0;
               UINT16 live_dmx_id = 0;
               gen_ca_set_dsc_for_live_play(live_dmx_id,live_stream_id);
            }
            return ret;
        }
        case PVR_MSG_CRYPTO_DATA:
        {
            return (INT8)gen_ca_crypto_data((pvr_crypto_data_param *)msg_code);
        }
        case PVR_MSG_CAL_CHUNK_HASH:
        {
            cp_data = (pvr_crypto_data_param *)msg_code;
            //return  create_sha_ramdrv(cp->input, cp->data_len, cp->output);
            #ifdef _C0700A_VSC_ENABLE_
                return RET_SUCCESS;
            #else
                return calculate_hmac(cp_data->input, cp_data->data_len, cp_data->output, cp_data->key_ptr);
            #endif
        }
        default:
            break;
    }
    return 0;
}
#endif/* GEN_CA_PVR_SUPPORT */

#if (defined(CI_PLUS_PVR_SUPPORT) && !defined(BC_STD_PVR_CRYPTO))
INT8 pvr3_evnt_callback_for_ci_plus(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    pvr_play_rec_t  *pvr_info = NULL;
    ciplus_crypto_general_param ciplus_param;
    ciplus_crypto_general_param *pvr_param = NULL;
    pvr_crypto_pids_param *pids_param = NULL;

#ifdef CIPLUS_PVR_REENCRYPT_PIDS_FROM_CI
    UINT8 slot_mask = 0;
    UINT16 *pid_list = NULL;
    UINT8 ci_dsc_pid_num = 0;
    UINT8 pid_max_num = 0;
#endif

    pvr_info  = api_get_pvr_info();
    switch (msg_type)
    {
        case PVR_MSG_REC_GET_KREC:
        {
            return (INT8)_pvr_generate_keys((pvr_crypto_key_param *)msg_code);
        }
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
#ifdef CIPLUS_PVR_REENCRYPT_PIDS_FROM_CI
            slot_mask = 1 << ci_get_cur_dsc_slot();
            pid_list = pids_param->pid_list;
            ci_dsc_pid_num = 0;
            pid_max_num = (UINT8)pids_param->pid_num;

            if (api_ci_get_dscpid_lst(slot_mask, pid_list, &ci_dsc_pid_num, pid_max_num) == FALSE)
            {
                return -1;
            }
            pids_param->pid_num = _pvr_check_reencrypt_pids(pids_param->pid_list, (UINT16)ci_dsc_pid_num);
            return 0;
#else
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pids_param->pid_num);
#endif
            return 0;
        }
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        {
            pids_param = (pvr_crypto_pids_param *)msg_code;
            pids_param->pid_num = _pvr_set_reencrypt_pids(pids_param->pid_info,
                                                          pids_param->pid_list, pids_param->pid_num);
            return 0;
        }
        case PVR_MSG_REC_START_OP_STARTDMX:
        {
            pvr_param = &ciplus_param;
            MEMSET(pvr_param, 0, sizeof(ciplus_crypto_general_param));
            _pvr_ciplus_param_trans((void *)pvr_param, (void *)msg_code, CRYPTO_PARAM_TRANS_MODE_GENERAL_PARAM);
            PVR_CRYPTO_DEBUG("pvr start re-encrypting record\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr, pvr_param->crypto_mode);

            if (ciplus_pvr_rec_config(pvr_param) != 0)
            {
                PVR_CRYPTO_ERROR("ciplus_pvr_rec_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        {
            pvr_param = &ciplus_param;
            MEMSET(pvr_param, 0, sizeof(ciplus_crypto_general_param));
            _pvr_ciplus_param_trans((void *)pvr_param, (void *)msg_code, CRYPTO_PARAM_TRANS_MODE_GENERAL_PARAM);
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting record, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            return (INT8)ciplus_pvr_rec_stop(pvr_param);
        }
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        {
            pvr_param = &ciplus_param;
            MEMSET(pvr_param, 0, sizeof(ciplus_crypto_general_param));
            _pvr_ciplus_param_trans((void *)pvr_param, (void *)msg_code, CRYPTO_PARAM_TRANS_MODE_GENERAL_PARAM);
            PVR_CRYPTO_DEBUG("pvr start re-encrypting play\n"
                             "\tpvr_hnd: 0x%X, pid num: %d, dmx id: %d, ca_mode: %d\n"
                             "\tkey info: (%d, %d, 0x%X, %d)\n",
                             pvr_param->pvr_hnd, pvr_param->pid_num, pvr_param->dmx_id, pvr_param->ca_mode,
                             pvr_param->key_num, pvr_param->key_len, pvr_param->keys_ptr, pvr_param->crypto_mode);

            if (ciplus_pvr_playback_config(pvr_param) != 0)
            {
                PVR_CRYPTO_ERROR("cas9_pvr_playback_config() fail\n");
                return -1;
            }
            return 0;
        }
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        {
            pvr_param = &ciplus_param;
            MEMSET(pvr_param, 0, sizeof(ciplus_crypto_general_param));
            _pvr_ciplus_param_trans((void *)pvr_param, (void *)msg_code, CRYPTO_PARAM_TRANS_MODE_GENERAL_PARAM);
            PVR_CRYPTO_DEBUG("pvr stop re-encrypting play, pvr_hnd: 0x%X, dmx id: %d\n",
                             pvr_param->pvr_hnd, pvr_param->dmx_id);
            ret = (INT8)ciplus_pvr_playback_stop(pvr_param);
            ciplus_set_dsc_for_live_play(0, (UINT32)ciplus_get_ts_stream_id());
            return ret;
        }
        case PVR_MSG_PLAY_RL_SHIFT:
            pvr_info->play.rl_a_time = msg_code;


            break;
        case PVR_MSG_PLAY_URI_NOTIFY:
            ciplus_fetch_uri((UINT8 *)msg_code, 16);
            break;
        case PVR_MSG_PLAY_RL_RESET:
            break;
        case PVR_MSG_PLAY_RL_INVALID:
            break;
        default:
            break;
    }

}
#endif /* CI_PLUS_PVR_SUPPORT */

BOOL is_pvr3_ca_evnt(UINT32 msg_type, BOOL *is_return)
{
    BOOL ret = FALSE;

    switch (msg_type)
    {
#if (defined(CI_PLUS_PVR_SUPPORT) && !defined(BC_STD_PVR_CRYPTO))
        case PVR_MSG_REC_GET_KREC:
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        case PVR_MSG_REC_START_OP_STARTDMX:
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        case PVR_MSG_PLAY_RL_SHIFT:
            *is_return = TRUE;
            ret = TRUE;
            break;
        case PVR_MSG_PLAY_URI_NOTIFY:
        case PVR_MSG_PLAY_RL_RESET:
        case PVR_MSG_PLAY_RL_INVALID:
            *is_return = FALSE;
            ret = TRUE;
            break;
#endif
#if (defined(CAS7_PVR_SUPPORT) || defined(BC_PVR_STD_SUPPORT))
        case PVR_MSG_CRYPTO_DATA:
        case PVR_MSG_REC_GET_KREC:
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        case PVR_MSG_REC_START_OP_STARTDMX:
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
            *is_return = TRUE;
            ret = TRUE;
            break;
#endif
#if (defined(CAS9_PVR_SUPPORT) || defined(C0200A_PVR_SUPPORT) || defined(GEN_CA_PVR_SUPPORT))
        case PVR_MSG_REC_GET_KREC:
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        case PVR_MSG_REC_START_OP_STARTDMX:
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        case PVR_MSG_CRYPTO_DATA:
        case PVR_MSG_CAL_CHUNK_HASH:
        case PVR_MSG_REC_GET_RP_INFO:
        case PVR_MSG_PLAY_FILE_DEL:
            *is_return = TRUE;
            ret = TRUE;
            break;
#endif
#ifdef BC_PVR_SUPPORT
        case PVR_MSG_REC_SET_REENCRYPT_PIDS:
        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
        case PVR_MSG_REC_START_OP_STARTDMX:
        case PVR_MSG_REC_STOP_OP_STOPDMX:
        case PVR_MSG_PLAY_START_OP_STARTDMX:
        case PVR_MSG_PLAY_STOP_OP_STOPDMX:
        case PVR_MSG_BLOCK_MODE_DECRYPT:
            *is_return = TRUE;
            ret = TRUE;
            break;
#endif
#ifdef CAS9_PVR_SCRAMBLED
        case PVR_MSG_CRYPTO_DATA:
        case PVR_MSG_REC_GET_KREC:
            *is_return = TRUE;
            ret = TRUE;
            break;
#endif
        default:
            *is_return = TRUE;
            ret = FALSE;
            break;
    }
    return ret;
}
INT8 pvr3_evnt_callback_for_ca(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    INT8 ret = 0;

#ifdef _INVW_JUICE
    /* inview_pvr_callback is used to pass pvr based event to Juice */
    if (inview_pvr_callback) { inview_pvr_callback(handle, msg_type, msg_code); }
#endif /* _INVW_JUICE */
    switch (msg_type)
    {
#ifdef CAS9_PVR_SCRAMBLED
        case PVR_MSG_CRYPTO_DATA:
        {
            ret = (INT8)cas9_crypto_data((pvr_crypto_data_param *)msg_code);
        }
        case PVR_MSG_REC_GET_KREC:
        {
            ret = (INT8)_pvr_generate_keys((pvr_crypto_key_param *)msg_code);
        }
#endif

        default:

#if (defined(CI_PLUS_PVR_SUPPORT) && !defined(BC_STD_PVR_CRYPTO))
            ret = pvr3_evnt_callback_for_ci_plus(handle,  msg_type,  msg_code);
#endif

#if (defined(CAS7_PVR_SUPPORT) || defined(BC_PVR_STD_SUPPORT))
            ret = pvr3_evnt_callback_for_cas7(handle,  msg_type,  msg_code);
#endif

#ifdef CAS9_PVR_SUPPORT
            ret = pvr3_evnt_callback_for_cas9(handle,  msg_type,  msg_code);
#endif

#ifdef GEN_CA_PVR_SUPPORT
            ret = pvr3_evnt_callback_for_genCA(handle,  msg_type,  msg_code);
#endif

#ifdef C0200A_PVR_SUPPORT
            ret = pvr3_evnt_callback_for_c0200a(handle,  msg_type,  msg_code);
#endif
#if defined(BC_PVR_SUPPORT)
            ret = pvr3_evnt_callback_for_bc(handle,  msg_type,  msg_code);
#endif
            break;
    }
    return ret;
}

BOOL api_pvr_is_item_canbe_play(UINT8 *back_saved, UINT32 rl_idx __MAYBE_UNUSED__, 
    PVR_STATE state __MAYBE_UNUSED__, BOOL preview_mode __MAYBE_UNUSED__)
{
    struct list_info play_pvr_info;
    struct list_info temp_play_pvr_info;
#ifdef CAS9_V6    
    struct smc_device *smc_dev;    
    smc_dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);
    UINT8 card_status = 0;
#endif    

#ifdef BC_PVR_SUPPORT
    extern UINT8 replay_pc_status;
#endif

    MEMSET(&play_pvr_info, 0, sizeof(struct list_info));
    MEMSET(&temp_play_pvr_info, 0, sizeof(struct list_info));
    api_get_temp_play_pvr_info(&temp_play_pvr_info);
    api_get_play_pvr_info(&play_pvr_info);

#if defined(SUPPORT_CAS9) || defined (SUPPORT_CAS7)
#ifdef COMBO_CA_PVR
    if (1 == temp_play_pvr_info.rec_type)
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg_ext("PS record is not supported!", NULL, 0);
        win_compopup_open_ext(back_saved);
        return FALSE;
    }

    if (( (RSM_C0200A_MULTI_RE_ENCRYPTION == temp_play_pvr_info->rec_special_mode) ||
        (RSM_CAS9_RE_ENCRYPTION == temp_play_pvr_info.rec_special_mode)  ||
            (RSM_CAS9_MULTI_RE_ENCRYPTION == temp_play_pvr_info.rec_special_mode)) &&
            (!temp_play_pvr_info.is_scrambled))
    {
        if (!ca_is_card_inited())
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
            win_compopup_set_msg_ext("Can not play without correct card!", NULL, 0);
            win_compopup_open_ext(back_saved);
            return FALSE;
        }
    }
#else
    if ((1 == temp_play_pvr_info.rec_type) && (1 == temp_play_pvr_info.ca_mode))
    {
        win_compopup_init(WIN_POPUP_TYPE_OK);
        win_compopup_set_msg_ext("PS record is not supported!", NULL, 0);
        win_compopup_open_ext(back_saved);
        return FALSE;
    }

    #if defined(CAS9_V6) && defined(CAS9_PlAY_REC_NO_CARD)
    if(TRUE==temp_play_pvr_info.ca_mode)
    #endif
    {   //vicky_20141027_play_fta_even_no_card
        if (!ca_is_card_inited())
        {
            win_compopup_init(WIN_POPUP_TYPE_OK);
#ifdef CAS9_V6
            card_status = api_cnx_get_correct_card(); 
            if (SUCCESS == smc_card_exist(smc_dev))
            {
                switch (card_status)
                {
                    case CA_CARD_PROBLEM:
                        win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_CARD_PROBLEM);
                        break;
                    case CA_CARD_INCORRECT:
                        win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_INCORRECT_CARD);
                        break;                    
                    default:
                        win_compopup_set_msg_ext(NULL, NULL, RS_UNKNOWN_ERROR);
                        break;
                }
            }
            else
            {
            win_compopup_set_msg_ext(NULL, NULL, RS_CONAX_NO_CARD);
            }
#else
            win_compopup_set_msg_ext("Can not play without correct card!", NULL, 0);
#endif
            win_compopup_open_ext(back_saved);
            return FALSE;
        }
    }//vicky_20141027_play_fta_even_no_card
#endif
#endif

#ifdef BC_PVR_SUPPORT
    if (0 != bc_pvr_playback_check(rl_idx, state, preview_mode))
    {
        return FALSE;
    }
    else
    {
        if (REPLAY_PC_STATUS_PC_PROGRAM == replay_pc_status)   //parental control program without correct pin
        {
            if (preview_mode)
            {
                api_show_row_logo(MEDIA_LOGO_ID);
            }
            return FALSE;
        }
    }
#endif

#ifdef SUPPORT_CAS7
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_info  = api_get_pvr_info();
    if ((api_pvr_is_recording()) || (PVR_STATE_TMS == pvr_info->pvr_state))
    {
        if ((RSM_COMMON_RE_ENCRYPTION == play_pvr_info.rec_special_mode)
                && (play_pvr_info.record_de_encryp_key_mode != get_cur_record_key_mode()))
        {

            win_compopup_init(WIN_POPUP_TYPE_SMSG);
#ifdef SD_UI
            win_compopup_set_frame(PVR_SD_UI_WIN_SHOW_X, PVR_SD_UI_WIN_SHOW_Y,
                                   PVR_SD_UI_WIN_SHOW_W, PVR_SD_UI_WIN_SHOW_H);
#endif
            win_compopup_set_msg_ext("Can not play old version record while recording!", NULL, 0);
            win_compopup_open_ext(back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
            win_compopup_smsg_restoreback();
            MAT_DEBUG_PRINTF("Can not play the old version record while recoding \n");

            return FALSE;
        }
    }
#endif
    return TRUE;
}

#ifdef CI_PLUS_PVR_SUPPORT
static BOOL ap_pvr_get_partition_id(const char *mnt_name, UINT32 *partition_id)
{
    BOOL ret = FALSE;
    int temp_ret = RET_FAILURE;
    UINT8 buf[16] = { 0 };
    UINT8 i = 0;
    int   valid_mnt_len = 9;


#ifdef USB_SUPPORT_HUB
    struct usb_disk_info_ex disk_info;
    int disk_type = 0;
    char dev_path[16];
    int node_id = 0;
    int fd = 0;
    int tmp = 0;
#else
    struct usb_disk_info disk_info;
#endif

    if (STRLEN(mnt_name) < valid_mnt_len) //only deal with partition like "/mnt/uda1"...
    {
        return ret;
    }
    partition_id[0] = 0;
    partition_id[1] = 0;

    if (MEMCMP(mnt_name, "/mnt/sd", 7) == 0)
    {
        MEMSET(buf, 0, 16);
        temp_ret = sd_m33_ioctl(SD_GET_ID, (UINT32)buf);
        if (temp_ret != RET_SUCCESS)
        {
            return ret;
        }

        //pack 8 Bytes into partition_id
        partition_id[0] = buf[15];
        partition_id[0] <<= 8;
        partition_id[0] |= buf[14];
        partition_id[0] <<= 8;
        partition_id[0] |= buf[13];
        partition_id[0] <<= 8;
        partition_id[0] |= buf[12];

        partition_id[1] = buf[11];
        partition_id[1] <<= 8;
        partition_id[1] |= buf[10];
        partition_id[1] <<= 8;
        partition_id[1] |= buf[9];
        partition_id[1] <<= 8;
        partition_id[1] |= buf[8];

        libc_printf("CID valid is ");
        for (i = 0; i < 16; i++)
        {
            libc_printf("%02x", buf[i]);
        }
        libc_printf(" \n");
        libc_printf("  Manufacturer ID: 0x%02x \n", buf[15]);
        libc_printf("  OEM/Application ID: %c%c \n", buf[14], buf[13]);
        libc_printf("  Product name: %c%c%c%c%c\n", buf[12], buf[11], buf[10], buf[9], buf[8]);
        libc_printf(" \n");
    }
    else if (MEMCMP(mnt_name, "/mnt/ud", 7) == 0)
    {
#ifdef USB_SUPPORT_HUB
        MEMSET(&disk_info, 0, sizeof(struct usb_disk_info_ex));
        disk_type = disk_name_prefix2type(&mnt_name[5]);
        if (disk_type == MNT_TYPE_USB)
        {
            snprintf(dev_path, 16, "/dev/%s", &mnt_name[5]);
            fd = fs_open(dev_path, O_RDONLY, 0);
            tmp = fs_ioctl(fd, IOCTL_GET_NODEID, &node_id, sizeof(int));
            fs_close(fd);

            if (tmp == 0)
            {
                temp_ret =    usbd_get_disk_info_ex(node_id, &disk_info);
            }
        }
#else
        MEMSET(&disk_info, 0, sizeof(struct usb_disk_info));
        temp_ret =  usbd_get_disk_info(&disk_info);
#endif
        if (temp_ret != RET_SUCCESS)
        {
            return ret;
        }

        partition_id[0] = disk_info.bcd_usb;
        partition_id[0] <<= 16;
        partition_id[0] |= disk_info.id_vendor;

        partition_id[1] = disk_info.id_product;
        partition_id[1] <<= 16;
        partition_id[1] |= disk_info.i_serial_number;

        libc_printf("  bcdUSB = %x\n", disk_info.bcd_usb);
        libc_printf("  idVendor = %x\n", disk_info.id_vendor);
        libc_printf("  idProduct = %x\n", disk_info.id_product);
        libc_printf("  iSerialNumber = %x\n", disk_info.i_serial_number);
    }

    ret = TRUE;

    return partition_id;
}
#endif
void ap_pvr_monitor_ci_plus(struct list_info *rl_info __MAYBE_UNUSED__)
{
#ifdef CI_PLUS_PVR_SUPPORT
    pvr_play_rec_t  *pvr_info = NULL;
    UINT32 rl_a_time = 0;
    INT32 day = 0;
    INT32 hour = 0;
    INT32 min = 0;
    INT32 sec = 0;
    UINT32 retention_time = 0;
    struct PVR_DTM a_time;
    struct PVR_DTM cur_time;

    pvr_info  = api_get_pvr_info();
        if(( pvr_info->play.play_handle != 0) && ((POBJECT_HEAD)&g_win_pvr_ctrl != menu_stack_get_top()))
    {
        pvr_get_rl_info(pvr_get_index( pvr_info->play.play_handle), &rl_info);
        if ((rl_info.is_reencrypt) && (PVR_COPY_NEVER ==  rl_info.copy_control))
        {
            retention_time = _pvr_data_trans_retention_time(rl_info.retention_limit);
                pvr_info->play.rl_a_time = 0;

            _pvr_get_local_time(&cur_time);
            _pvr_convert_time_by_offset(&a_time, &cur_time,
                                        -(retention_time / PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN),
                                        -((retention_time % PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN) / 60));
            // check Start <= A_time
            if (__pvr_compare_time(&rl_info.tm, &a_time) <= 0)
            {
                _pvr_get_time_offset(&rl_info.tm, &a_time, &day, &hour, &min, &sec);
                // here to add some seconds to avoid the limit band affect
                rl_a_time = day * PVR_TIME_HOUR_PER_DAY * PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN + hour
                            * PVR_TIME_MIN_PER_HOUR * PVR_TIME_S_PER_MIN + min * PVR_TIME_S_PER_MIN + sec + 1;

                // check the rl_A_time < duration
                if (rl_info.duration + 2 <= rl_a_time)
                {
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_PVR, PVR_END_DATAEND +
                                (pvr_msg_hnd2idx(pvr_info.play.play_handle) << HND_INDEX_OFFSET), TRUE);
                    osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
                }
            }
        }
    }
#endif
}

#ifdef CI_PLUS_PVR_SUPPORT

void ap_pvr_ci_plus_msg_proc(UINT32 pvr_msg_code, UINT8 *back_saved)
{
    UINT32 pvr_handle = 0;
    UINT8 rec_pos = 0;
    UINT32 pic_time = 0;
    pvr_play_rec_t  *pvr_info = NULL;
    pvr_record_t *rec = NULL;
    struct list_info  pvr_info_temp;
    enum VDEC_PIC_STATUS pic_status;

    api_get_pvr_info(&pvr_info);
    rec = api_pvr_get_rec_by_prog_id(pvr_info->play.play_chan_id, &rec_pos);
    switch (pvr_msg_code)
    {
        case PVR_MSG_PLAY_RL_RESET:
            if(osal_get_tick() > ( pvr_info->play.rl_mutex_ptm + PVR_CI_PLUS_MSG_PROC_TIME))
            {
                libc_printf("APP reset RL play point to %d!\n", pvr_info->play.rl_a_time+3);
                pvr_info->play.rl_mutex_ptm = osal_get_tick()+PVR_CI_PLUS_MSG_PROC_TIME;
                //change to paly state before excute "prev/next/enter" keys when not in normal play state
                //change to paly state before excute "prev/next/enter" keys when not in normal play state
                if(pvr_p_get_state( pvr_info->play.play_handle) != NV_PLAY)
                {
                    pvr_p_play( pvr_info->play.play_handle);
                }
                pvr_p_timesearch( pvr_info->play.play_handle, pvr_info->play.rl_a_time+3);
            }
            else
            {
                libc_printf("APP drop reset RL play point Request!\n");
            }
            break;
        case PVR_MSG_PLAY_RL_INVALID:
            //fix bug: re-encrypt tms/record-playback -> RL record playback,
            // will cause live channel no AV, reason is not call "api_ci_aes_stop"
            pvr_handle = 0;
            if( pvr_info->tms_r_handle != 0)
            {
                pvr_handle = pvr_info->tms_r_handle;
            }
            else if (rec != NULL)
            {
                pvr_handle = rec->record_handle;
            }

            if (pvr_handle != 0)
            {
                MEMSET(&pvr_info_temp, 0, sizeof(struct list_info));
                pic_status = key_get_video_status(&pic_time);
                pvr_get_rl_info(pvr_get_index(pvr_handle), &pvr_info_temp);
                if ((pvr_info_temp.ca_mode) && (pvr_info_temp.is_reencrypt) && (pic_status != VDEC_PIC_NORMAL))
                {
                    ciplus_set_dsc_for_live_play(0, (UINT32)ciplus_get_ts_stream_id());
                }
            }

            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
            win_compopup_set_msg("Recording Retention Limit!", NULL, 0);
            win_compopup_open_ext(back_saved);
            osal_task_sleep(PVR_POP_WIN_SHOW_TIME_DEFALUT);
            win_compopup_smsg_restoreback();
            break;
    }
}
#endif

#ifdef CAS7_PVR_SCRAMBLE
static  void pvr_ca_playback_pre(void)
{
    struct dec_parse_param paramcsa;
    struct dmx_device *pdmx_dev = NULL;
    P_CSA_DEV pcsa_dev = NULL;

    pcsa_dev = (P_CSA_DEV)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);
    paramcsa.dec_dev = (void *)pcsa_dev;
    paramcsa.type = CSA;
    MEMSET(&paramcsa, 0, sizeof(struct dec_parse_param));
    pdmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, PVR_DMX_IDX2);
    dmx_io_control(pdmx_dev, IO_SET_DEC_HANDLE, (UINT32)&paramcsa);
    dmx_io_control(pdmx_dev, IO_SET_DEC_STATUS, (UINT32)1);
}
#endif
//name:clear_mmi_message
//Description: Clear the mmi message for CA Scrambled record
//Need cro CAS9_PVR_SCRAMBLED or CAS7_PVR_SCRAMBLE
void clear_mmi_message(void)
{
#ifdef CAS9_PVR_SCRAMBLED
    /*clean msg*/
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);  //fixed bug44675
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
#endif

#ifdef CAS7_ORIG_STREAM_RECORDING
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);  //fixed bug44675
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
#endif
#ifdef CAS7_PVR_SCRAMBLE
    /*clean msg*/
    clean_mmi_msg(CA_MMI_PRI_01_SMC, TRUE);  //fixed bug44675
    clean_mmi_msg(CA_MMI_PRI_03_MAT, TRUE);
    clean_mmi_msg(CA_MMI_PRI_04_PPV, TRUE);
    clean_mmi_msg(CA_MMI_PRI_06_BASIC, TRUE);
    pvr_ca_playback_pre();
#endif
}

