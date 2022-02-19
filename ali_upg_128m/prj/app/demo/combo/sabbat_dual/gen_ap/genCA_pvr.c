/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: genCA_pvr.c
*
*    Description: This file contains the callback functions for re-encryption of transport stream
                 and encryption and HMAC of related metadata
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <asm/chip.h>
#include <api/libci/ci_plus.h>
#include <openssl/fips_rand.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/dmx/dmx.h>
#include <openssl/aes.h>
#include <basic_types.h>
#include <osal/osal.h>
#include <bus/otp/otp.h>
#include <retcode.h>
#include <hal/hal_gpio.h>
#include <hld/crypto/crypto.h>
#include <api/libc/alloc.h>
#include <hld/dmx/dmx_dev.h>
#include <api/libcas/mcas.h>
#include <api/libpvr/lib_pvr.h>
#include <api/librsa/rsa_verify.h>
#include <asm/chip.h>
#include "genCA_pvr.h"
#include <api/libtsi/db_3l.h>
#include "generate_key.h"
#include "../pvr_ctrl_basic.h"

/*******************************************************************************
*    definitions
*******************************************************************************/
#if 0
#define GEN_CA_PVR_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#define GEN_CA_PVR_ERROR(fmt, arg...) \
    do { \
        libc_printf("Error: %s line %d: "fmt, __FILE__, __LINE__, ##arg); \
    } while (0)
#else
#define GEN_CA_PVR_DEBUG(...)            do{} while(0)
#define GEN_CA_PVR_ERROR(...)            do{} while(0)
#endif

#define __DECRYPT_STREAM_ID_FIX__  //Enable it if the stream ID User in CA lib is fix.	//Ben 180619#2

#define INVALID_DSC_SUB_DEV_ID            0xff
#define INVALID_DSC_STREAM_ID            0xff
#define INVALID_REC_ID  0xff                
#define MAX_PVR_DE_ENCONFIG     3      
#define MAX_DSC_STREAM 8
#define INVALID_CE_KEY_POS                0xff

#define INVALID_CRYPTO_STREAM_ID        INVALID_DSC_STREAM_ID
#define INVALID_CRYPTO_STREAM_HANDLE    0xffffffff

#define GEN_CA_PVR_RSC_TYPE_REC            0
#define GEN_CA_PVR_RSC_TYPE_PLAY            1


#define GEN_CA_PVR_RSC_PLAY_NUM            PVR_MAX_PLAY_NUM
#define INVALID_CHANNEL_ID            0xff

#define gen_ca_pvr_lock()        osal_mutex_lock(g_cpm.mutex_id, OSAL_WAIT_FOREVER_TIME)
#define gen_ca_pvr_un_lock()    osal_mutex_unlock(g_cpm.mutex_id)

extern UINT32 g_pvr_level2pos;
extern BOOL get_mute_state(void);
extern RET_CODE gen_ca_pvr_setup_key(UINT32 key_pos);

/*******************************************************************************
* Local variables declaration
*******************************************************************************/
typedef struct
{
    UINT8 playing;    //0:stop, 1:playing, 2:jump
    UINT8 nextSetted;
    UINT8 state;    //0: normal, 1:backward
    UINT32 storeInfoBegin;
    UINT32 storeInfoEnd;
    UINT32 pvr_hnd;
    UINT32 key_pos;
    UINT32 jump_time;
}gen_ca_replay_status;

typedef struct
{
    UINT32    pvr_hnd;
    UINT32    crypto_stream_hnd;    // crypto stream handle used by PVR to record or playback
    UINT32    crypto_stream_id;    // crypto stream id used by PVR to record or playback
    UINT32    crypto_key_pos;        // crypto stream key pos used by PVR to record or playback
    
    UINT32 rec_id;  // Record idx
    UINT32 aes_id;  //AES device idx    
    UINT32 dec_stream_id;   //decrypt stream id

} gen_ca_pvr_resource;

typedef struct
{
    ID                    mutex_id;
    gen_ca_pvr_resource     rec[GEN_CA_PVR_RSC_REC_NUM];
    gen_ca_pvr_resource     play[GEN_CA_PVR_RSC_PLAY_NUM];
} gen_ca_pvr_mgr;

/* PVR Manager */
static gen_ca_pvr_mgr g_cpm;
static UINT16 timeshift_stream_id = 0xff;     //save the stream id for do timeshift

// crypto related devices
static p_ce_device pvr_ce_dev = NULL;
static p_dsc_dev pvr_dsc_dev  = NULL;
static p_aes_dev playback_aes_dev  = NULL; // used to encrypt or decrypt TS data
static UINT32 aes_dev_id = INVALID_DSC_SUB_DEV_ID;

// pvr_de_enconfig is used by other module, MUST defined as static or global
static DEEN_CONFIG pvr_de_enconfig[MAX_PVR_DE_ENCONFIG]={{0,NULL,0,0,0,NULL,0,0},
    {0,NULL,0,0,0,NULL,0,0},{0,NULL,0,0,0,NULL,0,0}};

static gen_ca_replay_status gen_ca_trackReplay={0, INVALID_CHANNEL_ID, FALSE, 0, 0, 0, 0, 0};
static OSAL_ID g_gen_ca_tsk_id = OSAL_INVALID_ID;

/*******************************************************************************
*    Local functions
*******************************************************************************/
/*****temp code to manage prog_id and handle*****/

struct channel_handle g_chan_handle[GEN_CA_PVR_RSC_REC_NUM]=
{
    {
        .state = 0,
    },
#ifndef _C0700A_VSC_ENABLE_    
    {
        .state = 0,
    },
    {
        .state = 0,
    },
#endif    
};
//for Ali internal test
/*
static void gen_ca_change_key_monitor(void)
{
	UINT8 i=0;
	UINT rec_time = 0;
	for(i=0;i<GEN_CA_PVR_RSC_REC_NUM;i++)
	{
		if(g_chan_handle[i].state == 1)
		{
			rec_time = pvr_r_get_ms_time(g_chan_handle[i].handle);
			if((rec_time - g_chan_handle[i].last_time_change_key) > 60000)
			{
				g_chan_handle[i].last_time_change_key = rec_time;
				change_key(g_chan_handle[i].prog_id);
			}
		}
	}
}
*/
static void gen_ca_set_handle_prog_id(UINT32 prog_id , UINT32 handle)
{
	UINT8 i=0;
	for(i=0;i<GEN_CA_PVR_RSC_REC_NUM;i++)
	{
		if(g_chan_handle[i].state == 0)
		{
			g_chan_handle[i].state = 1;
			g_chan_handle[i].handle = handle;
			g_chan_handle[i].prog_id = prog_id;
			//g_chan_handle[i].last_time_change_key = 0;//for Ali internal test
			break;
		}
	}
}

static UINT32 gen_ca_get_handle_by_prog_id(UINT32 prog_id)
{
	UINT8 i=0;
	for(i=0;i<GEN_CA_PVR_RSC_REC_NUM;i++)
	{
		if(g_chan_handle[i].state == 1)
		{
			if(g_chan_handle[i].prog_id == prog_id)
			{
				return g_chan_handle[i].handle;
			}
		}
	}
    return 0;
}

static void gen_ca_clear_handle_by_prog_id(UINT32 prog_id)
{
	UINT8 i=0;
	for(i=0;i<GEN_CA_PVR_RSC_REC_NUM;i++)
	{
		if(g_chan_handle[i].state == 1)
		{
			if(g_chan_handle[i].prog_id == prog_id)
			{
				g_chan_handle[i].state = 0;
				g_chan_handle[i].handle = 0;
				g_chan_handle[i].prog_id = 0;
				//g_chan_handle[i].last_time_change_key = 0;//for Ali internal test
				break;
			}
		}
	}
}
/*****temp code to manage prog_id and handle*****/
static gen_ca_pvr_resource *gen_ca_pvr_resource_occupy(int rsc_type, UINT32 pvr_hnd,UINT32* p_rec_id)
{
    int i = -1;
    gen_ca_pvr_resource *rsc = NULL;

    if(NULL!=p_rec_id)
    {
        *p_rec_id=INVALID_REC_ID;     
    }
    
    gen_ca_pvr_lock();
    if (GEN_CA_PVR_RSC_TYPE_REC == rsc_type)
    {
        for (i = 0; i < GEN_CA_PVR_RSC_REC_NUM; ++i)
        {
            if (0 == g_cpm.rec[i].pvr_hnd)
            {
                rsc = &g_cpm.rec[i];
                if(NULL!=p_rec_id)
                {
                    *p_rec_id=i;
                }               
                rsc->pvr_hnd = pvr_hnd;
                break;
            }
        }
    }
    else if (GEN_CA_PVR_RSC_TYPE_PLAY == rsc_type)
    {
        for (i = 0; i < GEN_CA_PVR_RSC_PLAY_NUM; ++i)
        {
            if (0 == g_cpm.play[i].pvr_hnd)
            {
                rsc = &g_cpm.play[i];
                if(NULL!=p_rec_id)
                {
                    *p_rec_id=i;
                }                
                rsc->pvr_hnd = pvr_hnd;
                break;
            }
        }
    }
    gen_ca_pvr_un_lock();
    return rsc;
}

static gen_ca_pvr_resource *gen_ca_pvr_resource_find(int rsc_type, UINT32 pvr_hnd)
{
    int i = -1;
    gen_ca_pvr_resource *rsc = NULL;

    gen_ca_pvr_lock();
    if (GEN_CA_PVR_RSC_TYPE_REC == rsc_type)
    {
        for (i = 0; i < GEN_CA_PVR_RSC_REC_NUM; ++i)
        {
            if (g_cpm.rec[i].pvr_hnd == pvr_hnd)
            {
                rsc = &g_cpm.rec[i];
                break;
            }
        }
    }
    else if (GEN_CA_PVR_RSC_TYPE_PLAY == rsc_type)
    {
        for (i = 0; i < GEN_CA_PVR_RSC_PLAY_NUM; ++i)
        {
            if (g_cpm.play[i].pvr_hnd == pvr_hnd)
            {
                rsc = &g_cpm.play[i];
                break;
            }
        }
    }
    gen_ca_pvr_un_lock();
    return rsc;
}

static int gen_ca_pvr_resource_release(gen_ca_pvr_resource *rsc)
{
    if(NULL == rsc)
    {
        return -1;
    }
    gen_ca_pvr_lock();
    rsc->pvr_hnd = 0;
    rsc->crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
    rsc->crypto_stream_id = INVALID_CRYPTO_STREAM_ID;
    rsc->crypto_key_pos = INVALID_CE_KEY_POS;
	
    rsc->rec_id=INVALID_REC_ID;   
    rsc->aes_id=INVALID_DSC_SUB_DEV_ID;
    rsc->dec_stream_id=INVALID_CRYPTO_STREAM_ID;     

    gen_ca_pvr_un_lock();
    return 0;
}

static void _gen_ca_pvr_rec_config_free_resource(UINT32 aes_id,UINT32 encrypt_stream_id)
{
    if(INVALID_DSC_SUB_DEV_ID != aes_id)
	dsc_set_sub_device_id_idle(AES,aes_id);
	
    if(INVALID_CRYPTO_STREAM_ID != encrypt_stream_id)
       dsc_set_stream_id_idle(encrypt_stream_id);

}
//get record or play pid list
UINT16 get_dsc_pid_multi_des(UINT16 *pid_list, UINT16 pid_number,UINT32 prog_id)   
{        
	P_NODE tmp_node; 
	UINT16 i = 0;
   	 UINT16 j=0;  
	UINT16 ref_pids[MAX_DSC_STREAM]; 
	if(pid_number>MAX_DSC_STREAM)    
	{           
		pid_number = MAX_DSC_STREAM;    
	}        
	MEMSET(&tmp_node, 0, sizeof(P_NODE));   
	get_prog_by_id(prog_id, &tmp_node);       
	ref_pids[0] = tmp_node.video_pid;   
	for(i=1,j=0;(i<MAX_DSC_STREAM && j<tmp_node.audio_count);i++, j++)   
	{        
		ref_pids[i] = tmp_node.audio_pid[j];    
	}   
	if(i<MAX_DSC_STREAM)       
	{          
		ref_pids[i++] = tmp_node.subtitle_pid;  
	}  
	if(i<MAX_DSC_STREAM)    
	{                
		ref_pids[i++] = tmp_node.teletext_pid;  
	}        
	MEMCPY(pid_list, ref_pids, pid_number*sizeof(UINT16));  
	return pid_number;
}

static BOOL gen_ca_check_channel_is_recording(void)
{
	pvr_play_rec_t  *pvr_info = NULL;
	pvr_record_t   *rec      = NULL;
	
	pvr_info  = api_get_pvr_info();
	rec = api_pvr_get_rec_by_prog_id(pvr_info->play.play_chan_id, NULL);
	if ((rec != NULL) || (pvr_info->tms_chan_id == pvr_info->play.play_chan_id))
    	{
        	return TRUE;
    	}
    	return FALSE;
}

static void gen_ca_pvr_replay_monitor(void)
{
    return;
    
	if(gen_ca_trackReplay.playing == 1)
	{
		UINT32 play_time = 0;
		UINT32 set_time = 0;
		UINT32 key_level = 0;
		PVR_STATE play_state;

		play_state = pvr_p_get_state(gen_ca_trackReplay.pvr_hnd);
        	if((play_state==NV_FB) || (play_state==NV_REVSLOW))
        	{
            		if(gen_ca_trackReplay.state==0)
            		{
                		gen_ca_trackReplay.state=1;
                		gen_ca_trackReplay.nextSetted=FALSE;
            		}
        	}
        	else
        	{
            		if(gen_ca_trackReplay.state==1)
            		{
                		gen_ca_trackReplay.state=0;
                		gen_ca_trackReplay.nextSetted=FALSE;
            		}
        	}
        	play_time = pvr_p_get_ms_time(gen_ca_trackReplay.pvr_hnd);

		if(gen_ca_trackReplay.state==0)//normal
        	{
        		if((TRUE == gen_ca_check_channel_is_recording()) && (FALSE == gen_ca_trackReplay.nextSetted))
        		{
        			struct gen_ca_key_info_data *key_info_data = NULL;
				key_info_data = (struct gen_ca_key_info_data *)MALLOC(sizeof(struct gen_ca_key_info_data));
				MEMSET(key_info_data,0,sizeof(struct gen_ca_key_info_data));
        			if(TRUE == pvr_get_key_info(gen_ca_trackReplay.pvr_hnd,key_info_data,play_time))
        			{
        				gen_ca_trackReplay.storeInfoBegin = key_info_data->key_info_pre.ptm;
	                     	gen_ca_trackReplay.storeInfoEnd = key_info_data->key_info_next.ptm;
        			}
				else
				{
					GEN_CA_PVR_ERROR("%s() : pvr_get_key_info fail!\n", __FUNCTION__);
				}
				FREE(key_info_data);
        		}
        		if(gen_ca_trackReplay.storeInfoEnd != 0)//not last key
        		{
	        		if(gen_ca_trackReplay.storeInfoEnd-gen_ca_trackReplay.storeInfoBegin > 2000)
	        		{
	                     	set_time = gen_ca_trackReplay.storeInfoEnd - 2000; // 2 sec
	        		}
	                    else
	                    	{
	                        	set_time = (gen_ca_trackReplay.storeInfoBegin + gen_ca_trackReplay.storeInfoEnd) / 2;
	                    	}

				if((FALSE == gen_ca_trackReplay.nextSetted) &&(play_time >= set_time))
	                	{
	                		struct gen_ca_key_info_data *key_info_data = NULL;
					key_info_data = (struct gen_ca_key_info_data *)MALLOC(sizeof(struct gen_ca_key_info_data));
					MEMSET(key_info_data,0,sizeof(struct gen_ca_key_info_data));
					key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);
					if(TRUE == pvr_get_key_info(gen_ca_trackReplay.pvr_hnd,key_info_data,play_time))
					{
						if(0 == key_info_data->key_info_next.key_type)
						{
							if(FALSE == gen_ca_p_set_pvr_key(&(key_info_data->key_info_next),gen_ca_trackReplay.key_pos,\
																	THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))
							{
								GEN_CA_PVR_ERROR("%s() : gen_ca_p_set_pvr_key fail!\n", __FUNCTION__);
							}
						}
						else
						{
							if(FALSE == gen_ca_p_set_pvr_key(&(key_info_data->key_info_next),gen_ca_trackReplay.key_pos+1,\
																THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))
							{
								GEN_CA_PVR_ERROR("%s() : gen_ca_p_set_pvr_key fail!\n", __FUNCTION__);
							}
						}
						gen_ca_trackReplay.nextSetted = TRUE;
					}
					else
					{
						GEN_CA_PVR_ERROR("%s() : pvr_get_key_info fail!\n", __FUNCTION__);
					}
					
					FREE(key_info_data);
				}

				if(play_time > gen_ca_trackReplay.storeInfoEnd)
				{
					struct gen_ca_key_info_data *key_info_data = NULL;

					key_info_data = (struct gen_ca_key_info_data *)MALLOC(sizeof(struct gen_ca_key_info_data));
					MEMSET(key_info_data,0,sizeof(struct gen_ca_key_info_data));
	                    		if(TRUE == pvr_get_key_info(gen_ca_trackReplay.pvr_hnd,key_info_data,play_time))
					{
	                        		gen_ca_trackReplay.storeInfoBegin = key_info_data->key_info_pre.ptm;
	                        		gen_ca_trackReplay.storeInfoEnd = key_info_data->key_info_next.ptm;
	                        		gen_ca_trackReplay.nextSetted = FALSE;
	                   	 	}
	                    		else
	                    		{
	                       		 GEN_CA_PVR_ERROR("%s() : pvr_get_key_info fail!\n", __FUNCTION__);
	                    		}

	                    		FREE(key_info_data);
				}
        		}
		}
		else		//backward play
		{
			if((0 != gen_ca_trackReplay.storeInfoBegin) \
				&& (play_time < (gen_ca_trackReplay.storeInfoBegin + gen_ca_trackReplay.storeInfoEnd)/2))
			{
				struct gen_ca_key_info_data *key_info_data = NULL;

				key_info_data = (struct gen_ca_key_info_data *)MALLOC(sizeof(struct gen_ca_key_info_data));
				MEMSET(key_info_data,0,sizeof(struct gen_ca_key_info_data));
				key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);
				if(TRUE == pvr_get_key_info(gen_ca_trackReplay.pvr_hnd,key_info_data,gen_ca_trackReplay.storeInfoBegin - 1))
				{
					if(0 == key_info_data->key_info_pre.key_type)
					{
						if(FALSE == gen_ca_p_set_pvr_key(&(key_info_data->key_info_pre),gen_ca_trackReplay.key_pos,\
															THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))
						{
								GEN_CA_PVR_ERROR("%s() : gen_ca_p_set_pvr_key fail!\n", __FUNCTION__);
						}
					}
					else
					{
						if(FALSE == gen_ca_p_set_pvr_key(&(key_info_data->key_info_pre),gen_ca_trackReplay.key_pos+1,\
															THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS))
						{
							GEN_CA_PVR_ERROR("%s() : gen_ca_p_set_pvr_key fail!\n", __FUNCTION__);
						}
					}
					gen_ca_trackReplay.storeInfoBegin = key_info_data->key_info_pre.ptm;
	                        	gen_ca_trackReplay.storeInfoEnd = key_info_data->key_info_next.ptm;
	                        	gen_ca_trackReplay.nextSetted = FALSE;
				}
				else
				{
					GEN_CA_PVR_ERROR("%s() : pvr_get_key_info fail!\n", __FUNCTION__);
				}

				FREE(key_info_data);
			}
		}
	}
	else if(gen_ca_trackReplay.playing == 2)	//jump
	{
		UINT32 play_time = 0;
		#define CHECK_INTERVAL    5*1000    // five seconds
		play_time = pvr_p_get_ms_time(gen_ca_trackReplay.pvr_hnd);
		if(play_time<(gen_ca_trackReplay.jump_time+CHECK_INTERVAL))
        	{
            		gen_ca_trackReplay.playing=1;
            		if(!get_mute_state())
            		{
                		api_audio_set_mute(FALSE);    //set un-mute
            		}
        	}
        	else
        	{
            		GEN_CA_PVR_DEBUG("wait jump complete play_time=%d check_time=%d\n",play_time,(gen_ca_trackReplay.jump_time+CHECK_INTERVAL));
        	}
	}
}

BOOL gen_ca_pvr_replay_time_search(UINT32 time)
{
    return FALSE;
    
	struct gen_ca_key_info_data *key_info_data = NULL;
	UINT32 play_time = time*1000;	//ms
	UINT32 key_level = 0;

	key_info_data = (struct gen_ca_key_info_data *)MALLOC(sizeof(struct gen_ca_key_info_data));
	MEMSET(key_info_data,0,sizeof(struct gen_ca_key_info_data));
	key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);
	if(TRUE == pvr_get_key_info(gen_ca_trackReplay.pvr_hnd,key_info_data,play_time))
	{
		if(0 == key_info_data->key_info_pre.key_type)
			gen_ca_p_set_pvr_key(&(key_info_data->key_info_pre),gen_ca_trackReplay.key_pos,\
										THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS);
		else
			gen_ca_p_set_pvr_key(&(key_info_data->key_info_pre),gen_ca_trackReplay.key_pos+1,\
										THREE_LEVEL==key_level? g_pvr_level2pos: GEN_CA_PVR_KEY_POS);

		gen_ca_trackReplay.playing = 2;
		gen_ca_trackReplay.storeInfoBegin = key_info_data->key_info_pre.ptm;
	       gen_ca_trackReplay.storeInfoEnd = key_info_data->key_info_next.ptm;
	       gen_ca_trackReplay.nextSetted = FALSE;
		gen_ca_trackReplay.state = 0;
		gen_ca_trackReplay.jump_time = play_time;
	}
	else
	{
		GEN_CA_PVR_ERROR("%s() : get key info from chunk fail!\n", __FUNCTION__);
	}

	FREE(key_info_data);
    return TRUE;
}

BOOL gen_ca_pvr_set_key_info(UINT32 prog_id , UINT8 *keydata , struct gen_ca_crypto_info crypto_info)
{
	UINT32 rec_time=0;
	RET_CODE ret = RET_FAILURE;
	DSC_EN_PRIORITY priority_param;
	DSC_DEV *p_dsc_dev0 = NULL;
	gen_ca_key_info key_Info;
	UINT32 handle = 0;
	p_dsc_dev0 = (DSC_DEV *) dev_get_by_id(HLD_DEV_TYPE_DSC, 0);

	handle = gen_ca_get_handle_by_prog_id(prog_id);
	
	rec_time = pvr_r_get_ms_time(handle);
    	priority_param.sub_module = crypto_info.crypto_mode;
    	// Even key
    	if (0 == crypto_info.key_type)
       	priority_param.priority = EVEN_PARITY_MODE;
    	// Odd key
    	else
       	priority_param.priority = ODD_PARITY_MODE;

       priority_param.dev_ptr = (UINT32)dev_get_by_id(HLD_DEV_TYPE_AES, crypto_info.sub_device_id);
   	ret = dsc_ioctl(p_dsc_dev0,IO_DSC_SET_ENCRYPT_PRIORITY,(UINT32)&priority_param);
	if(ret != RET_SUCCESS)
    	{
        	GEN_CA_PVR_ERROR("%s : DSC_SET_ENCRYPT_PRIORITY Fail\n", __FUNCTION__);		
		return FALSE;
    	}

	key_Info.key_index = crypto_info.gen_ca_key_cnt;
    	key_Info.key_len= crypto_info.key_len;
    	key_Info.key_type = crypto_info.key_type;
    	MEMCPY(key_Info.key_data,keydata,crypto_info.key_len);
    	key_Info.ptm= rec_time;
	ret = pvr_save_key_info(handle,&key_Info);
	if(ret != RET_SUCCESS)
    	{
        	GEN_CA_PVR_ERROR("%s : pvr_save_key_info Fail\n", __FUNCTION__);
		return FALSE;
    	}
	return TRUE;
}

// configure crypto for re-encrypt ts, and encrypt key
int gen_ca_pvr_rec_config(pvr_crypto_general_param *rec_param)
{
    	UINT32 prog_id = 0;  
    	UINT32 decrypt_stream_id = INVALID_CRYPTO_STREAM_ID;
    	UINT32 encrypt_stream_id = INVALID_CRYPTO_STREAM_ID;
    	p_csa_dev pvr_csa_dev = NULL;
    	p_aes_dev pvr_aes_dev = NULL;
    	struct dmx_device *dmx = NULL;
    	UINT32 aes_id=INVALID_DSC_SUB_DEV_ID;  
    	UINT32 rec_id=INVALID_REC_ID;  
	UINT32 key_level = 0;
	RET_CODE ret = RET_FAILURE;    

    	if(NULL == rec_param)
    	{
        	return -1;
    	}

    	dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, rec_param->dmx_id);
    	prog_id = pvr_r_get_channel_id(rec_param->pvr_hnd);

/***********get free re-encrypt source***********/	
	// step 1. get a free encrypt stream ID
    	encrypt_stream_id = dsc_get_free_stream_id(TS_MODE);
 	if (INVALID_CRYPTO_STREAM_ID == encrypt_stream_id)
 	{
		GEN_CA_PVR_ERROR("Invalid encrypt stream id: %d\n", encrypt_stream_id);
       	return -1;
    	}	
	// step 2. get a free AES device ID  	
    	aes_id = dsc_get_free_sub_device_id(AES);
    	if (INVALID_DSC_SUB_DEV_ID == aes_id)
    	{
       	GEN_CA_PVR_ERROR ("%s : dsc_get_free_sub_device_id fail\n", __FUNCTION__);
		_gen_ca_pvr_rec_config_free_resource(aes_id,encrypt_stream_id);
		return -1;
    	}
	// step 3. get the AES device by ID	
    	pvr_aes_dev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, aes_id);

/***********get free decrypt source***********/
#ifdef _C0700A_VSC_ENABLE_
    decrypt_stream_id = 0;
#else
	//Ben 180619#2
   	//decrypt_stream_id = dsc_get_free_stream_id(TS_MODE);
	//libc_printf("1.decrypt_stream_id[%d] \n",decrypt_stream_id);
	UINT32 ga_stream_id = get_ca_stream_id();
	decrypt_stream_id = ga_stream_id;
	//libc_printf("2.decrypt_stream_id[%d] \n",decrypt_stream_id);
	//
#endif
	if (INVALID_CRYPTO_STREAM_ID==decrypt_stream_id)
       {
            GEN_CA_PVR_ERROR("No free crypto stream id\n");
            return -1;
       }
   	pvr_csa_dev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);
/***********get free record source***********/	
    	gen_ca_pvr_resource *rec_rsc = NULL; 
    	rec_rsc = gen_ca_pvr_resource_occupy(GEN_CA_PVR_RSC_TYPE_REC, rec_param->pvr_hnd,&rec_id);
    	if(INVALID_REC_ID==rec_id)
    	{       
        	GEN_CA_PVR_ERROR("Without valid rec_id\n");
		_gen_ca_pvr_rec_config_free_resource(aes_id,encrypt_stream_id);
        #ifndef _C0700A_VSC_ENABLE_
		dsc_set_stream_id_idle(decrypt_stream_id);
        #endif
    	    	return -1;
    	} 
    
    	if (NULL == rec_rsc)
    	{
       	 GEN_CA_PVR_ERROR("No free PVR record resource\n");   
		_gen_ca_pvr_rec_config_free_resource(aes_id,encrypt_stream_id);
        #ifndef _C0700A_VSC_ENABLE_
		dsc_set_stream_id_idle(decrypt_stream_id);
        #endif
    	    	return -1;
    	}
/***********set record parameter***********/	
    	rec_rsc->rec_id=rec_id;
    	rec_rsc->aes_id=aes_id;   
    	rec_rsc->dec_stream_id=decrypt_stream_id;
    	rec_rsc->crypto_stream_id = encrypt_stream_id;
/***********find free key position***********/
	otp_init(NULL);
    	key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);

    	CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,0};
	p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    	MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));

    	if( THREE_LEVEL > key_level)
    	{
       	key_pos_param.ce_key_level = GEN_CA_CRYPTO_KEY_LEVEL;
    	}
    	else
    	{
        	key_pos_param.ce_key_level = THREE_LEVEL;
    	}
        #if 1//def _C0700A_VSC_ENABLE_	//Ben 180727#1
        key_pos_param.number = 1;
        #else
    	key_pos_param.number = 2;
        #endif
    	key_pos_param.pos = INVALID_CE_KEY_POS;
    	key_pos_param.root = GEN_CA_M2M2_KEY_POS;
    	ret = ce_ioctl(p_ce_dev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
    	if (RET_SUCCESS != ret)
    	{
       	GEN_CA_PVR_ERROR("find free key pos fail!\n");
		_gen_ca_pvr_rec_config_free_resource(aes_id,encrypt_stream_id); 
        #ifndef _C0700A_VSC_ENABLE_
		dsc_set_stream_id_idle(decrypt_stream_id);
        #endif
        	gen_ca_pvr_resource_release(rec_rsc);
        	return -1;
    	}
	
/***********set recording key***********/
	UINT8 *key_data = NULL;
    struct gen_ca_pvr_crypto_param crypto_param;
    
	key_data = (UINT8 *)MALLOC(GEN_CA_KEY_LEN_MAX);
	MEMSET(key_data,0,GEN_CA_KEY_LEN_MAX);
	crypto_param.crypto_key_pos = key_pos_param.pos;
	crypto_param.pid_list= rec_param->pid_list;
	crypto_param.pid_num= rec_param->pid_num;
	crypto_param.stream_id = encrypt_stream_id;
	crypto_param.crypto_dev_id = aes_id;
	crypto_param.prog_id = prog_id;
	rec_rsc->crypto_stream_hnd = gen_ca_r_set_pvr_crypto_para(&crypto_param,key_data);

	if (INVALID_CRYPTO_STREAM_HANDLE == rec_rsc->crypto_stream_hnd)
    	{          
        	GEN_CA_PVR_ERROR("%s() failed! encrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
           	 __FUNCTION__, rec_rsc->crypto_stream_hnd, rec_rsc->crypto_stream_id,
            	rec_param->crypto_key_pos);
        	_gen_ca_pvr_rec_config_free_resource(aes_id,encrypt_stream_id); 
         #ifndef _C0700A_VSC_ENABLE_    
		dsc_set_stream_id_idle(decrypt_stream_id);
         #endif
        	gen_ca_pvr_resource_release(rec_rsc);
		ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos+1);
		FREE(key_data);
        	return -1;
    	}
	rec_rsc->crypto_key_pos = key_pos_param.pos;
/***********save recording key into chunk***********/	
	gen_ca_set_handle_prog_id(prog_id , rec_param->pvr_hnd);
    #if 0
	crypto_info.gen_ca_key_cnt= 0;
	crypto_info.key_len = 16;
	crypto_info.key_type= EVEN_PARITY_MODE;
	crypto_info.crypto_mode = AES;
	crypto_info.sub_device_id = aes_id;
	struct gen_ca_crypto_info crypto_info;    
	if(FALSE == gen_ca_pvr_set_key_info(prog_id,key_data,crypto_info))
	{          
        	GEN_CA_PVR_ERROR("%s() : gen_ca_pvr_save_key failed!\n", __FUNCTION__);
        	_gen_ca_pvr_rec_config_free_resource(aes_id,encrypt_stream_id); 
         #ifndef _C0700A_VSC_ENABLE_    
		dsc_set_stream_id_idle(decrypt_stream_id);
         #endif
        	gen_ca_pvr_resource_release(rec_rsc);
		ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
		ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos+1);
		FREE(key_data);		
		gen_ca_clear_handle_by_prog_id(prog_id);
        	return -1;
    	}
    #endif
	FREE(key_data);

    	timeshift_stream_id = rec_rsc->crypto_stream_id;
/***********set path between dmx to dsc***********/
    	pvr_de_enconfig[(UINT8)rec_id].do_decrypt = 1;
    	pvr_de_enconfig[(UINT8)rec_id].dec_dev = pvr_csa_dev;
    	pvr_de_enconfig[(UINT8)rec_id].decrypt_mode = CSA;
    	pvr_de_enconfig[(UINT8)rec_id].dec_dmx_id = rec_rsc->dec_stream_id;

    	pvr_de_enconfig[(UINT8)rec_id].do_encrypt = 1;
    	pvr_de_enconfig[(UINT8)rec_id].enc_dev = pvr_aes_dev;
    	pvr_de_enconfig[(UINT8)rec_id].encrypt_mode = AES;
    	pvr_de_enconfig[(UINT8)rec_id].enc_dmx_id = rec_rsc->crypto_stream_id;

    	dmx_io_control(dmx, IO_SET_DEC_CONFIG, (UINT32)&pvr_de_enconfig[(UINT8)rec_id]);

    	/* set other return values */
    	rec_param->crypto_stream_hnd = rec_rsc->crypto_stream_hnd;
    	rec_param->crypto_stream_id = rec_rsc->crypto_stream_id;

    libc_printf("gen_ca_pvr_rec_config: dec_stream_id %d, enc_stream_id %d, crypto_key_pos 0x%x\n", 
        rec_rsc->dec_stream_id, rec_rsc->crypto_stream_id, rec_rsc->crypto_key_pos);
    	return 0;
}

// decrypt key and configure crypto for decrypt ts
int gen_ca_pvr_playback_config(pvr_crypto_general_param *play_param,INT8 timeshift_flag)
{
    	struct dmx_device *dmx =NULL;
    	gen_ca_pvr_resource *play_rsc = NULL;
    	struct dec_parse_param p_param={NULL,0};
	UINT32 key_level = 0;
    	RET_CODE ret = RET_FAILURE;  	

    	if(NULL == play_param)
    	{
        	return -1;
    	}
    	dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, play_param->dmx_id);

/***********get key info from chunk***********/
	UINT32 __MAYBE_UNUSED__ play_time = 0;
	struct gen_ca_key_info_data *key_info_data;

	key_info_data = (struct gen_ca_key_info_data *)MALLOC(sizeof(struct gen_ca_key_info_data));
	MEMSET(key_info_data,0,sizeof(struct gen_ca_key_info_data));
	
	play_time = pvr_p_get_ms_time(play_param->pvr_hnd);
    #if 0
	if(FALSE == pvr_get_key_info(play_param->pvr_hnd,key_info_data,play_time))
	{
		GEN_CA_PVR_ERROR("%s() : pvr_get_key_info failed!\n", __FUNCTION__);
		FREE(key_info_data);
		return -1;
	}
    #endif
/***********get free decrypt source(AES)***********/
    	if(NULL == playback_aes_dev)
    	{
       	if (INVALID_DSC_SUB_DEV_ID != (aes_dev_id = dsc_get_free_sub_device_id(AES)))
        	{
           		 playback_aes_dev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, aes_dev_id);
        	}
        	else
        	{
            		GEN_CA_PVR_ERROR("no free AES device\n");
			FREE(key_info_data);
            		return -1;
        	}
    	}
/***********get free playback source***********/
    	play_rsc = gen_ca_pvr_resource_occupy(GEN_CA_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd,NULL);
    	if (NULL == play_rsc)
    	{
        	GEN_CA_PVR_ERROR("No free PVR play resource\n");
        	dsc_set_sub_device_id_idle(AES, aes_dev_id);
		FREE(key_info_data);
        	return -1;
    	}

    	if((ALI_C3701 <= sys_ic_get_chip_id()) || (0 == timeshift_flag) || (1 == sys_data_get_rec_num()))
    	{
        	play_rsc->crypto_stream_id = dsc_get_free_stream_id(TS_MODE);
        	if (INVALID_CRYPTO_STREAM_ID == play_rsc->crypto_stream_id)
        	{
           		GEN_CA_PVR_ERROR("No free crypto stream id\n");
           		dsc_set_sub_device_id_idle(AES, aes_dev_id);
           		gen_ca_pvr_resource_release(play_rsc);
			FREE(key_info_data);
           		return -1;
        	}
    	}
    	else
        	play_rsc->crypto_stream_id = timeshift_stream_id;	
/***********find free key position***********/
	key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);

    	CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,0};
	p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    	MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));

    	if( THREE_LEVEL > key_level)
    	{
        	key_pos_param.ce_key_level = GEN_CA_CRYPTO_KEY_LEVEL;
    	}
    	else
    	{
        	key_pos_param.ce_key_level = THREE_LEVEL;
    	}
        #if 1//def _C0700A_VSC_ENABLE_	//Ben 180727#1
        key_pos_param.number = 1;
        #else
    	key_pos_param.number = 2;
        #endif
    	key_pos_param.pos = INVALID_CE_KEY_POS;
    	key_pos_param.root = GEN_CA_M2M2_KEY_POS;
    	ret = ce_ioctl(p_ce_dev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
    	if (RET_SUCCESS != ret)
    	{
        	GEN_CA_PVR_ERROR("find free key pos fail!\n");
		dsc_set_sub_device_id_idle(AES, aes_dev_id);
           	gen_ca_pvr_resource_release(play_rsc);
		FREE(key_info_data);
        	return INVALID_CRYPTO_STREAM_HANDLE;
    	}

/***********set playback decrypt parameter***********/
	struct gen_ca_pvr_crypto_param crypto_param;

	crypto_param.crypto_key_pos = key_pos_param.pos;
	crypto_param.pid_list= play_param->pid_list;
	crypto_param.pid_num= play_param->pid_num;
	crypto_param.stream_id = play_rsc->crypto_stream_id;
	crypto_param.crypto_dev_id = aes_dev_id;
    	play_rsc->crypto_stream_hnd = \
        	gen_ca_p_set_pvr_crypto_para(&crypto_param,&(key_info_data->key_info_pre));

    	if (INVALID_CRYPTO_STREAM_HANDLE == play_rsc->crypto_stream_hnd)
    	{
        	GEN_CA_PVR_ERROR("%s() failed! decrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
            	__FUNCTION__, play_rsc->crypto_stream_hnd, play_rsc->crypto_stream_id,
            	play_rsc->crypto_key_pos);
        	dsc_set_sub_device_id_idle(AES, aes_dev_id);
	 	dsc_set_stream_id_idle(play_rsc->crypto_stream_id);
        	gen_ca_pvr_resource_release(play_rsc);
		ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos);
	 	ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, key_pos_param.pos+1);
	 	FREE(key_info_data);
        	return -1;
    	}
    	play_rsc->crypto_key_pos = key_pos_param.pos;
/***********configure dsc crypto for playback***********/
    	MEMSET(&p_param, 0, sizeof(struct dec_parse_param));
    	p_param.dec_dev = playback_aes_dev;
    	p_param.type = AES;
    	dmx_io_control(dmx, IO_SET_DEC_HANDLE, (UINT32)&p_param);
    	dmx_io_control(dmx, IO_SET_DEC_STATUS, 1);
    	dsc_ioctl(pvr_dsc_dev, IO_PARSE_DMX_ID_SET_CMD, play_rsc->crypto_stream_id);

    	/* set other return values */
    	play_param->crypto_stream_hnd = play_rsc->crypto_stream_hnd;
    	play_param->crypto_stream_id = play_rsc->crypto_stream_id;

    	gen_ca_trackReplay.storeInfoBegin = key_info_data->key_info_pre.ptm;
    	gen_ca_trackReplay.storeInfoEnd = key_info_data->key_info_next.ptm;
    	gen_ca_trackReplay.nextSetted = FALSE;
    	gen_ca_trackReplay.state = 0;
    	gen_ca_trackReplay.pvr_hnd = play_param->pvr_hnd;
    	gen_ca_trackReplay.key_pos = key_pos_param.pos;
    	gen_ca_trackReplay.jump_time=0;
    	gen_ca_trackReplay.playing=1;
    	FREE(key_info_data);
	libc_printf("gen_ca_pvr_playback_config: crypto_stream_id %d, crypto_key_pos 0x%x\n", play_rsc->crypto_stream_id, play_rsc->crypto_key_pos);
    	return 0;
}

void gen_ca_set_dsc_for_live_play(UINT16 dmx_id, UINT32 stream_id)
{
    p_csa_dev csa_dev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
    struct dec_parse_param param={NULL,0};

    GEN_CA_PVR_DEBUG("%s() dmx_id: %d, CSA stream_id: %d\n", __FUNCTION__, dmx_id, stream_id);

	//Ben 180727#1
	UINT32 ga_stream_id = get_ca_stream_id();
	GEN_CA_PVR_DEBUG("%s() dmx_id: %d, CSA stream_id: %d\n", __FUNCTION__, dmx_id, stream_id);
	//
    MEMSET(&param, 0, sizeof(struct dec_parse_param));
    param.dec_dev = csa_dev;
    param.type = CSA;
    dmx_io_control(dmx, IO_SET_DEC_HANDLE, (UINT32)&param);
    dmx_io_control(dmx, IO_SET_DEC_STATUS, 1);
    dsc_ioctl(pvr_dsc_dev, IO_PARSE_DMX_ID_SET_CMD, ga_stream_id);	//Ben 180727#1
}

// reset crypto for FTA TS recording
static int gen_ca_pvr_reset_rec_config(UINT16 dmx_id,UINT8 index)
{
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);

    pvr_de_enconfig[index].do_decrypt = 0; // for FTA, no need to open decrypt device
    pvr_de_enconfig[index].dec_dev = NULL;
    pvr_de_enconfig[index].do_encrypt = 0;
    pvr_de_enconfig[index].enc_dev = NULL;

    dmx_io_control(dmx, IO_SET_DEC_CONFIG, (UINT32)&pvr_de_enconfig[index]);
    return 0;
}

// when stop record, need delete encrypt stream.
int gen_ca_pvr_rec_stop(pvr_crypto_general_param *rec_param)
{
    RET_CODE ret = RET_FAILURE;
    gen_ca_pvr_resource *rec_rsc = NULL;
    p_aes_dev pvr_aes_dev  = NULL;
    __MAYBE_UNUSED__ UINT32 decrypt_stream_id = INVALID_CRYPTO_STREAM_ID;
    UINT32 encrypt_stream_id = INVALID_CRYPTO_STREAM_ID;
    UINT32 rec_id=INVALID_REC_ID;
    UINT32 aes_id=INVALID_DSC_SUB_DEV_ID;

    if(NULL == rec_param)
    {
        return -1;
    }

    UINT32 prog_id;
    prog_id = pvr_r_get_channel_id(rec_param->pvr_hnd);
    gen_ca_clear_handle_by_prog_id(prog_id);
    gen_ca_clear_crypto_info(prog_id);
	
    rec_rsc = gen_ca_pvr_resource_find(GEN_CA_PVR_RSC_TYPE_REC, rec_param->pvr_hnd);
    if (NULL == rec_rsc)
    {
        GEN_CA_PVR_ERROR("Cannot find record crypto stream resource, pvr handle: 0x%X\n", rec_param->pvr_hnd);
        return -1;
    }

    rec_id=rec_rsc->rec_id;
    aes_id=rec_rsc->aes_id;
#ifndef  __DECRYPT_STREAM_ID_FIX__	
    decrypt_stream_id=rec_rsc->dec_stream_id;
#endif
    encrypt_stream_id=rec_rsc->crypto_stream_id;
	
    if((rec_id>=MAX_PVR_DE_ENCONFIG) || (INVALID_REC_ID==rec_id))
    {
        GEN_CA_PVR_ERROR("Invalid rec_id(%d) out of range(0~2).\n", rec_id);
        return -1;
    }
        
    if(INVALID_DSC_SUB_DEV_ID==aes_id)
    {
        GEN_CA_PVR_ERROR("Invalid aes_id_id(%d)\n", aes_id);
        return -1;
    }

    pvr_aes_dev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, aes_id);

    GEN_CA_PVR_DEBUG("%s()\n\tpvr handle: 0x%X, dmx_id: %d\n"
        "\tencrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
        __FUNCTION__, rec_param->pvr_hnd, rec_param->dmx_id,
        rec_rsc->crypto_stream_hnd, rec_rsc->crypto_stream_id, rec_rsc->crypto_key_pos);

    if (INVALID_CRYPTO_STREAM_HANDLE != rec_rsc->crypto_stream_hnd)
    {
        ret = aes_ioctl(pvr_aes_dev ,IO_DELETE_CRYPT_STREAM_CMD, rec_rsc->crypto_stream_hnd);
    }

    if (INVALID_CE_KEY_POS != rec_rsc->crypto_key_pos || INVALID_CE_KEY_POS != rec_rsc->crypto_key_pos+1)
    {
        ce_ioctl(pvr_ce_dev, IO_CRYPT_POS_SET_IDLE, rec_rsc->crypto_key_pos);
	 ce_ioctl(pvr_ce_dev, IO_CRYPT_POS_SET_IDLE, rec_rsc->crypto_key_pos+1);
    }
	
    if (INVALID_CRYPTO_STREAM_ID != rec_rsc->crypto_stream_id)
    {
        dsc_ioctl(pvr_dsc_dev ,IO_DSC_SET_PVR_KEY_IDLE, rec_rsc->crypto_stream_id);
    }

    #if 0//ndef _C0700A_VSC_ENABLE_ //Ben 180727#1
    if (INVALID_CRYPTO_STREAM_ID != rec_rsc->dec_stream_id)
    {
	    dsc_set_stream_id_idle(decrypt_stream_id);
    }
    #endif
    _gen_ca_pvr_rec_config_free_resource(aes_id,encrypt_stream_id);

    gen_ca_pvr_resource_release(rec_rsc);

    gen_ca_pvr_reset_rec_config(rec_param->dmx_id,rec_id);

    return (RET_SUCCESS == ret) ? 0 : -1;
}

int gen_ca_pvr_playback_stop(pvr_crypto_general_param *play_param)
{
    RET_CODE ret = RET_FAILURE;

    gen_ca_pvr_resource *play_rsc = NULL;

    if(NULL == play_param)
    {
        return -1;
    }

	gen_ca_trackReplay.playing = 0;
	gen_ca_trackReplay.nextSetted = FALSE;
	gen_ca_trackReplay.state = 0;
	gen_ca_trackReplay.storeInfoBegin = 0;
	gen_ca_trackReplay.storeInfoEnd = 0;
	gen_ca_trackReplay.pvr_hnd = 0;
	gen_ca_trackReplay.key_pos = 0;
	gen_ca_trackReplay.jump_time = 0;
    play_rsc = gen_ca_pvr_resource_find(GEN_CA_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
    if (NULL == play_rsc)
    {
        GEN_CA_PVR_ERROR("Cannot find play crypto stream resource, pvr handle: 0x%X\n", play_param->pvr_hnd);
        return -1;
    }

    GEN_CA_PVR_DEBUG("%s()\n\tpvr handle: 0x%X, dmx_id: %d\n"
        "\tdecrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
        __FUNCTION__, play_param->pvr_hnd, play_param->dmx_id,
        play_rsc->crypto_stream_hnd, play_rsc->crypto_stream_id, play_rsc->crypto_key_pos);

    if (INVALID_CRYPTO_STREAM_HANDLE != play_rsc->crypto_stream_hnd)
    {
        ret = aes_ioctl(playback_aes_dev ,IO_DELETE_CRYPT_STREAM_CMD, play_rsc->crypto_stream_hnd);
        dsc_set_sub_device_id_idle(AES, aes_dev_id);
        aes_dev_id = INVALID_DSC_SUB_DEV_ID;
        playback_aes_dev = NULL;
    }
    if (INVALID_CE_KEY_POS != play_rsc->crypto_key_pos || INVALID_CE_KEY_POS != play_rsc->crypto_key_pos+1)
    {
        ce_ioctl(pvr_ce_dev, IO_CRYPT_POS_SET_IDLE, play_rsc->crypto_key_pos);
	 ce_ioctl(pvr_ce_dev, IO_CRYPT_POS_SET_IDLE, play_rsc->crypto_key_pos+1);
    }

    if (INVALID_CRYPTO_STREAM_ID != play_rsc->crypto_stream_id)
    {
        dsc_set_stream_id_idle(play_rsc->crypto_stream_id);
    }

    gen_ca_pvr_resource_release(play_rsc);

    return (RET_SUCCESS == ret) ? 0 : -1;
}


static void gen_ca_main_task(void)
{
	UINT32 dwTimeStamp = 0;
	while(1)
	{
		if( (osal_get_tick() - dwTimeStamp) > 250 )
	      	{
	            	//gen_ca_pvr_replay_monitor();
			//for Ali internal test
			//gen_ca_change_key_monitor();

	            	dwTimeStamp = osal_get_tick();
	       }
		osal_task_sleep(10);
	}
	
}
INT32 gen_ca_task_init(void)
{
    OSAL_T_CTSK t_ctsk;

    MEMSET(&t_ctsk, 0, sizeof(T_CTSK));
    /* create main thread */
    t_ctsk.task = (FP)(gen_ca_main_task);
    t_ctsk.itskpri=OSAL_PRI_NORMAL;
    t_ctsk.quantum=10;
    t_ctsk.stksz=0x4000;
    t_ctsk.para1=0;
    t_ctsk.para2=0;
    t_ctsk.name[0] = 'G';
    t_ctsk.name[1] = 'E';
    t_ctsk.name[2] = 'N';

    g_gen_ca_tsk_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == g_gen_ca_tsk_id)
    {
	 GEN_CA_PVR_ERROR("%s : osal_task_create Fail\n", __FUNCTION__);
        return ERR_FAILUE;
    }

/*    g_gen_ca_mutex = osal_mutex_create();
    if(OSAL_INVALID_ID==g_gen_ca_mutex)
    {
        ASSERT(0);
        while(1);
    }*/
    return SUCCESS;
}

#ifdef _C0700A_VSC_ENABLE_
static int casA_load_otpkey(UINT8 keyset)
{
    p_ce_device p_cedev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    OTP_PARAM opt_info;

    MEMSET(&opt_info, 0, sizeof(OTP_PARAM));
    opt_info.otp_addr =  OTP_ADDESS_1 + (4 * keyset);
    opt_info.otp_key_pos = KEY_0_0 + keyset;
    if (RET_SUCCESS != ce_key_load(p_cedev0, &opt_info))
    {
        GEN_CA_PVR_ERROR("load m2m2 key failed!");
        return -1;
    }
    GEN_CA_PVR_DEBUG("load %dth key successfully!\n", keyset);
    return 0;
}
#endif

int gen_ca_pvr_init(void)
{
    int i = -1;

    GEN_CA_PVR_DEBUG("%s() load m2m2 key and PVR key to CE\n", __FUNCTION__);

    //add for pvr generate the R1
    if(ALI_S3281 == sys_ic_get_chip_id())
    {
        if(THREE_LEVEL != sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS))
        {
            bl_generate_store_key_for_pvr();
        }
    }
#ifdef _C0700A_VSC_ENABLE_
    casA_load_otpkey(KEY_0_2);
#else
    gen_ca_load_m2m2_key();
    //gen_ca_load_pk_to_ce();
#endif    
	
    pvr_ce_dev = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    pvr_dsc_dev = (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);

    if((NULL == pvr_ce_dev) || (NULL == pvr_dsc_dev))
    {
        ASSERT(0);
        while(1);
        return -1;
    }

    MEMSET(&g_cpm, 0, sizeof(gen_ca_pvr_mgr));
    if (OSAL_INVALID_ID == (g_cpm.mutex_id = osal_mutex_create()))
    {
        GEN_CA_PVR_ERROR("Create mutex failed!\n");
        ASSERT(0);
        while(1);
        return -1;
    }

    gen_ca_pvr_lock();

    for (i = 0; i < GEN_CA_PVR_RSC_REC_NUM; ++i)
    {
        g_cpm.rec[i].pvr_hnd = 0;
        g_cpm.rec[i].crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
        g_cpm.rec[i].crypto_stream_id = INVALID_CRYPTO_STREAM_ID;
        g_cpm.rec[i].crypto_key_pos = INVALID_CE_KEY_POS;
    }

    for (i = 0; i < GEN_CA_PVR_RSC_PLAY_NUM; ++i)
    {
        g_cpm.play[i].pvr_hnd = 0;
        g_cpm.play[i].crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
        g_cpm.play[i].crypto_stream_id = INVALID_CRYPTO_STREAM_ID;
        g_cpm.play[i].crypto_key_pos = INVALID_CE_KEY_POS;
    }

    gen_ca_pvr_un_lock();

    return 0;
}


/**
 * encrypt/decrypt data: default to (TDES, CBC)
 */
int gen_ca_crypto_data(pvr_crypto_data_param *cp)
{
    p_ce_device p_ce_dev0 = NULL;
    p_des_dev des_dev = NULL;
    UINT16 pid_list[1] = {0x1234};
    DES_INIT_PARAM des_param ;
    KEY_PARAM key_param;
    CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,GEN_CA_M2M2_KEY_POS};
    UINT32 key_level = sys_ic_get_kl_key_mode(GEN_CA_M2M2_KEY_POS);
    UINT32 key_pos = INVALID_CE_KEY_POS;
    UINT32 des_dev_id = INVALID_DSC_SUB_DEV_ID;
    UINT32 stream_id = INVALID_CRYPTO_STREAM_ID;
    RET_CODE ret = RET_FAILURE;

    if(NULL == cp)
    {
        return -1;
    }
#if 0 // not encrypt/decrypt data, only for test
    MEMCPY(cp->output, cp->input, cp->data_len);
    return RET_SUCCESS;
#endif
    MEMSET(&des_param, 0, sizeof(DES_INIT_PARAM));
    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    if (INVALID_DSC_SUB_DEV_ID == (des_dev_id = dsc_get_free_sub_device_id(DES)))
    {
        GEN_CA_PVR_ERROR("dsc_get_free_sub_device_id() failed\n");
        return -1;
    }

    if (INVALID_CRYPTO_STREAM_ID == (stream_id = dsc_get_free_stream_id(PURE_DATA_MODE)))
    {
        GEN_CA_PVR_ERROR("dsc_get_free_stream_id() failed\n");
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    des_dev = (p_des_dev)dev_get_by_id(HLD_DEV_TYPE_DES, des_dev_id);
    if (!(p_ce_dev0 && des_dev))
    {
        GEN_CA_PVR_ERROR("No valid device for crypto: (0x%X, 0x%X)\n", p_ce_dev0, des_dev);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
    if( THREE_LEVEL > key_level)
    {
        key_pos_param.ce_key_level = GEN_CA_CRYPTO_KEY_LEVEL;
    }
    else
    {
        key_pos_param.ce_key_level = THREE_LEVEL;
    }
    #if 1//def _C0700A_VSC_ENABLE_	//Ben 180727#1 for GSP CA
    key_pos_param.number = 1;
    #else
    key_pos_param.number = 2;
    #endif
    key_pos_param.pos = INVALID_CE_KEY_POS;
    key_pos_param.root = GEN_CA_M2M2_KEY_POS;
    ret = ce_ioctl(p_ce_dev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
    if (RET_SUCCESS != ret)
    {
        GEN_CA_PVR_ERROR("find free key pos fail!\n");
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    // generate encrypting key
    key_pos = key_pos_param.pos;

    //ret = ce_generate_cw_key(cp->key_ptr, AES, THREE_LEVEL==key_level ? g_pvr_level2pos : GEN_CA_PVR_KEY_POS,key_pos);
    ret = gen_ca_pvr_setup_key(key_pos);

    
    if (RET_SUCCESS != ret)
    {
        GEN_CA_PVR_ERROR("generate key fail!\n");
        ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    MEMSET(&des_param, 0, sizeof(DES_INIT_PARAM));
    des_param.dma_mode = PURE_DATA_MODE;
    des_param.key_from = KEY_FROM_CRYPTO;
    des_param.key_mode = TDES_ABA_MODE;
    des_param.parity_mode = EVEN_PARITY_MODE; //AUTO_PARITY_MODE0;
    des_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    des_param.scramble_control = 0;
    des_param.stream_id = stream_id;
    des_param.work_mode = WORK_MODE_IS_CBC;
    des_param.sub_module = TDES;
    des_param.cbc_cts_enable = 0;
    ret = des_ioctl(des_dev, IO_INIT_CMD, (UINT32)&des_param);
    if (RET_SUCCESS != ret)
    {
        GEN_CA_PVR_ERROR("des_ioctl() IO_INIT_CMD failed\n");
        ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    key_param.handle = INVALID_CRYPTO_STREAM_HANDLE;
    key_param.ctr_counter = NULL;
    key_param.init_vector = cp->iv_ptr;
    key_param.key_length = cp->key_len; // 128 bits ,or  192bits or 256 bits
    key_param.pid_len = 1; //not used
    key_param.pid_list = pid_list; //not used
    key_param.p_des_iv_info = (DES_IV_INFO *)(cp->iv_ptr);
    key_param.p_des_key_info = NULL;
    key_param.stream_id = stream_id; //0-3 for dmx id , 4-7 for pure data mode
    key_param.force_mode = 1;
    //key_param.pos = key_pos; //Descrambler can find the key which store in Crypto Engine by the key_pos
    key_param.pos = (key_pos&0xFF); //Descrambler can find the key which store in Crypto Engine by the key_pos
    key_param.kl_sel = (key_pos>>8)&0x0F;

    ret = des_ioctl(des_dev, IO_CREAT_CRYPT_STREAM_CMD, (UINT32)&key_param);
    if (RET_SUCCESS != ret)
    {
        GEN_CA_PVR_ERROR("des_ioctl() IO_CREAT_CRYPT_STREAM_CMD failed\n");
        ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

    // encrypt or decrypt data
    if (cp->encrypt)
    {
        ret = des_encrypt(des_dev, stream_id, cp->input, cp->output, cp->data_len);
    }
    else
    {
        ret = des_decrypt(des_dev, stream_id, cp->input, cp->output, cp->data_len);
    }

    if (RET_SUCCESS != ret)
    {
        GEN_CA_PVR_ERROR("%scrypt pure data failed! ret = %d\n", cp->encrypt ? "En" : "De", ret);
    }

    // release resource
    des_ioctl(des_dev, IO_DELETE_CRYPT_STREAM_CMD, key_param.handle);
    ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
    dsc_set_stream_id_idle(stream_id);
    dsc_set_sub_device_id_idle(DES, des_dev_id);
    return (RET_SUCCESS == ret) ? 0 : -1;
}

//#endif /* GEN_CA_PVR_SUPPORT */
