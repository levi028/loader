/*****************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: cc_tsg_mem_rec.c
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2009/2/21      roman
*
*****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <api/libnim/lib_nim.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/decv/decv.h>
#include <bus/tsi/tsi.h>
#include <api/libpvr/lib_pvr.h>
#include <api/libtsi/sec_pmt.h>
#include <api/libsi/si_tdt.h>
#include <api/libttx/lib_ttx.h>
#include <api/libsubt/lib_subt.h>
#include <api/libsi/sie_monitor.h>
#include <api/libtsg/lib_tsg.h>

#include "cc_tsg_task.h"

static UINT8 m_cc_tsg_task_start_ok;

#define MEM_PRT(...)
//#define MEM_PRT libc_printf

typedef struct
{
    UINT32 prog_id;
    UINT32 hnd;
    UINT8  active;
    pfn_prog_id_to_record_param prog_id_to_record;
}MEM_REC_INFO, *PMEM_REC_INFO;

static MEM_REC_INFO mem_rec;

BOOL mem_r_update(UINT32 handle, UINT32 size, UINT16 offset)
{
    if(cc_tsg_task_is_running() == 0 || handle == 0)
        return TRUE;

    cc_tsg_update_cache_buffer(CC_TSG_PLAY_HANDLE, size, offset);
    return TRUE;
}

INT32 mem_r_request(UINT32 handle, UINT8 **addr, INT32 length, INT32 *ret_len)
{
    INT32 len = 0;

    if(cc_tsg_task_is_running() == 0 || handle == 0)
        return 0;

    len =  cc_tsg_request_cache_buffer(handle, addr, length);
    return len;
}

UINT32 pvr_start_memory_record(struct dmx_device *dmx_dev, UINT32 prog_id)
{
    struct pvr_rec_io_param r_param;
    UINT16 pids[PVR_MAX_PID_NUM*2];
    UINT16 pids_bak[PVR_MAX_PID_NUM*2];
    UINT16 pid_num, pid_num_bak;
    UINT32 i, j;
    UINT8 ecm_cnt;
    UINT8 __MAYBE_UNUSED__ emm_cnt;

    struct record_prog_param *prog_info, prog_info_body;

    prog_info = &prog_info_body;
    MEMSET(prog_info, 0, sizeof(struct record_prog_param));
    MEM_PRT("%s, prog_id = %xh\n", __FUNCTION__, prog_id);

    if(mem_rec.active)
    {
        ASSERT(0);
    }


    if(!mem_rec.prog_id_to_record(prog_id, &prog_info_body))
        return 0;

    cc_tsg_task_set_pids(prog_info->pid_info.video_pid, prog_info->pid_info.audio_pid[0],prog_info->av_flag);

    {
        if(prog_info->ca_mode || prog_info->is_scrambled)//if is scrambled tv, fisrt check emm_count to avoid creating too many of empty files
        {
            i = 0;
            while(sie_get_emm_pid(NULL) == 0)
            {
                i++;
                if(i > 10)
                {
                    //return 0;
                    break;
                }
                osal_task_sleep(10);
            }
        }


        pid_num = 0;

		if(H264_DECODER == prog_info->h264_flag)
        {
            pids[pid_num++] = (prog_info->pid_info.video_pid|H264_VIDEO_PID_FLAG);
	    }
        else if(H265_DECODER == prog_info->h264_flag)
		{
			pids[pid_num++] = (prog_info->pid_info.video_pid|H265_VIDEO_PID_FLAG);
        }
        else
        {
            pids[pid_num++] = prog_info->pid_info.video_pid;
        }
        pids[pid_num++] = prog_info->pid_info.audio_pid[prog_info->cur_audio_pid_sel];
        pids[pid_num++] = prog_info->pid_info.pcr_pid;    //PCR
        pids[pid_num++] = prog_info->pid_info.pat_pid;//PAT
        pids[pid_num++] = prog_info->pid_info.pmt_pid; //PMT
        pids[pid_num++] = prog_info->pid_info.sdt_pid; //SDT
        pids[pid_num++] = prog_info->pid_info.eit_pid; //EIT

        if(prog_info->pid_info.audio_count > 1)
        {
            for(i = 0; i < prog_info->pid_info.audio_count; i++)
            {
                if(i != prog_info->cur_audio_pid_sel)
                {
                    pids[pid_num++] = prog_info->pid_info.audio_pid[i];
                }
            }
        }
        //Teletext

#if (TTX_ON == 1)
    {
        struct t_ttx_lang *ttx_list = NULL, *p_ttx = NULL;
        UINT8 num;

        ttxeng_get_init_lang(&ttx_list, &num);
        for(i=0; i<num; i++)
        {
            p_ttx = &ttx_list[i];
            pids[pid_num++]  = p_ttx->pid;
        }
    }
#endif
        //Subtitle
#if (SUBTITLE_ON == 1)
    {
        struct t_subt_lang *subt_list = NULL, *p_subt = NULL;
        UINT8 num;

        subt_get_language(&subt_list, &num);
        for(i=0; i<num; i++)
        {
            p_subt = &subt_list[i];
               pids[pid_num++]  = p_subt->pid;
        }
    }
#endif

        if(prog_info->ca_mode || prog_info->is_scrambled)//emm is needed for playback,ecm is option
        {
            pids[pid_num++] = prog_info->pid_info.cat_pid; //cat
    //#if (SYS_PROJECT_FE != PROJECT_FE_ATSC)
            if((pvr_get_project_mode()&PVR_ATSC)==0)
            {                
#ifdef NEW_DEMO_FRAME
                ecm_cnt = prog_info->pid_info.ecm_pid_count;
                emm_cnt = prog_info->pid_info.emm_pid_count;

                for(i = 0; i < prog_info->pid_info.ecm_pid_count; i++)
                {
                    pids[pid_num++] = prog_info->pid_info.ecm_pids[i];
                }

                for(i = 0; i < prog_info->pid_info.emm_pid_count; i++)
                {
                    pids[pid_num++] = prog_info->pid_info.emm_pids[i];
                }
#else
                sie_get_ecm_emm_cnt(&ecm_cnt, &emm_cnt);
                if(ecm_cnt)
                {
                    pid_num+=sie_get_ecm_pid(&pids[pid_num]);
                }

                if(emm_cnt)
                {
                    pid_num+=sie_get_emm_pid(&pids[pid_num]);
                }
#endif
                if(ecm_cnt ==0)
                {
                    m_cc_tsg_task_start_ok = FALSE;
                    return FALSE;
                }else
                {
                    m_cc_tsg_task_start_ok = TRUE;
                }
//                MEM_PRT("ecm_cnt = %d, emm_cnt = %d\n",ecm_cnt, emm_cnt);
            }
    //#endif
            pids[pid_num++] = prog_info->pid_info.nit_pid; //nit
        }

        //delete same pid!
        pid_num_bak = 2;
        MEMCPY(pids_bak, pids, sizeof(UINT16)*2);//copy V/A/PCR
        for(i=2; i<pid_num; i++)
        {
            for(j=0; j<i; j++)
            {
                if(pids[i] == pids[j])
                {
                    pids[i] = INVALID_PID;
                    break;
                }
            }
            if(pids[i] != INVALID_PID)
            {
                pids_bak[pid_num_bak++] = pids[i];
                if(pid_num_bak>PVR_MAX_PID_NUM)
                    break;
            }
        }

        if(pid_num_bak>PVR_MAX_PID_NUM)
            pid_num_bak =PVR_MAX_PID_NUM;
        pid_num = pid_num_bak;
        MEMCPY(pids, pids_bak, sizeof(UINT16)*pid_num_bak);

        ASSERT(pid_num <= PVR_MAX_PID_NUM);
        
        r_param.io_buff_in     = (UINT8 *)pids;
        r_param.buff_in_len    = pid_num * sizeof(pids[0]); //io_parameter.buff_in_len;
        //r_param.io_buff_out     = io_parameter.io_buff_out;
        //r_param.buff_out_len    = io_parameter.buff_out_len;

        if ( (pids[0] & H264_VIDEO_PID_FLAG) == H264_VIDEO_PID_FLAG)
            r_param.h264_flag = 1;
        else
            r_param.h264_flag = 0;

        r_param.hnd = CC_TSG_PLAY_HANDLE;
        r_param.record_whole_tp = pvr_r_get_record_all();
        r_param.rec_type = 0;
        r_param.is_scrambled = TRUE;
        r_param.request = mem_r_request;
        r_param.update = mem_r_update;

        dmx_start(dmx_dev);
        dmx_io_control(dmx_dev, CREATE_RECORD_STR_EXT, (UINT32)&r_param);
    }

    mem_rec.active = TRUE;
    mem_rec.prog_id = prog_id;
    mem_rec.hnd = CC_TSG_PLAY_HANDLE;

    dmx_io_control(dmx_dev, IO_SET_TSG_AV_MODE, 0);

    return mem_rec.hnd;
}

RET_CODE pvr_stop_memory_record(struct dmx_device *dmx_dev, UINT32 rec_handle)
{
    RET_CODE ret = RET_SUCCESS;

    MEM_PRT("%s, rec_handle = %xh\n", __FUNCTION__, rec_handle);

    if(mem_rec.active)
    {
        // dmx1 no recording.
        ret |= dmx_io_control(dmx_dev, DELETE_RECORD_STR, rec_handle);
    }
    mem_rec.active = 0;
    mem_rec.hnd = 0;
    m_cc_tsg_task_start_ok = FALSE;
    dmx_io_control(dmx_dev, IO_CLEAR_TSG_AV_MODE, 0);
    return ret;
}

void init_mem_record(pfn_prog_id_to_record_param prog_id_to_record)
{
    if (prog_id_to_record == NULL)
    {
        ASSERT(0);
    }
    m_cc_tsg_task_start_ok = FALSE;
    mem_rec.prog_id_to_record = prog_id_to_record;
}

BOOL is_mem_reocder_runing_ok(void)
{
    return m_cc_tsg_task_start_ok;
}


