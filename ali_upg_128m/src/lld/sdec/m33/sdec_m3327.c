/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sdec_m3327.c
*
*    Description: The file is to operate subtitle decoder
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <osal/osal.h>
#include <basic_types.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/sdec/sdec.h>
#include <hld/sdec/sdec_dev.h>
#include <hld/osd/osddrv.h>
#include <api/libsubt/subt_osd.h>
#include "sdec_buffer.h"
#include "sdec_m3327.h"
#include "sdec_m3327_internal.h"

struct sdec_m3327_private * g_sdec_priv = NULL;
static OSAL_ID sdec_tsk_id = OSAL_INVALID_ID;
OSAL_ID sdec_flag_id = OSAL_INVALID_ID;
UINT8  sdec_tsk_qtm = 0;
UINT8  sdec_tsk_pri = 0;
UINT16 rle_len = 0;
UINT8 *rle_buf = NULL;
UINT8 cnt4bit = 0;
UINT8 cnt2bit = 0;
UINT8 *subt_data_buff = NULL;
UINT16 subt_data_buff_len = 0;
UINT32 sdec_temp_size = 0;
UINT8 *sdec_temp_buf = NULL;
UINT32 sdec_temp_buf_len = 0;
UINT32 sdec_temp_size2 = 0;
UINT16 g_composition_page_id = 0 ;
UINT16 g_ancillary_page_id  = 0;
struct sdec_data_hdr *sdec_bs_hdr_buf = NULL;
UINT8 page_released = 0;
UINT8 page_finished = 0;
BOOL subt_show_on = FALSE;
UINT8 subt_transparent_color = 0;
struct page_cb g_page_cb;
struct region_cb g_region_cb[MAX_REGION_IN_PAGE];
UINT8 g_region_idx = 0;
UINT8 identifier_detected = 0;
UINT8 sdec_stuffing_flag = 0;
UINT8    sdec_flag_not_enough_data = 0;
UINT8 subt_clut[256][4];
//UINT8 subt_clut[MAX_REGION_IN_PAGE][MAX_ENTRY_IN_CLUT][4];
UINT8 subt_clut_id[MAX_REGION_IN_PAGE]={0};
UINT32 sdec_get_clut = 0;
struct sdec_data_hdr *p_sdec_rd_hdr = NULL;

static struct sdec_data_hdr p_sdec_init_hdr;
static struct sdec_data_hdr *p_sdec_wr_hdr = NULL;
static BOOL sdec_stopped = FALSE;//get sdec status
static BOOL sdec_clear_page = FALSE;
static UINT8 last_stcid=0xff;//only use in the function below
static UINT32 last_pts_sdec = 0;
static struct sdec_device *sdec_dev = NULL;

const UINT8 subt_defalt_clut[MAX_ENTRY_IN_CLUT][4] =
{
    {0x00,0x00,0x00,0xff},{0x10,0x80,0x80,0x00},
    {0xeb,0x80,0x80,0x00},{0x10,0x80,0x80,0x00},
    {0xeb,0x80,0x80,0x00},{0x10,0x80,0x80,0x00},
    {0xeb,0x80,0x80,0x00},{0x10,0x80,0x80,0x00},
    {0xeb,0x80,0x80,0x00},{0x10,0x80,0x80,0x00},
    {0xeb,0x80,0x80,0x00},{0x10,0x80,0x80,0x00},
    {0xeb,0x80,0x80,0x00},{0x10,0x80,0x80,0x00},
    {0xeb,0x80,0x80,0x00},{0xdc,0x80,0x80,0xff}
};


BOOL get_sdec_status(void)
{
    return sdec_stopped;
}

void set_sdec_clear_page(BOOL clear)
{
	if((TRUE!=clear)&&(FALSE!=clear))
	{
		return;
	}
    sdec_clear_page = clear;
}

__ATTRIBUTE_RAM_
void sdec_add_to_temp(UINT8 *p_data,UINT32 size)
{
    UINT32 i = 0;

    if(NULL == p_data)
    {
        return;
    }

    //SDEC_PRINTF("2. p_data = %8x, size = %d\n",p_data,size);
    if(sdec_temp_size+size >sdec_temp_buf_len)
    {
        SDEC_PRINTF("%s : Error: exceptional temp data size!\n",__FUNCTION__);
        return;
    }
    for(i=0;i<size;i++)
    {
        sdec_temp_buf[sdec_temp_size+i]=*p_data++;
    }
    sdec_temp_size += size;
}

__ATTRIBUTE_RAM_
void sdec_copy_to_temp(UINT8 *p_data,UINT32 size)
{
    UINT32 i = 0;

    if(NULL == p_data)
    {
        return;
    }

    if(size >sdec_temp_buf_len-184)
    {
        SDEC_PRINTF("%s : Error: exceptional temp data size!\n",__FUNCTION__);
        return;
    }
    for(i=0;i<size;i++)
    {
        sdec_temp_buf[i]=*p_data++;
    }
    sdec_temp_size = size;
}

__ATTRIBUTE_RAM_
UINT8 getnext2bit(void)
{

    UINT8 mychar = 0;
    UINT8 my2bit = 0;
    UINT8 temp_const = (UINT8)0xc0;
    UINT8 temp_cnt2bit = 0;

    if(0==rle_len)
    {
        return 0xff; //
    }
    switch(cnt2bit)
    {
        case 0:
        rle_buf++;
        rle_len--;
        mychar = *rle_buf;
        my2bit = mychar >> 6;
        cnt2bit++;
        break;

        case 3:
        //mychar = *rle_buf++;
        //rle_len--;
        mychar = *rle_buf;
        my2bit = (mychar&0x03);
        cnt2bit = 0;
        break;

        default:
        mychar = *rle_buf;
        //my2bit = (mychar&(0xc0>>(2*cnt2bit))) >> (6-2*cnt2bit);
        temp_const = (UINT8)0xc0;
        temp_cnt2bit =(UINT8)(mychar&(temp_const>>(cnt2bit<<1)));
        my2bit = temp_cnt2bit >> (6-(cnt2bit<<1));
        cnt2bit++;
        break;
    }

    return my2bit;
}
__ATTRIBUTE_RAM_
UINT8 getnext4bit(void)
{

    UINT8 mychar = 0;
    UINT8 my4bit = 0;

    if((0==rle_len) && (0 == cnt4bit))
    {
        return 0xff; //
    }

    if( 0 == cnt4bit)
    {
        ++rle_buf;
        mychar = *rle_buf;
        rle_len--;
        // my4bit = (mychar&(0xf0>>(4*cnt4bit))) >> (4-4*cnt4bit);
        my4bit = (mychar >> 4);
        cnt4bit++;
    }
    else if( 1 == cnt4bit )
    {
        mychar = *rle_buf;
        //my4bit = (mychar&(0xf0>>(4*cnt4bit))) >> (4-4*cnt4bit);
        my4bit = (mychar&0x0F);
        cnt4bit = 0;
    }

    return my4bit;
}

__ATTRIBUTE_RAM_
UINT16 getnext8bit(void)
{
    UINT16 my8bit = 0;

    if(0==rle_len)
    {
        return 0xffff;
    }

    my8bit = *++rle_buf;
    rle_len--;

    return my8bit;
}

static void sdec_m3327_software_init(void)
{
    INT32 i = 0;
    INT32 j = 0;
    INT32 ret_func = 0;

    p_sdec_init_hdr.u_data_size = 0;
    p_sdec_rd_hdr = &p_sdec_init_hdr;
    p_sdec_wr_hdr = &p_sdec_init_hdr;

    sdec_get_clut = 0;

    sdec_temp_size = 0;
    sdec_temp_size2 = 0;

    page_released = 1;
    page_finished = 0;
    g_region_idx = 0;

    MEMSET(&g_page_cb,0,sizeof(struct page_cb));
    g_page_cb.clut_cnt = 0;
    g_page_cb.region_cnt = 0;
    for(i=0;i<MAX_REGION_IN_PAGE;i++)
    {
        g_page_cb.region_id[i]=0xff;
        g_page_cb.end_of_display = 1;
    }
    for(i=0;i<MAX_REGION_IN_PAGE;i++)
    {
        MEMSET(&g_region_cb[i],0,sizeof(struct region_cb));
        g_region_cb[i].object_cnt = 0;
        g_region_cb[i].region_id = 0xff;
        g_region_cb[i].region_width = 0;
        g_region_cb[i].region_height = 0;
        for(j=0;j<MAX_OBJECT_IN_REGION;j++)
        {
            g_region_cb[i].object_id[j] = 0xffff;
        }
        //g_region_cb[i].data = sdec_pixel_buf;
    }

    g_page_cb.display_width = 0;
    g_page_cb.display_height = 0;

    //Initialize subt_clut to transparent to fix "green block" issue
    //for some stream have no clut data
    for(i = 0; i < MAX_REGION_IN_PAGE; i++)
    {
        for(j=0;j<MAX_ENTRY_IN_CLUT;j++)
        {
			subt_clut[PALLETTE_OFFSET + i*MAX_ENTRY_IN_CLUT + j][3] = (15<<4);
        }
        //MEMCPY(subt_clut[i], subt_defalt_clut, sizeof(subt_defalt_clut));
    }

    //MEMSET(sdec_pixel_buf, subt_transparent_color, sdec_pixel_buf_len);

    identifier_detected=0;
    sdec_stuffing_flag = 0;
    sdec_flag_not_enough_data = 0;

    ret_func = sdec_sbf_create();
	ret_func=(RET_SUCCESS==ret_func)? RET_SUCCESS:RET_FAILURE;
    ret_func = sdec_hdr_buf_create();
	ret_func=(RET_SUCCESS==ret_func)? RET_SUCCESS:RET_FAILURE;
    SDEC_PRINTF("%s : initialize sdec!\n",__FUNCTION__);
    //osd_subt_enter();
}

static void sdec_task(UINT32 param1,__MAYBE_UNUSED__ UINT32 param2)
{
    OSAL_ER result = 0;
    UINT32 flgptn = 0 ;
    struct sdec_m3327_private*priv=NULL;
	if(0==param1)
	{
		return ;
	}
	param2=0;
    priv = (struct sdec_m3327_private*)(((struct sdec_device*)param1)->priv);

    while(1) // run forever when task exist.
    {
        if(SDEC_STATE_IDLE== priv->status )
        {
            result = osal_flag_wait(&flgptn,sdec_flag_id, SDEC_CMD_START|
                SDEC_CMD_SHOWON|SDEC_CMD_SHOWOFF, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
            if(flgptn&SDEC_CMD_START)
            {
                SDEC_PRINTF("%s : SDEC start!\n",__FUNCTION__);
                osal_flag_clear(sdec_flag_id, SDEC_CMD_START);
                sdec_m3327_software_init();
                osal_flag_set(sdec_flag_id,SDEC_MSG_START_OK);
                priv->status = SDEC_STATE_PLAY;

            }
            if(flgptn&SDEC_CMD_SHOWON)
            {
                SDEC_PRINTF("%s : SDEC start show!\n",__FUNCTION__);
                osal_flag_clear(sdec_flag_id, SDEC_CMD_SHOWON);
                subt_show_on = TRUE;
                osd_subt_enter();
                osal_flag_set(sdec_flag_id,SDEC_MSG_SHOWON_OK);

            }
            if(flgptn&SDEC_CMD_SHOWOFF)
            {
                SDEC_PRINTF("%s : SDEC hide!\n",__FUNCTION__);
                osal_flag_clear(sdec_flag_id, SDEC_CMD_SHOWOFF);
                subt_show_on = FALSE;
                osd_subt_leave();
                osal_flag_set(sdec_flag_id,SDEC_MSG_SHOWOFF_OK);
            }
        }
        else if(SDEC_STATE_PLAY == priv->status)
        {
            result = osal_flag_wait(&flgptn,sdec_flag_id, SDEC_CMD_STOP|
                SDEC_CMD_SHOWON|SDEC_CMD_SHOWOFF|SDEC_MSG_DATA_AVAILABLE|
                SDEC_MSG_BUFF_AVAILABLE, OSAL_TWF_ORW, 100);
             if(sdec_clear_page)
            {
                //libc_printf("\nclear page");
                SDEC_PRINTF("%s : SDEC clear page!\n",__FUNCTION__);
                 osd_subt_clear_page();
                 set_sdec_clear_page(FALSE);
            }
            if(OSAL_E_TIMEOUT==result)
            {
                 //SDEC_PRINTF("Wait update flag time out!\n");
                 if(priv->display_entry_cb)
                 {
                     priv->display_entry_cb(0);
                 }
                 //display_entry();
                 continue;
            }

            if(flgptn&SDEC_CMD_STOP)
            {
                SDEC_PRINTF("%s : SDEC stop!\n",__FUNCTION__);
                osal_flag_clear(sdec_flag_id, SDEC_CMD_STOP|SDEC_CMD_START|
                    SDEC_MSG_DATA_AVAILABLE|SDEC_MSG_BUFF_AVAILABLE);
                //osal_flag_clear(sdec_flag_id, SDEC_CMD_STOP);
                //osd_subt_leave();
                osal_flag_set(sdec_flag_id,SDEC_MSG_STOP_OK);
                priv->status = SDEC_STATE_IDLE;

                continue;
            }
            if(flgptn&SDEC_CMD_SHOWON)
            {
                SDEC_PRINTF("%s : SDEC show!\n",__FUNCTION__);
                osal_flag_clear(sdec_flag_id, SDEC_CMD_SHOWON);
                subt_show_on = TRUE;
                osd_subt_enter();
                osal_flag_set(sdec_flag_id,SDEC_MSG_SHOWON_OK);

            }
            if(flgptn&SDEC_CMD_SHOWOFF)
            {
                SDEC_PRINTF("%s : SDEC hide!\n",__FUNCTION__);
                osal_flag_clear(sdec_flag_id, SDEC_CMD_SHOWOFF);
                subt_show_on = FALSE;
                osd_subt_leave();
                osal_flag_set(sdec_flag_id,SDEC_MSG_SHOWOFF_OK);

            }
            if(flgptn&SDEC_MSG_DATA_AVAILABLE)
            {
                //SDEC_PRINTF("update flag received!\n");
                osal_flag_clear(sdec_flag_id, SDEC_MSG_DATA_AVAILABLE);
                if(priv->stream_parse_cb)
                {
                    priv->stream_parse_cb();
                }
                //stream_parse();
                 if(priv->display_entry_cb)
                 {
                    priv->display_entry_cb(1);
                 }
                //display_entry();
            }
            if(flgptn&SDEC_MSG_BUFF_AVAILABLE)
            {
                //SDEC_PRINTF("release flag received!\n");
                osal_flag_clear(sdec_flag_id, SDEC_MSG_BUFF_AVAILABLE);
                if(priv->stream_parse_cb)
                {
                    priv->stream_parse_cb();
                }
                //stream_parse();
                 if(priv->display_entry_cb)
                 {
                    priv->display_entry_cb(2);
                 }
                //display_entry();
            }
        }
    }

}

__ATTRIBUTE_REUSE_
INT32  sdec_m3327_open(struct sdec_device *dev)
{

    OSAL_T_CTSK t_ctsk;
    struct sdec_m3327_private *priv = NULL;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    priv = (struct sdec_m3327_private*)(dev->priv) ;
    MEMSET(&t_ctsk,0,sizeof(OSAL_T_CTSK));
    subt_show_on = TRUE;
    priv->status = SDEC_STATE_IDLE;

    sdec_flag_id=osal_flag_create(0x00000000);
    if(OSAL_INVALID_ID==sdec_flag_id)
    {
        SDEC_PRINTF("%s : Error: Create flag of sdec  failed!\n",__FUNCTION__);
        return RET_FAILURE;
    }
    t_ctsk.task = (FP)sdec_task;
    t_ctsk.para1 = (UINT32)dev;
    t_ctsk.para2 = 0;
    t_ctsk.stksz = 0x1500;
    t_ctsk.quantum = sdec_tsk_qtm;
    t_ctsk.itskpri = sdec_tsk_pri;
    sdec_tsk_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID==sdec_tsk_id)
    {
        SDEC_PRINTF("%s : Error: Can not create SDECDec_Task!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    SDEC_PRINTF("%s : sdec_m3327 open SUCCESS!\n",__FUNCTION__);
    return RET_SUCCESS;

}

INT32  sdec_m3327_close(struct sdec_device *dev)
{
    if(NULL == dev)
    {
        return RET_FAILURE;
    }


    if(OSAL_E_OK!=osal_task_delete(sdec_tsk_id))
    {
        SDEC_PRINTF("%s : Error: fail to delete task!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    if(OSAL_E_OK!=osal_flag_delete(sdec_flag_id))
    {
        SDEC_PRINTF("%s : Error: fail to delete flag!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    return RET_SUCCESS;
}

INT32 sdec_m3327_start(struct sdec_device *dev,UINT16 composition_page_id,UINT16 ancillary_page_id)
{
    struct sdec_m3327_private *priv = NULL;
    UINT32 flgptn = 0 ;
    OSAL_ER result = 0;

    if((NULL == dev)||(0xffff==composition_page_id)||(0xffff==ancillary_page_id))
    {
        return RET_FAILURE;
    }

    priv = (struct sdec_m3327_private*)(dev->priv) ;
    if( SDEC_STATE_IDLE== priv->status)
    {
        SDEC_PRINTF("%s : User start sdec!\n",__FUNCTION__);

        g_composition_page_id = composition_page_id;
        g_ancillary_page_id = ancillary_page_id;
        osal_flag_set(sdec_flag_id, SDEC_CMD_START);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,sdec_flag_id, SDEC_MSG_START_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
             SDEC_PRINTF("%s : Error: Wait SDEC_MSG_START_OK failed witherr code %s\n",
             __FUNCTION__, result == OSAL_E_TIMEOUT?"OSAL_E_TIMEOUT":"OSAL_E_FAIL");
             return !RET_SUCCESS;
        }
        osal_flag_clear(sdec_flag_id, SDEC_MSG_START_OK);
    }
    sdec_stopped = FALSE;
    return RET_SUCCESS;
}

INT32  sdec_m3327_stop(struct sdec_device *dev)
{
    struct sdec_m3327_private *priv = NULL;
    UINT32 flgptn = 0;
    OSAL_ER result = 0;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    priv = (struct sdec_m3327_private*)(dev->priv) ;

    if(SDEC_STATE_PLAY == priv->status)
    {
        SDEC_PRINTF("%s : User stop sdec!\n",__FUNCTION__);
        osal_flag_set(sdec_flag_id, SDEC_CMD_STOP);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,sdec_flag_id, SDEC_MSG_STOP_OK,
            OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
             SDEC_PRINTF("%s : Error: Wait SDEC_MSG_START_OK failed with err code %s\n",
             __FUNCTION__, result == OSAL_E_TIMEOUT?"OSAL_E_TIMEOUT":"OSAL_E_FAIL");
             return !RET_SUCCESS;
        }
        osal_flag_clear(sdec_flag_id, SDEC_MSG_STOP_OK);

    }
    sdec_stopped = TRUE;
    return RET_SUCCESS;
}


INT32  sdec_m3327_pause(struct sdec_device *dev)
{
    struct sdec_m3327_private *priv = NULL;
    UINT32 flgptn = 0 ;
    OSAL_ER result = 0;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    priv = (struct sdec_m3327_private*)(dev->priv) ;

    if(SDEC_STATE_PLAY== priv->status )
    {
        SDEC_PRINTF("%s : User pause sdec!\n",__FUNCTION__);
        osal_flag_set(sdec_flag_id, SDEC_CMD_PAUSE);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,sdec_flag_id, SDEC_MSG_PAUSE_OK,
            OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
             SDEC_PRINTF("%s : Error: Wait SDEC_MSG_PAUSE_OK failed with err code %s\n",
             __FUNCTION__, result == OSAL_E_TIMEOUT?"OSAL_E_TIMEOUT":"OSAL_E_FAIL");
             return !RET_SUCCESS;
        }
        osal_flag_clear(sdec_flag_id, SDEC_MSG_PAUSE_OK);
    }
       return RET_SUCCESS;
}
INT32  sdec_m3327_showonoff(struct sdec_device *dev,BOOL b_on)
{
    UINT32 flgptn = 0;
    OSAL_ER result = 0;

    if(NULL == dev)
    {
        return RET_FAILURE;
    }

    if( TRUE== b_on)
    {
        osal_flag_set(sdec_flag_id, SDEC_CMD_SHOWON);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,sdec_flag_id, SDEC_MSG_SHOWON_OK,
            OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
             SDEC_PRINTF("%s : Error: Wait SDEC_MSG_PAUSE_OK failed with err code %s\n",
             __FUNCTION__, result == OSAL_E_TIMEOUT?"OSAL_E_TIMEOUT":"OSAL_E_FAIL");
             return !RET_SUCCESS;
        }
        osal_flag_clear(sdec_flag_id, SDEC_MSG_SHOWON_OK);
    }
    else
    {
        osal_flag_set(sdec_flag_id, SDEC_CMD_SHOWOFF);
        flgptn = 0;
        result = osal_flag_wait(&flgptn,sdec_flag_id, SDEC_MSG_SHOWOFF_OK, 
            OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
             SDEC_PRINTF("%s : Error: Wait SDEC_MSG_PAUSE_OK failed with err code %s\n",
             __FUNCTION__, result == OSAL_E_TIMEOUT?"OSAL_E_TIMEOUT":"OSAL_E_FAIL");
             return !RET_SUCCESS;
        }
        osal_flag_clear(sdec_flag_id, SDEC_MSG_SHOWOFF_OK);
    }
       return RET_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////

__ATTRIBUTE_RAM_
INT32 sdec_m3327_request_write(struct sdec_device *dev,UINT32 u_size_requested,
struct control_block *pt_data_ctrl_bl,UINT8 **ppu_data,UINT32 *pu_size_got)
{
    UINT32 u_req_size = 0;
    UINT32 u_hdr_ptr = 0;
    INT32 ret_func = 0;
	if(NULL == dev)
    {
        SDEC_PRINTF("%s : Error parameter!\n",__FUNCTION__);
        return RET_FAILURE;
    }
    struct sdec_m3327_private *priv = (struct sdec_m3327_private*)(dev->priv) ;
    if(/*(NULL == *ppu_data)||*/(NULL == pu_size_got))
    {
        SDEC_PRINTF("%s : Error parameter!\n",__FUNCTION__);
        return RET_FAILURE;
    }

    if( SDEC_STATE_IDLE == priv->status)
    {
        SDEC_PRINTF("SDEC_STATE_IDLE!\n");
        return RET_STA_ERR;
    }

    if(0)//g_is_hdvideo)
    {
        SDEC_PRINTF("SDEC_HD_ABORT!\n");
        return RET_STA_ERR;
    }

    //u_size_requested= (u_size_requested>552)?552:(u_size_requested);
    u_size_requested= (u_size_requested>184)?184:(u_size_requested);

    u_req_size= u_size_requested;
    //SDEC_PRINTF("\nD WR REQ(%x):\n",u_size_requested);
    if(RET_FAILURE==sdec_sbf_wr_req(&u_req_size, ppu_data))
    {
        SDEC_PRINTF("request write FAILURE\n");
        return RET_FAILURE;
    }
    //SDEC_PRINTF("size= %d ,wpt = %x\n",u_req_size,*ppu_data);

    if(sdec_hdr_buf_wr_req(&u_hdr_ptr)==RET_FAILURE)
    {
        SDEC_PRINTF("HDR request write FAILURE\n");
        //static struct sdec_device *sdec_dev = NULL;

        sdec_dev = (struct sdec_device *)dev_get_by_name("SDEC_M3327_0");
        ret_func = sdec_m3327_stop(sdec_dev);
		ret_func=(RET_SUCCESS==ret_func)? RET_SUCCESS:RET_FAILURE;
        ret_func = sdec_m3327_start(sdec_dev,g_composition_page_id,g_ancillary_page_id);
		ret_func=(RET_SUCCESS==ret_func)? RET_SUCCESS:RET_FAILURE;
        //reset sdec to fix rotate stream's subtitle no show bug

        return RET_FAILURE;
    }
    p_sdec_wr_hdr = &sdec_bs_hdr_buf[u_hdr_ptr];
    p_sdec_wr_hdr->pu_start_point = *ppu_data;
    p_sdec_wr_hdr->u_data_size = 0;

    if (NULL == pt_data_ctrl_bl)
    {
        p_sdec_wr_hdr->b_info_valid = FALSE;
    }
    else
    {
        p_sdec_wr_hdr->b_info_valid = TRUE;
        p_sdec_wr_hdr->u_pes_start = (pt_data_ctrl_bl->data_continue == 0x00)?
                                0x01:0x00;
        //pSDecWrHdr->uCtrlByte = pt_data_ctrl_bl->uCtrlByte;

        if(1 == pt_data_ctrl_bl->pts_valid)
        {
            last_stcid = pt_data_ctrl_bl->stc_id;
            last_pts_sdec = pt_data_ctrl_bl->pts;
        }
        p_sdec_wr_hdr->u_stc_id = last_stcid;
        p_sdec_wr_hdr->u_pts = last_pts_sdec;
        pt_data_ctrl_bl->instant_update = 1;
    }

    *pu_size_got = (u_req_size>u_size_requested? u_size_requested : u_req_size);
    return RET_SUCCESS;

}
__ATTRIBUTE_RAM_
void sdec_m3327_update_write(struct sdec_device *dev,UINT32 u_size)
{
	if(NULL == dev)
    {
        SDEC_PRINTF("%s : Error parameter!\n",__FUNCTION__);
        return ;
    }
    struct sdec_m3327_private *priv = (struct sdec_m3327_private*)(dev->priv) ;

    if(SDEC_STATE_IDLE==priv->status  )
    {
        SDEC_PRINTF("SDEC_STATE_IDLE!\n");
        return ;
    }

    if(0)//g_is_hdvideo)
    {
        SDEC_PRINTF("SDEC_HD_ABORT!\n");
        return;
    }

    if(NULL==p_sdec_wr_hdr)
    {
        return;
    }
    if(!u_size)
    {
        return ;
    }

    //SDEC_PRINTF("D WR UPD:\n");
     sdec_sbf_wr_update(u_size);

    //SDEC_PRINTF("wr upd = %d\n",u_size);

    p_sdec_wr_hdr->u_data_size += u_size;

    sdec_hdr_buf_wr_update();
    osal_flag_set(sdec_flag_id,SDEC_MSG_DATA_AVAILABLE);
    //SDEC_PRINTF("UPD_STC=%8x, uPTS= %8x\n",stc_msb32,pSDecWrHdr->uPTS);
    return ;
}
///////////////////////////////////////////////////////////////////////////






