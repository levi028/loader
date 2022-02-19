/*****************************************************************************
    *    Copyright (c) 2013 ALi Corp. All Rights Reserved
    *    This source is confidential and is ALi's proprietary information.
    *    This source is subject to ALi License Agreement, and shall not be
         disclosed to unauthorized individual.

    *    File: lib_atsc_subt.c

    *    Description:
            this file the parse the subtitle defined by the specification
            SUBTITLING METHODS FOR BROADCAST CABLE.

            AMERICAN NATIONAL STANDARD
                ANSI/SCTE 27 2003
                (Formerly DVS 026)
            SUBTITLING METHODS FOR BROADCAST CABLE

            History:
            create thie file 2009/8/26 by zhaojian_tang (zhaojian_tang@ali.com.tw)
    *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
          KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
          IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
          PARTICULAR PURPOSE.
    *****************************************************************************/

#include <osal/osal.h>
#include <basic_types.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libtsi/si_types.h>
#include <api/libc/fast_crc.h>
#include <api/libsubt/lib_subt.h>
#include <api/libsubt/subt_osd.h>
#include <hld/sdec/sdec.h>
#ifdef LIB_TSI3_FULL
#include <api/libtsi/si_section.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/sie_monitor.h>
#endif
#include <hld/osd/osddrv_dev.h>
#include <hld/osd/osddrv.h>
#include <hld/snd/snd.h>

#include "lib_atsc_subt.h"
#include "lib_subt_atsc_osd.h"
#include "lib_subt_internal.h"

//#define _DEBUG_INFO
//#define SUBT_ATSC_PRINTF libc_printf
#define SUBT_ATSC_PRINTF(...)

UINT8 osd_status = 0;
//define the global variable using by this module.
UINT16 globe_width_real = 0;
UINT16 globe_height_real = 0;
struct osdrect g_subt_atsc_rect;

static OSAL_ID scte_semaphore = OSAL_INVALID_ID;
static UINT8 bits_used_in_bytes = 0;
static UINT16 m_u16bitmap_len = 0;
static UINT16 m_u16bitmap_len_global2 = 0;

static UINT8 m_lpsz_bitmap[REASSEMBLY_LENGTH] = {0XFF};


static UINT16 m_bitmapdatagot = 0;
static atsc_subt_segment_overlay m_segment_overlay;
static atsc_subt_fixed_header1 m_sub_thead1;
static atsc_subt_fixed_header2 m_sub_theader2;
static UINT16 bmp_consumer_bytes_offset = 0;
static BOOL subt_atsc_onoff = FALSE;
static OSAL_ID lib_subt_atsc_alarm_id = OSAL_INVALID_ID;
static BOOL g_lib_subt_atsc_clean_osd = FALSE;
static UINT8* g_sec_buf=NULL; //see use
static UINT32 g_sec_buf_len=0;
static UINT8* g_sec_buf_end = NULL;

static UINT8 g_subt_atsc_config=FALSE;
static struct atsc_subt_config_par g_subt_atsc_par;
static struct scte_subt_msg g_msg;
static OSAL_ID scte_sdec_flag_id = OSAL_INVALID_ID;
static UINT32  scte_sdec_status=SCTE_SDEC_STATE_IDLE; //SCTE_SDEC_STATE_EMPT;
static UINT8  *bs_buf_sdec_scte = NULL;
static INT32 bs_buf_len_sdec_scte = 0;
static INT32 bs_wr_ptr_sdec_scte = 0;
static INT32 bs_rd_ptr_sdec_scte = 0;
static struct scte_subt_msg *hdr_sdec_scte = NULL;
static INT32 hdr_buf_len_sdec_scte = 0;
static INT32 hdr_wr_ptr_sdec_scte = 0;
static INT32 hdr_rd_ptr_sdec_scte = 0;
static BOOL g_task_terminate = FALSE;
static ID_THREAD lib_subt_atsc_task_id = OSAL_INVALID_ID;

//function declear
//static UINT16 lib_subt_atsc_parse_simply_bitmap(void);
static BOOL get_bmp_token_bits(UINT16* offset, UINT8* bits_used, UINT8 *data,UINT8 bits_read,UINT8* out_data);
static UINT32 lib_subt_atsc_task_buffer_wr(struct scte_subt_msg *msg, UINT8 *buf,INT32 size);
static INT32 lib_subt_atsc_task_buffer_init(UINT32 param);

void lib_subt_copy_data(UINT32 data, UINT32 length)
{
	if((0xffffffff<=data)||(0xffffffff<=length))
	{
        return ;
    }
    osal_cache_invalidate((UINT8*)data, length);
    if (NULL == g_sec_buf)
   {
       libc_printf("\n%s-%d, error: g_sec_buf == NULL \n",__FUNCTION__,__LINE__);
       return;
   }
   MEMCPY((UINT8*)g_sec_buf, (UINT8*)data, length);
}

#if 1
void lib_subt_atsc_set_buf(struct atsc_subt_config_par *p_config)
{
    SUBT_ATSC_PRINTF("%s-%d\n",__FUNCTION__,__LINE__);
    BOOL bl_addr_range_legal = TRUE;

    if(p_config!=NULL)
    {
        /* Addr legal check */
		#if (defined(_M3715C_) || defined(_M3823C_)) 
		p_config->bs_buf_addr=(UINT8*)__MM_DCII_SUB_BS_START_ADDR;
		p_config->bs_buf_len=__MM_DCII_SUB_BS_LEN;
		//p_config->sec_buf_addr=(UINT8*)__MM_SUBT_ATSC_SEC_ADDR;
		//p_config->sec_buf_len=__MM_SUBT_ATSC_SEC_LEN;
		
		#endif
        bl_addr_range_legal  = osal_check_address_range_legal(p_config->bs_buf_addr, p_config->bs_buf_len);
        bl_addr_range_legal &= osal_check_address_range_legal(p_config->sec_buf_addr, p_config->sec_buf_len);
        if (FALSE == bl_addr_range_legal)
        {
            libc_printf("%s:%d Addr Illegal!\n",__func__,__LINE__);
            return;
        }
		if(p_config->drop_shadow_bottom_from_stream!=FALSE)
			p_config->drop_shadow_bottom_from_stream=FALSE;
		if(p_config->drop_shadow_right_from_stream!=FALSE)
			p_config->drop_shadow_right_from_stream=FALSE;
		if(p_config->outline_thickness_from_stream!=FALSE)
			p_config->outline_thickness_from_stream=FALSE;
		if(p_config->outline_thickness!=2)
			p_config->outline_thickness=2;
		if(p_config->drop_shadow_right!=2)
			p_config->drop_shadow_right=2;
		if(p_config->drop_shadow_bottom!=2)
			p_config->drop_shadow_bottom=2;
        g_subt_atsc_par=*((struct atsc_subt_config_par*)p_config);
        g_sec_buf=g_subt_atsc_par.sec_buf_addr;
        g_sec_buf_len=g_subt_atsc_par.sec_buf_len;
		g_sec_buf_end = g_sec_buf + g_sec_buf_len;
        SUBT_ATSC_PRINTF("%x-g_sec_buf(%x), len(%d)\n",(UINT8*)p_config,g_sec_buf,g_sec_buf_len);
        MEMSET(g_sec_buf,0x00,g_sec_buf_len);
        SUBT_ATSC_PRINTF("%s-Done\n",__FUNCTION__);
        g_subt_atsc_config=TRUE;
    }
    else
    {
        SUBT_ATSC_PRINTF("Oh!Oh! Null atsc_subt_config_par\n");
		return;
    }
}
#else
void lib_subt_atsc_set_buf(void* p_config)
{
    // libc_printf("%s-%d\n",__FUNCTION__,__LINE__);
    if(p_config!=NULL)
    {
        struct atsc_subt_config_par* par=(struct atsc_subt_config_par*)p_config;
        g_sec_buf=par->sec_buf_addr;
        g_sec_buf_len=par->sec_buf_len;
        libc_printf("%x-g_sec_buf(%x), len(%d)\n",(UINT8*)p_config,
        g_sec_buf,g_sec_buf_len);

        MEMSET(g_sec_buf,0x00,g_sec_buf_len);
        libc_printf("%s-%d\n",__FUNCTION__,__LINE__);
        SUBT_ATSC_PRINTF("%s-Done\n",__FUNCTION__);
        libc_printf("%s-%d\n",__FUNCTION__,__LINE__);
    }
    else
    {
        libc_printf("%s-%d\n",__FUNCTION__,__LINE__);
        SUBT_ATSC_PRINTF("Oh!Oh! Null atsc_subt_config_par\n");
        libc_printf("%s-%d\n",__FUNCTION__,__LINE__);
    }
}
#endif
static void lib_subt_atsc_timeout_handle(__MAYBE_UNUSED__ UINT time)
{
	time=0;
    if(lib_subt_atsc_alarm_id != OSAL_INVALID_ID)
    {
        osal_timer_delete(lib_subt_atsc_alarm_id);
        lib_subt_atsc_alarm_id = OSAL_INVALID_ID;
        //lib_subt_atsc_clear_osd(subt_atsc_osd_dev);
        g_lib_subt_atsc_clean_osd = TRUE;
    }
}

static void lib_subt_atsc_start_display_timer(void)
{
    OSAL_T_CTIM  t_dalm;

    MEMSET(&t_dalm,0,sizeof(OSAL_T_CTIM));
    if(lib_subt_atsc_alarm_id != OSAL_INVALID_ID)
    {
        osal_timer_delete(lib_subt_atsc_alarm_id);
        lib_subt_atsc_alarm_id = OSAL_INVALID_ID;
    }

    t_dalm.callback = lib_subt_atsc_timeout_handle;
    t_dalm.type = TIMER_ALARM;
    t_dalm.time  = (( m_sub_theader2.display_duration1 << 8 )|m_sub_theader2.display_duration2)*1000;
    switch(m_sub_theader2.display_standard)
    {
        case TYPE_720_480_30:
            t_dalm.time = t_dalm.time/30;
            break;
        case TYPE_720_576_25:
            t_dalm.time = t_dalm.time/25;
            break;
        case TYPE_1280_720_60:
        case TYPE_1920_1080_60:
            t_dalm.time = t_dalm.time/60;
            break;
        default:
        break;
    }
    if(0==t_dalm.time)
    {
        t_dalm.time = SUBT_DEFAULT_DISPLAY_DURATION;
    }
    if(t_dalm.time < SUBT_DISPLAY_DURATION_LIMITED)
    {
        t_dalm.time = SUBT_DISPLAY_DURATION_LIMITED;
    }
    lib_subt_atsc_alarm_id = osal_timer_create(&t_dalm);
}


// cpu code, recommand move to lib_atsc_subt_buf.c
void lib_subt_atsc_clean_up(void)
{
    
    MEMSET(&m_segment_overlay,0,sizeof(m_segment_overlay));
    m_bitmapdatagot = 0;
}

void lib_subt_atsc_delete_timer()
{
    if(lib_subt_atsc_alarm_id != OSAL_INVALID_ID)
    {
        osal_timer_delete(lib_subt_atsc_alarm_id);
        lib_subt_atsc_alarm_id = OSAL_INVALID_ID;
    }
}
static BOOL lib_subt_atsc_check_crc( UINT8 *data, UINT32 data_len, UINT32 crc)
{
    UINT32 check_crc = 0;
	if((0xFFFFFFFF<=crc)||(0xFFFFFFFF<=data_len))
	{
        return FALSE;
    }	
    if(NULL == data)
    {
        return FALSE;
    }
    mg_setup_crc_table();
    check_crc = (UINT32)mg_table_driven_crc(0xFFFFFFFF,data,data_len);
    return crc == check_crc;
}

// CPU Code, move to lib_atsc_subt_buf.c
UINT16 lib_subt_atsc_stream_identify(UINT16 length,UINT8 *data)
{
    UINT16 offset = 0;
    UINT16 segment_bmp_len =0;
    //UINT32 bmp_len = 0;
    UINT32 crc = 0;
    atsc_subt_fixed_header2 tmp_header2;
    atsc_subt_segment_overlay segment_overlay;
	UINT32 ret_func=0;
    if((NULL == data)||(0xffff<=length))
    {
        return 0;
    }
    MEMSET(&tmp_header2,0,sizeof(atsc_subt_fixed_header2));
    MEMSET(&segment_overlay,0,sizeof(atsc_subt_segment_overlay));
    m_u16bitmap_len = m_bitmapdatagot;
    if((ATSC_SUBT_TABLE_ID == data[offset])&&(length >= ATSC_SUBT_HEAD1_LEN) )// && 0 == m_bitmapdatagot )
    {
        MEMCPY(&crc,(data+length-4),sizeof(crc));
        crc = bswap_constant_32(crc);
        if( !lib_subt_atsc_check_crc(data,length-ATSC_SBUT_CRC_LEN,crc))
        {
            SUBT_ATSC_PRINTF("crc error\n");
            return offset;
        }
        //libc_printf("=====================================****\n");
        MEMCPY(&m_sub_thead1,data,ATSC_SUBT_HEAD1_LEN);
        if(m_sub_thead1.protocol_version == 0 )
        {
            if(m_sub_thead1.segmentation_overlay_included)
            {
                //libc_printf("Segmentation condtion:\n");
                MEMCPY(&segment_overlay,(data+ATSC_SUBT_HEAD1_LEN), ATSC_SEGMENT_OVERLAY_LEN);
                MEMCPY(&tmp_header2,(data+ATSC_SUBT_HEAD1_LEN+ATSC_SEGMENT_OVERLAY_LEN),ATSC_SBUT_HEAD2_LEN);
            }
            else
            {
                //libc_printf("Not segmentation condition:\n");
                MEMCPY(&tmp_header2,(data+ATSC_SUBT_HEAD1_LEN),ATSC_SBUT_HEAD2_LEN);
                tmp_header2.display_in_pts = bswap_constant_32(tmp_header2.display_in_pts);
                tmp_header2.block_length = bswap_constant_16(tmp_header2.block_length);
                MEMCPY(&m_sub_theader2,&tmp_header2,ATSC_SBUT_HEAD2_LEN);
                offset = ATSC_SUBT_HEAD1_LEN+ATSC_SBUT_HEAD2_LEN;
                //bmp_len = (((m_sub_thead1.section_len4 & 0xf)<<8)|m_sub_thead1.section_len8)*
                // ((((m_segment_overlay.last_segment_number_hight & 0xff)<<4)|
                // m_segment_overlay.last_segment_number_low)+1);
                segment_bmp_len = (m_sub_thead1.section_len4<<8|m_sub_thead1.section_len8)-offset+3;
                if(segment_bmp_len > (length - offset - ATSC_SBUT_CRC_LEN))
                {
                    segment_bmp_len = (length - offset - ATSC_SBUT_CRC_LEN);
                }
           
			
				
                MEMSET(m_lpsz_bitmap,0XFF,REASSEMBLY_LENGTH);
                //libc_printf("1m_bitmapdatagot = %d\n",m_bitmapdatagot);
                MEMCPY(m_lpsz_bitmap+m_bitmapdatagot,data+offset,segment_bmp_len);
                m_bitmapdatagot = segment_bmp_len;
                //libc_printf("m_bitmapdatagot = %d\n",m_bitmapdatagot);
                //libc_printf("length = %d\n",length);
                //libc_printf("bmp_len = %d\n",bmp_len);
                //libc_printf("segment_bmp_len = %d\n",segment_bmp_len);
                g_msg.b_info_valid = TRUE;
                g_msg.u_data_size = m_bitmapdatagot;
                MEMCPY(&(g_msg.header1),&m_sub_thead1,ATSC_SUBT_HEAD1_LEN);
                MEMCPY(&(g_msg.header2),&m_sub_theader2,ATSC_SBUT_HEAD2_LEN);
                //libc_printf(" -======> writer data 1\n");
                ret_func=lib_subt_atsc_task_buffer_wr(&g_msg,m_lpsz_bitmap,g_msg.u_data_size);
				if(ret_func!=RET_SUCCESS)
				{
					return 0;
				}
                //lib_subt_atsc_parse_simply_bitmap();
                lib_subt_atsc_clean_up();
                return offset;
            }
            tmp_header2.display_in_pts = bswap_constant_32(tmp_header2.display_in_pts);
            tmp_header2.block_length = bswap_constant_16(tmp_header2.block_length);

           if(((segment_overlay.segment_number_hight & MASK_LOW_FOUR_BIT)
              <<EIGHT_BIT|segment_overlay.segment_number_low) ==ATSC_SUBT_DIVIDE_SEG)
            {
                //libc_printf("The first segment \n");
                MEMCPY(&m_segment_overlay,&segment_overlay,sizeof(m_segment_overlay));
                MEMCPY(&m_sub_theader2,&tmp_header2,ATSC_SBUT_HEAD2_LEN);
                g_msg.b_info_valid = TRUE;
                MEMCPY(&(g_msg.header1),&m_sub_thead1,ATSC_SUBT_HEAD1_LEN);
                MEMCPY(&(g_msg.header2),&m_sub_theader2,ATSC_SBUT_HEAD2_LEN);
                MEMCPY(&(g_msg.overlay),&m_segment_overlay,sizeof(m_segment_overlay));
                m_bitmapdatagot = 0;

               // bmp_len = (( (m_sub_thead1.section_len4 & 0xf)<<8)| m_sub_thead1.section_len8)*
                //((((m_segment_overlay.last_segment_number_hight & 0xff)<<4)|
               // m_segment_overlay.last_segment_number_low)+1);
                
               	MEMSET(m_lpsz_bitmap,0xff,REASSEMBLY_LENGTH);
			   
			
			
            }

            //libc_printf("Message body table extension: ------------
            //( %d )\n",((segment_overlay.table_extension_hight<<8)|
            //segment_overlay.table_extension_low));

            if( (segment_overlay.table_extension_hight ==m_segment_overlay.table_extension_hight)
                && (segment_overlay.table_extension_low== m_segment_overlay.table_extension_low))
            {
                offset = ATSC_SUBT_HEAD1_LEN + ATSC_SEGMENT_OVERLAY_LEN + ATSC_SBUT_HEAD2_LEN;
                segment_bmp_len = (m_sub_thead1.section_len4<<8|m_sub_thead1.section_len8) - offset+3;
                if( ((segment_overlay.segment_number_hight & 0xf)<<8|
                    segment_overlay.segment_number_low) != ATSC_SUBT_DIVIDE_SEG)  // Not the first segment
                {
                    //libc_printf("Current segment is not the first one\n");
                    offset = ATSC_SUBT_HEAD1_LEN + ATSC_SEGMENT_OVERLAY_LEN;
                    segment_bmp_len = (m_sub_thead1.section_len4<<8|m_sub_thead1.section_len8)-offset+3;
                }

                if(segment_bmp_len > (length - offset - ATSC_SBUT_CRC_LEN))
                {
                    segment_bmp_len = (length - offset - ATSC_SBUT_CRC_LEN);
                }

                MEMCPY(m_lpsz_bitmap+m_bitmapdatagot,data+offset,segment_bmp_len);

                m_bitmapdatagot += segment_bmp_len;

                if(((segment_overlay.segment_number_hight & 0xf)<<8|segment_overlay.segment_number_low) ==
                    (((m_segment_overlay.last_segment_number_hight&0xff)<<4)|m_segment_overlay.last_segment_number_low))
                {
                    ret_func=lib_subt_atsc_task_buffer_wr(&g_msg,m_lpsz_bitmap,m_bitmapdatagot);
					if(ret_func!=RET_SUCCESS)
					{
						return 0;
					}
                    lib_subt_atsc_clean_up();
                }
            }
        }

    }

    return offset;//C-WARNING
}


static BOOL lib_subt_atsc_sync2(UINT32 pts)
{
    UINT32 stc_id = 0;
    UINT32 stc = 0;
    UINT32 display_in_pts = 0;
    UINT32 gap0=0;
    UINT32 gap1=0; //remove warngings
    UINT64 pts64 = 0;
    UINT64 stc64 = 0;
	UINT32 flgptn = 0 ;

    if(get_stc(&stc, 0) == RET_SUCCESS)
    {
        stc = (stc<<1);
        gap0 = (pts>=stc)?(pts-stc):(stc-pts);
    }
    stc = 0;
    if(get_stc(&stc, 1) == RET_SUCCESS)
    {
        stc = (stc<<1);
        gap1 = (pts>=stc)?(pts-stc):(stc-pts);
    }
    if(gap0 <= gap1)
    {
        stc_id = 0;
    }
    else
    {
        stc_id = 1;
    }
    //SUBT_ATSC_PRINTF("stc_id = %d\n",stc_id);
    display_in_pts = pts;
    stc = 0;
    while(display_in_pts)
    {
        if(RET_SUCCESS != get_stc(&stc, stc_id))
        {
            SUBT_ATSC_PRINTF("Fail to get_stc(), Assert()\n");
         #if 0
                    ASSERT(0);
        //libc_printf("%s-Fail(%x) to get_stc(%d)-",__FUNCTION__,ret,stc_id);
        //libc_printf("2stc(%x),display_in_pts(%x)\n",stc*2,display_in_pts);
        #else
                return TRUE;    //force leave while loop
        #endif
        }
        stc *= 2;
        if((display_in_pts > stc)&&((display_in_pts - stc)> FREE_RUN_THRESHOLD))
        {
            //libc_printf("xxxxxxxxxxxxxx\n");

            //libc_printf("(display_in_pts - stc) > FREE_RUN_THRESHOLD = %d\n",
            //(display_in_pts - stc - FREE_RUN_THRESHOLD)/90);

            return TRUE;
        }
        if(display_in_pts == stc)
        {
            return TRUE;
        }
        else if(display_in_pts > stc)
        {
            if((display_in_pts - stc) > SYNC_OK_THRESHOLD)  // delay process
            {
                //libc_printf("sync delay1\n");
                osal_flag_wait(&flgptn,scte_sdec_flag_id, SCTE_SDEC_CMD_STOP, OSAL_TWF_ORW, 0);
    		 	if(flgptn&SCTE_SDEC_CMD_STOP) 
    		 	{
    				SUBT_ATSC_PRINTF("%s:receive stop1\n",__FUNCTION__);
    				break;
    		 	}
                osal_task_sleep(1);
            }
        }
        else if(display_in_pts < stc)
        {
            // First pre process
            pts64 = (UINT64)display_in_pts + 0x100000000llu;
            stc64 = (UINT64)stc;

            //Future
            if(((pts64 - stc64) < ATSC_SUBT_PAST_THRESHOLD) &&((pts64-stc64)>SYNC_OK_THRESHOLD))
            {
             //libc_printf("sync delay2\n");
                osal_flag_wait(&flgptn,scte_sdec_flag_id, SCTE_SDEC_CMD_STOP, OSAL_TWF_ORW, 0);
    		 	if(flgptn&SCTE_SDEC_CMD_STOP) 
    		 	{
    				SUBT_ATSC_PRINTF("%s:receive stop2\n",__FUNCTION__);
    				break;
    		 	}
                osal_task_sleep(1);
            }
            else if((pts64 - stc64) >= ATSC_SUBT_PAST_THRESHOLD)  // Past
            {
                return TRUE;
            }
        }
    }
    return FALSE;//   C-warning
}





static UINT16 lib_subt_atsc_display_outline(atsc_subt_bmp_header *bmp_head, struct osdrect *rect)
{
    struct osdrect fill_rect;

    if((NULL == bmp_head) || (NULL == rect))
    {
        return 0;
    }

    MEMSET(&fill_rect,0,sizeof(struct osdrect));
    if(bmp_head->outline_style == OUTLINE_OUTLINE)
    {
        if(TRUE==g_subt_atsc_config)
        {
           if(FALSE==g_subt_atsc_par.outline_thickness_from_stream)
           {
               bmp_head->bmp_outline.outline_thickness =g_subt_atsc_par.outline_thickness;
           }
        }
        else
        {
           bmp_head->bmp_outline.outline_thickness = 2;
        }
       // Left
        fill_rect.u_left = rect->u_left - bmp_head->bmp_outline.outline_thickness;
        fill_rect.u_top = rect->u_top-bmp_head->bmp_outline.outline_thickness;
        fill_rect.u_height = rect->u_height+ 2*bmp_head->bmp_outline.outline_thickness;
        fill_rect.u_width = bmp_head->bmp_outline.outline_thickness;
        lib_subt_atsc_display(&fill_rect,bmp_head,ATSC_SUBT_OUTLINE_COLOR_INDEX);
        fill_rect.u_left = rect->u_left +rect->u_width;   // Right
        lib_subt_atsc_display(&fill_rect,bmp_head,ATSC_SUBT_OUTLINE_COLOR_INDEX);

        fill_rect.u_left = rect->u_left;
        fill_rect.u_top = rect->u_top + rect->u_height;
        fill_rect.u_height = bmp_head->bmp_outline.outline_thickness;
        fill_rect.u_width = rect->u_width; // Bottom
        lib_subt_atsc_display(&fill_rect,bmp_head,ATSC_SUBT_OUTLINE_COLOR_INDEX);

        fill_rect.u_left = rect->u_left;
        fill_rect.u_top = rect->u_top - bmp_head->bmp_outline.outline_thickness;
        fill_rect.u_height = bmp_head->bmp_outline.outline_thickness;
        fill_rect.u_width = rect->u_width;  // Top
        lib_subt_atsc_display(&fill_rect,bmp_head,ATSC_SUBT_OUTLINE_COLOR_INDEX);
    }
    else if(bmp_head->outline_style == OUTLINE_DROP_SHADOW)
    {
        if(TRUE==g_subt_atsc_config)
        {
            if(FALSE==g_subt_atsc_par.drop_shadow_right_from_stream)
            {
                 bmp_head->bmp_drop_shadow.shadow_right =g_subt_atsc_par.drop_shadow_right;
            }
            if(FALSE==g_subt_atsc_par.drop_shadow_bottom_from_stream)
            {
                 bmp_head->bmp_drop_shadow.shadow_bottom = g_subt_atsc_par.drop_shadow_bottom;
            }
        }
        else
        {
            bmp_head->bmp_drop_shadow.shadow_right =2;
            bmp_head->bmp_drop_shadow.shadow_bottom = 2;
        }
        fill_rect.u_left = rect->u_left + bmp_head->bmp_drop_shadow.shadow_right;
        fill_rect.u_top = rect->u_top + bmp_head->bmp_drop_shadow.shadow_bottom;
        fill_rect.u_height = rect->u_height;
        fill_rect.u_width = rect->u_width;
        lib_subt_atsc_display(&fill_rect,bmp_head,ATSC_SUBT_SHADOW_COLOR_INDEX);
    }
    return 0;
}

static UINT16 lib_atsc_subt_get_larger_value(UINT16 m,UINT16 n)
{
    UINT16 ret = 0;

    if(m<n)
    {
        ret = n;
    }
    else
    {
        ret = m;
    }

    return ret;
}

static void atsc_subt_set_char_attribute(atsc_subt_bmp_header *bmp_header,UINT8 *m_lpsz_bitmap2)
{
    if((NULL == bmp_header )||(NULL == m_lpsz_bitmap2))
    {
        return;
    }
    bmp_header->char_color.y_component = (m_lpsz_bitmap2[1] & 0xf8)>>3;
    bmp_header->char_color.opaque_enable =  (m_lpsz_bitmap2[1] &0x4) >>2;
    bmp_header->char_color.cr_component = ((m_lpsz_bitmap2[1]&0x3)<<3)|(m_lpsz_bitmap2[2]&0x7);
    bmp_header->char_color.cb_component = m_lpsz_bitmap2[2] & 0x1f;
    #ifdef SHOW_FORCE_CHAR_COLOR
    bmp_header->char_color.opaque_enable =1;
    #endif
    SUBT_ATSC_PRINTF("Char:Y(%d),op(%d),Cr(%d),Cb(%d)\n",
    bmp_header->char_color.y_component,bmp_header.char_color.opaque_enable,
    bmp_header->char_color.cr_component,bmp_header.char_color.cb_component);
}
static void atsc_subt_set_bitmap_pos(atsc_subt_bmp_header *bmp_header,UINT8 *m_lpsz_bitmap2)
{
    if((NULL == bmp_header) ||(NULL == m_lpsz_bitmap2))
    {
        return;
    }
    bmp_header->bitmap_t_h_coordinate = (m_lpsz_bitmap2[3] << 4) | ((m_lpsz_bitmap2[4] &0xf0)>>4) ;
    bmp_header->bitmap_t_v_coordinate = ((m_lpsz_bitmap2[4] & 0x0f) << 8) | m_lpsz_bitmap2[5];
    bmp_header->bitmap_b_h_coordinate = (m_lpsz_bitmap2[6] << 4) |( (m_lpsz_bitmap2[7] &0xf0)>>4) ;
    bmp_header->bitmap_b_v_coordinate = ((m_lpsz_bitmap2[7] & 0x0f) << 8) | m_lpsz_bitmap2[8];
}

static void atsc_subt_set_bg_frame_attribute(atsc_subt_bmp_header *bmp_header,UINT8 *m_lpsz_bitmap2,UINT16 offset)
{
    if((NULL == bmp_header) ||(NULL == m_lpsz_bitmap2)||(0xffff<=offset))
    {
        return;
    }
    bmp_header->bmp_frame.frame_t_h_coordinate = (m_lpsz_bitmap2[offset] << 4)|((m_lpsz_bitmap2[offset+1] &0xf0)>>4);
    bmp_header->bmp_frame.frame_t_v_coordinate = ((m_lpsz_bitmap2[offset+1] & 0x0f) << 8)|m_lpsz_bitmap2[offset+2];
    bmp_header->bmp_frame.frame_b_h_coordinate = (m_lpsz_bitmap2[offset+3] << 4)|((m_lpsz_bitmap2[offset+4] &0xf0)>>4) ;
    bmp_header->bmp_frame.frame_b_v_coordinate = ((m_lpsz_bitmap2[offset+4] & 0x0f) << 8)|m_lpsz_bitmap2[offset+5];
    SUBT_ATSC_PRINTF("frame coordinate th =%d, tv =%d bh =%d bv =%d\n",
                     bmp_header->bmp_frame.frame_t_h_coordinate,
                     bmp_header->bmp_frame.frame_t_v_coordinate,
                     bmp_header->bmp_frame.frame_b_h_coordinate,
                     bmp_header->bmp_frame.frame_b_v_coordinate);

    bmp_header->bmp_frame.frame_color.y_component = (m_lpsz_bitmap2[offset+6] & 0xf8)>>3;
    bmp_header->bmp_frame.frame_color.opaque_enable = (m_lpsz_bitmap2[offset+6] &0x4) >>2;
    bmp_header->bmp_frame.frame_color.cr_component = ((m_lpsz_bitmap2[offset+6]  & 0x3) <<3) |
              (m_lpsz_bitmap2[offset+7] & 0x7);
    bmp_header->bmp_frame.frame_color.cb_component = m_lpsz_bitmap2[offset+7] & 0x1f;
}

static void atsc_subt_set_outline_attribute(atsc_subt_bmp_header *bmp_header,
    UINT8 *m_lpsz_bitmap2, UINT16 offset)
{
    if((NULL == bmp_header) ||(NULL == m_lpsz_bitmap2)||(0xffff<=offset))
    {
        return;
    }
    bmp_header->bmp_outline.outline_thickness = (m_lpsz_bitmap2[offset] & 0xf) ;
    SUBT_ATSC_PRINTF("outline thickness = %d\n",
    bmp_header->bmp_outline.outline_thickness);

    bmp_header->bmp_outline.outline_color.y_component =( m_lpsz_bitmap2[offset+1] & 0xf8)>>3;
    bmp_header->bmp_outline.outline_color.opaque_enable = (m_lpsz_bitmap2[offset+1] & 0x4)>>2;

    bmp_header->bmp_outline.outline_color.cr_component=((m_lpsz_bitmap2[offset+1]&0x3)<<3)|
        (m_lpsz_bitmap2[offset+2] & 0x7);

    bmp_header->bmp_outline.outline_color.cb_component = m_lpsz_bitmap2[offset+2] & 0x1f;
    SUBT_ATSC_PRINTF("Outline:Y(%d),op(%d),Cr(%d),Cb(%d)\n",
                     bmp_header->bmp_outline.outline_color.y_component,
                     bmp_header->bmp_outline.outline_color.opaque_enable,
                     bmp_header->bmp_outline.outline_color.cr_component,
                     bmp_header->bmp_outline.outline_color.cb_component);
}

static void atsc_subt_set_outline_shadow_attribute(atsc_subt_bmp_header *bmp_header,
       UINT8 *m_lpsz_bitmap2, UINT16 offset)
{
    if((NULL == bmp_header) ||(NULL == m_lpsz_bitmap2)||(0xffff<=offset))
    {
        return;
    }
    bmp_header->bmp_drop_shadow.shadow_right = (m_lpsz_bitmap2[offset] &0xf0) >>4 ;
    bmp_header->bmp_drop_shadow.shadow_bottom = (m_lpsz_bitmap2[offset] & 0x0f);
    SUBT_ATSC_PRINTF("drop shadow with sr:%d and sb:%d\n",bmp_header->bmp_drop_shadow.shadow_right,
                       bmp_header->bmp_drop_shadow.shadow_bottom);
    bmp_header->bmp_drop_shadow.shadow_color.y_component = (m_lpsz_bitmap2[offset+1] & 0xf8)>>3;
    bmp_header->bmp_drop_shadow.shadow_color.opaque_enable = (m_lpsz_bitmap2[offset+1] &0x4) >>2;
    bmp_header->bmp_drop_shadow.shadow_color.cr_component =
        ((m_lpsz_bitmap2[offset+1] & 0x3) <<3) | (m_lpsz_bitmap2[offset+2] & 0x7);
    bmp_header->bmp_drop_shadow.shadow_color.cb_component = (m_lpsz_bitmap2[offset+2] & 0x1f) ;
}

#ifdef SHOW_FORCE_DROP_SHADOW
static void atsc_subt_force_drop_shadow(atsc_subt_bmp_header *bmp_header)
{
    if(NULL == bmp_header)
    {
        FALSE;
    }
    bmp_header->outline_style = OUTLINE_DROP_SHADOW;
    bmp_header->bmp_drop_shadow.shadow_bottom=\
    bmp_header->bmp_drop_shadow.shadow_right=\
    bmp_header->bmp_outline.outline_thickness;
    bmp_header->bmp_drop_shadow.shadow_color.y_component =\
    bmp_header->bmp_outline.outline_color.y_component;
    bmp_header->bmp_drop_shadow.shadow_color.opaque_enable = \
    bmp_header->bmp_outline.outline_color.opaque_enable;
    bmp_header->bmp_drop_shadow.shadow_color.cr_component = \
    bmp_header->bmp_outline.outline_color.cr_component;
    bmp_header->bmp_drop_shadow.shadow_color.cb_component =\
    bmp_header->bmp_outline.outline_color.cb_component;
    bmp_header->bmp_outline.outline_thickness=0;
    bmp_header->bmp_outline.outline_color.y_component=0;
    bmp_header->bmp_outline.outline_color.opaque_enable=0;
    bmp_header->bmp_outline.outline_color.cr_component=0;
    bmp_header->bmp_outline.outline_color.cb_component=0;
}
#endif

static BOOL atsc_subtitle_coordinate_invalid(atsc_subt_bmp_header *bmp_header)
{
    if(NULL == bmp_header)
    {
        return FALSE;
    }
    if( (0 == bmp_header->bitmap_b_h_coordinate) || (0 == bmp_header->bitmap_b_v_coordinate) || \
        (bmp_header->bitmap_b_h_coordinate<bmp_header->bitmap_t_h_coordinate)||\
        (bmp_header->bitmap_b_v_coordinate < bmp_header->bitmap_t_v_coordinate))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void atsc_subt_simple_bitmap_on_part(UINT8 on_pixels,UINT16 *x,UINT16 y,struct osdrect *rect)
{
	if((NULL==rect)||(NULL==x)||(0xffff<=y))
	{
		return;
	}
    if(0 == on_pixels)
    {
        on_pixels = SIMPLY_BITMAP_PART_ON_PIXEL_LEN;
    }
    osd_set_rect(rect, *x, y, on_pixels, 1);
    (*x) = (*x)+rect->u_width;
}

static void atsc_subt_simple_bitmap_off_part(UINT8 off_pixels,UINT16 *x,UINT16 y,struct osdrect *rect)
{
	if((NULL==rect)||(NULL==x)||(0xffff<=y))
	{
		return;
	}
    if(0 == off_pixels)
    {
        off_pixels = SIMPLY_BITMAP_PART_OFF_PIXEL_LEN;
    }
    osd_set_rect(rect, *x, y, off_pixels, 1);
    (*x) = (*x)+rect->u_width;
}

static void atsc_subt_simple_bitmap_on_all(UINT8 on_pixels,UINT16 *x,UINT16 y,struct osdrect *rect)
{
	if((NULL==rect)||(NULL==x)||(0xffff<=y))
	{
		return;
	}
    if(0 == on_pixels)
    {
        on_pixels = SIMPLY_BITMAP_ALL_ON_PIXEL_LEN;
    }
    osd_set_rect(rect, *x, y, on_pixels, 1);
    (*x) = (*x)+rect->u_width;
}


static void atsc_subt_simple_bitmap_off_all(UINT8 off_pixels,UINT16 *x,UINT16 y,struct osdrect *rect)
{
	if((NULL==rect)||(NULL==x)||(0xffff<=y))
	{
		return;
	}
    if(0 == off_pixels)
    {
        off_pixels = SIMPLY_BITMAP_ALL_OFF_PIXEL_LEN;
    }
    osd_set_rect(rect, *x, y, off_pixels, 1);
    (*x) = (*x)+rect->u_width;
}

static RET_CODE lib_subt_atsc_parse_simply_bitmap2(struct scte_subt_msg *msg)
{
    UINT16 offset = 0;
    UINT16 m_u16bitmap_len_local = 0;
    UINT8 *m_lpsz_bitmap2 = NULL;
    UINT8 bmp_operator = 0;
    UINT8 on_pixels = 0;
    UINT8 off_pixels = 0;
    UINT8 steering = 0;
    UINT16 x = 0;
    UINT16 y = 0;
    BOOL re_parse_bitmap = FALSE;
    UINT16 re_parse_offset = 0;
    UINT16 re_parse_bits_used =0;
    UINT16 n_width_real=0;
    struct osd_device *subt_atsc_osd_dev=NULL;
    atsc_subt_fixed_header2 m_subt_header2_local;
    atsc_subt_bmp_header bmp_header;
    struct osdrect rect;

    if(NULL == msg)
    {
        return RET_FAILURE;
    }
    MEMSET(&bmp_header,0,sizeof(atsc_subt_bmp_header));
    m_lpsz_bitmap2 = msg->buff;
    m_u16bitmap_len_local= msg->u_data_size;
    subt_atsc_osd_dev= lib_subt_atsc_get_osd_handle();
    MEMSET(&rect,0,sizeof(struct osdrect));
    MEMSET(&m_subt_header2_local,0,sizeof(atsc_subt_fixed_header2));
    MEMCPY(&m_subt_header2_local,&(msg->header2),ATSC_SBUT_HEAD2_LEN);
    offset = 0;
    MEMSET(&bmp_header,0,sizeof(bmp_header));
    MEMCPY(&(bmp_header),m_lpsz_bitmap2,1);
    atsc_subt_set_char_attribute(&bmp_header,m_lpsz_bitmap2);
    atsc_subt_set_bitmap_pos(&bmp_header,m_lpsz_bitmap2);
    if(TRUE == atsc_subtitle_coordinate_invalid(&bmp_header))
    {
        return RET_FAILURE;    //offset;
    }
    offset += ATSC_SUBT_SIMPLY_BMP_HEAD_LEN;
    if( BG_FRAME==bmp_header.backgroud_style )
    {
        atsc_subt_set_bg_frame_attribute(&bmp_header,m_lpsz_bitmap2,offset);
        offset += ATSC_SUBT_FRAME_LEN;
    }
    #ifdef  SHOW_BY_CPU
    bmp_header.outline_style = OUTLINE_NONE;
    #endif
    if(OUTLINE_OUTLINE== bmp_header.outline_style )
    {
        atsc_subt_set_outline_attribute(&bmp_header,m_lpsz_bitmap2,offset);
    }
    else if( OUTLINE_DROP_SHADOW==bmp_header.outline_style )
    {
       atsc_subt_set_outline_shadow_attribute(&bmp_header,m_lpsz_bitmap2,offset);
    }
    #ifdef SHOW_FORCE_DROP_SHADOW
    atsc_subt_force_drop_shadow(&bmp_header);
    #endif
    offset += ATSC_SUBT_OUTLINE_LEN;
    MEMCPY(&(bmp_header.bmp_length),(m_lpsz_bitmap2+offset),2);
    bmp_header.bmp_length = bswap_constant_16(bmp_header.bmp_length);
    if(bmp_header.bmp_length > m_subt_header2_local.block_length - offset)
    {
        bmp_header.bmp_length = m_subt_header2_local.block_length - offset;
        return RET_FAILURE; //offset;
    }
    offset += 2;
    steering = 0;
    bits_used_in_bytes = 0;
    bmp_consumer_bytes_offset = offset;
    if(m_subt_header2_local.pre_clear_display)
    {
        osd_set_rect(&rect,bmp_header.bitmap_t_h_coordinate,bmp_header.bitmap_t_v_coordinate,\
         (bmp_header.bitmap_b_h_coordinate - bmp_header.bitmap_t_h_coordinate)+OSD_RECT_REDUNDANCY,\
         (bmp_header.bitmap_b_v_coordinate - bmp_header.bitmap_t_v_coordinate)+OSD_RECT_REDUNDANCY);
        if(!lib_subt_atsc_get_osd_rect(m_subt_header2_local.display_standard,&rect))
        {
            return RET_FAILURE;    //offset;
        }
        g_lib_subt_atsc_clean_osd = FALSE;
        lib_subt_atsc_clear_osd(subt_atsc_osd_dev);
        MEMSET(&g_subt_atsc_rect, 0 , sizeof(g_subt_atsc_rect));
        MEMCPY(&g_subt_atsc_rect,&rect,sizeof(g_subt_atsc_rect));
        if((FALSE ==  g_subt_show_on) || (lib_subt_atsc_create_region(subt_atsc_osd_dev,&rect,
             m_subt_header2_local.display_standard) != SUCCESS))
        {
            return RET_FAILURE;    //offset;
        }
    }
    if((!m_subt_header2_local.pre_clear_display) && ( 0== osd_status))
    {
        osd_set_rect(&rect,bmp_header.bitmap_t_h_coordinate,bmp_header.bitmap_t_v_coordinate,\
                    (bmp_header.bitmap_b_h_coordinate - bmp_header.bitmap_t_h_coordinate)+OSD_RECT_REDUNDANCY, \
                    (bmp_header.bitmap_b_v_coordinate - bmp_header.bitmap_t_v_coordinate)+OSD_RECT_REDUNDANCY);
        if(!lib_subt_atsc_get_osd_rect(m_subt_header2_local.display_standard,&rect))
        {
            return RET_FAILURE;    // offset;
        }
        MEMSET(&g_subt_atsc_rect, 0 , sizeof(g_subt_atsc_rect));
        MEMCPY(&g_subt_atsc_rect,&rect,sizeof(g_subt_atsc_rect));
        if(lib_subt_atsc_create_region(subt_atsc_osd_dev,&rect,m_subt_header2_local.display_standard) != SUCCESS)
        {
            return RET_FAILURE;    // offset;
        }
    }
    lib_subt_atsc_osd_set_pallette(subt_atsc_osd_dev,&bmp_header);
    if( BG_FRAME==bmp_header.backgroud_style )
    {
        osd_set_rect(&rect,bmp_header.bmp_frame.frame_t_h_coordinate,bmp_header.bmp_frame.frame_t_v_coordinate,\
                    bmp_header.bmp_frame.frame_b_h_coordinate-bmp_header.bmp_frame.frame_t_h_coordinate,\
                    bmp_header.bmp_frame.frame_b_v_coordinate-bmp_header.bmp_frame.frame_t_v_coordinate);
        if(lib_subt_atsc_get_osd_rect(m_subt_header2_local.display_standard,&rect))
        {
            osddrv_region_fill((HANDLE)subt_atsc_osd_dev,0,&rect,ATSC_SUBT_FRAME_COLOR_INDEX);
        }
    }
    else
    {
        osd_set_rect(&rect,0,0,g_subt_atsc_rect.u_width,g_subt_atsc_rect.u_height);
        lib_subt_atsc_display(&rect,&bmp_header,OFF_COLOR_INDEX);
    }
    x = 0;//bmp_header.bitmap_T_H_coordinate;
    y = 0;//bmp_header.bitmap_T_V_coordinate;
    re_parse_bitmap = FALSE;
    re_parse_offset = bmp_consumer_bytes_offset;
    re_parse_bits_used = bits_used_in_bytes;
    n_width_real=0;
    m_u16bitmap_len_global2 = m_u16bitmap_len_local;
	UINT16 bitmap_h = bmp_header.bitmap_b_h_coordinate - bmp_header.bitmap_t_h_coordinate;
re_parase_simply_bitmap:
	while((offset < (m_u16bitmap_len_local-4)) && (y<=bitmap_h))  //4: CRC_LEN at the end of message data
    {
        if(!get_bmp_token_bits(&bmp_consumer_bytes_offset,&bits_used_in_bytes,m_lpsz_bitmap2,1,&steering))
        {
            break;
        }
        if(1 == steering)
        {
            get_bmp_token_bits(&bmp_consumer_bytes_offset,&bits_used_in_bytes,m_lpsz_bitmap2,3,&on_pixels);
            get_bmp_token_bits(&bmp_consumer_bytes_offset,&bits_used_in_bytes,m_lpsz_bitmap2,5,&off_pixels);
            atsc_subt_simple_bitmap_on_part(on_pixels,&x,y,&rect);
            lib_subt_atsc_display(&rect,&bmp_header,ON_COLOR_INDEX);
            if(!re_parse_bitmap)// Make line drawn to be given thickness
            {
                lib_subt_atsc_display_outline(&bmp_header,&rect);
            }
            atsc_subt_simple_bitmap_off_part(off_pixels,&x,y,&rect);
        }
        else
        {
            if(!get_bmp_token_bits(&bmp_consumer_bytes_offset,&bits_used_in_bytes,m_lpsz_bitmap2,1,&steering))
            {
                break;
            }
            if(1 == steering)
            {
                get_bmp_token_bits(&bmp_consumer_bytes_offset,&bits_used_in_bytes,m_lpsz_bitmap2,6,&off_pixels);
                atsc_subt_simple_bitmap_off_all(off_pixels,&x,y,&rect);
            }
            else
            {
                if(!get_bmp_token_bits(&bmp_consumer_bytes_offset,&bits_used_in_bytes,m_lpsz_bitmap2,1,&steering))
                {
                    break;
                }
                switch(steering)
                {
                case 1:
                get_bmp_token_bits(&bmp_consumer_bytes_offset,&bits_used_in_bytes,m_lpsz_bitmap2,4,&on_pixels);
                atsc_subt_simple_bitmap_on_all(on_pixels,&x,y,&rect);
                lib_subt_atsc_display(&rect,&bmp_header,ON_COLOR_INDEX);
                if(!re_parse_bitmap)
                {
                    lib_subt_atsc_display_outline(&bmp_header,&rect);
                }
                break;
                default:
                bmp_operator = 0;
                get_bmp_token_bits(&bmp_consumer_bytes_offset,&bits_used_in_bytes,m_lpsz_bitmap2,2,&bmp_operator);
                switch(bmp_operator)
                {
                case 0:
                break;
                case 1:
                n_width_real = lib_atsc_subt_get_larger_value(n_width_real,x);
                x += rect.u_width;
                y++;
                globe_height_real = y;
                x = 0;//bmp_header.bitmap_T_H_coordinate;
                break;
                default:
                break;
                }
                break;
                }
            }
        }
        offset = bmp_consumer_bytes_offset;
    }
    if((!re_parse_bitmap)&&((bmp_header.outline_style==OUTLINE_DROP_SHADOW)||
        (bmp_header.outline_style==OUTLINE_OUTLINE)))
    {
         bmp_consumer_bytes_offset = re_parse_offset;
         bits_used_in_bytes = re_parse_bits_used;
         y = 0;
         re_parse_bitmap = !re_parse_bitmap;
         offset = bmp_consumer_bytes_offset;
         goto re_parase_simply_bitmap;
    }
    globe_width_real = n_width_real;
    lib_subt_atsc_show_screen();
    lib_subt_atsc_start_display_timer();
    return RET_SUCCESS;    //    offset;
}


BOOL get_bmp_token_bits(UINT16 *offset, UINT8 *bits_used, UINT8 *data,
                        UINT8 bits_read,UINT8 *out_data)
{
    BOOL ret = FALSE;
	if((NULL==offset)||(NULL==bits_used)||(NULL==data)||(NULL==out_data)||(0xff<=bits_read))
	{
		return FALSE;
	}
    osal_semaphore_capture(scte_semaphore, OSAL_WAIT_FOREVER_TIME);
    UINT8 bits_left = 8-*bits_used;
    UINT8 bits_mask = 0xff;
    UINT8 tmp = 0;

    do
    {
        if(bits_left >= bits_read)
        {
            bits_mask = bits_mask << (8-bits_read);
            *out_data = data[*offset] << *bits_used;
            *out_data = *out_data & bits_mask;
            *out_data = *out_data >> *bits_used;
            *out_data = *out_data >> (8-bits_read-*bits_used);
            *bits_used = *bits_used + bits_read;
        }
        else
        {
            if( (*offset)+1 > m_u16bitmap_len_global2)
            {
                break;
            }
            bits_mask = 0xff;
            bits_mask = (bits_mask << (8-bits_left));
            bits_mask = bits_mask >> (8-bits_left);
            *out_data = data[*offset]  & bits_mask;
            bits_mask = 0xff;
            bits_mask = (bits_mask << (8-(bits_read-bits_left)));
            tmp = ( data[++(*offset)]  & bits_mask ) ;
            tmp = tmp >>(8-(bits_read-bits_left));
            *out_data = ( *out_data ) << (bits_read-bits_left) | tmp;
            *bits_used = bits_read-bits_left;
        }
        ret = TRUE;
    }while(0);

    osal_semaphore_release(scte_semaphore);
    return ret;
}

void lib_subt_atsc_show_onoff(BOOL onoff)
{
    subt_atsc_onoff = onoff;
    INT32 ret_func = 0;

    struct osd_device *subt_atsc_osd_dev= lib_subt_atsc_get_osd_handle();
	if(((TRUE!=onoff)&&(FALSE!=onoff))||(NULL==subt_atsc_osd_dev))
	{
		return;
	}
    if(FALSE == onoff)
    {
        SUBT_ATSC_PRINTF("clear %d\n",__LINE__);
		SUBT_ATSC_PRINTF("lib_subt_atsc_show_off\n");
        g_lib_subt_atsc_clean_osd = FALSE;
		if(1 == osd_status)
		{
        	ret_func =  lib_subt_atsc_clear_osd(subt_atsc_osd_dev);
		}
		if(ret_func!=SUCCESS)
		{
			return;
		}
    }
}

static INT32 lib_subt_atsc_task_buffer_init(__MAYBE_UNUSED__ UINT32 param)
{

    INT32 bs_buf_len_sdec_scte1 = 0;
    INT32 hdr_buf_len_sdec_scte1 = 0;
    UINT8 *bs_buf_sdec_scte1 = NULL;
	param=0;
    if(TRUE==g_subt_atsc_config)
    {
        bs_buf_sdec_scte = g_subt_atsc_par.bs_buf_addr;
        bs_buf_len_sdec_scte = g_subt_atsc_par.bs_buf_len;
    }
    else
    {
        //bs_buf_sdec_scte = bs_buff;
        bs_buf_len_sdec_scte = 16*1024;
        bs_buf_len_sdec_scte1 = bs_buf_len_sdec_scte;
        bs_buf_sdec_scte = MALLOC(bs_buf_len_sdec_scte1);
        if(bs_buf_sdec_scte==NULL)
        {	
            SUBT_ATSC_PRINTF("Fail to alloc mem for atsc subt\n");
            return RET_FAILURE;
        }
    }
    bs_buf_sdec_scte1 = bs_buf_sdec_scte;
    bs_buf_len_sdec_scte1 = bs_buf_len_sdec_scte;
    hdr_buf_len_sdec_scte = 6;
    hdr_buf_len_sdec_scte1 = hdr_buf_len_sdec_scte;
    hdr_sdec_scte = (struct scte_subt_msg*)MALLOC(hdr_buf_len_sdec_scte1*sizeof(struct scte_subt_msg));
    if(hdr_sdec_scte==NULL)
    {	
        SUBT_ATSC_PRINTF("Fail to alloc mem for hdr_sdec_scte\n");
        return RET_FAILURE;
    }
    SUBT_ATSC_PRINTF("%s-%d:Malloc(%d) for hdr_sdec_scte\n",__FUNCTION__,
              __LINE__,hdr_buf_len_sdec_scte*sizeof(struct scte_subt_msg));
    MEMSET(bs_buf_sdec_scte1,0,bs_buf_len_sdec_scte1);
    scte_semaphore = osal_semaphore_create(1);
    osd_status = 0;

    osal_semaphore_capture(scte_semaphore, OSAL_WAIT_FOREVER_TIME);
    bs_wr_ptr_sdec_scte = 0;
    bs_rd_ptr_sdec_scte = 0;

    hdr_wr_ptr_sdec_scte = 0;
    hdr_rd_ptr_sdec_scte = 0;

    osal_semaphore_release(scte_semaphore);
    return RET_SUCCESS;
}

static UINT32 lib_subt_atsc_task_buffer_wr(struct scte_subt_msg *msg, UINT8 *buf, INT32 size)
{
    INT32 hdr_remain_size = 0;
    INT32 hdr_wr_ptr = 0;
    INT32 hdr_rd_ptr = 0;

    INT32 bs_remain_size = 0;
    INT32 bs_wr_ptr = 0;
    INT32 bs_rd_ptr = 0;

    osal_semaphore_capture(scte_semaphore, OSAL_WAIT_FOREVER_TIME);
//   SUBT_ATSC_PRINTF("%s-%d:Scte Size(%d),scte_sdec_status(%d)\n",__FUNCTION__,
    //__LINE__,size,scte_sdec_status); //vicky20110321
    if((NULL==msg)||(NULL==buf)||(0>size))
	{
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
    if(scte_sdec_status != SCTE_SDEC_STATE_PLAY)
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
    hdr_wr_ptr = hdr_wr_ptr_sdec_scte;
    hdr_rd_ptr = hdr_rd_ptr_sdec_scte;

    bs_wr_ptr = bs_wr_ptr_sdec_scte;
    bs_rd_ptr = bs_rd_ptr_sdec_scte;

    if((hdr_wr_ptr>=hdr_buf_len_sdec_scte) ||( hdr_rd_ptr>=hdr_buf_len_sdec_scte)) // hdr error
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }

    if(bs_wr_ptr >=bs_buf_len_sdec_scte ) // bs error 1
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
    if(bs_rd_ptr >= bs_buf_len_sdec_scte)// bs error 2
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
    if(hdr_wr_ptr>=hdr_rd_ptr)
    {
        hdr_remain_size = hdr_buf_len_sdec_scte - (hdr_wr_ptr - hdr_rd_ptr);
    }
    else
    {
        hdr_remain_size = hdr_rd_ptr - hdr_wr_ptr;
    }

    if(hdr_remain_size<=1) //HDR FULL
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
    if(bs_wr_ptr >= bs_rd_ptr)
    {
        bs_remain_size = bs_buf_len_sdec_scte - bs_wr_ptr-1;
        if(bs_remain_size < size)
        {
            bs_remain_size = bs_rd_ptr-1;
        }
    }
    else //bs_wr_ptr < bs_rd_ptr
    {
        bs_remain_size = bs_rd_ptr - bs_wr_ptr-1;
    }

    if(bs_remain_size < size) //BS FULL
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }

    if((bs_wr_ptr>=bs_rd_ptr)&&((bs_buf_len_sdec_scte - bs_wr_ptr-1)<size))
    {
        bs_wr_ptr = 0;
    }

    if(msg->b_info_valid!= TRUE)
    {
      //libc_printf("wr msg->bInfoValid = FALSE\n");
		 return ATSC_RET_FAILURE;//SDBBP();
    }
    MEMCPY(&(hdr_sdec_scte[hdr_wr_ptr]), msg, sizeof(struct scte_subt_msg));
    hdr_sdec_scte[hdr_wr_ptr].buff = bs_buf_sdec_scte+bs_wr_ptr;
    hdr_sdec_scte[hdr_wr_ptr].u_data_size = size;
    MEMCPY(hdr_sdec_scte[hdr_wr_ptr].buff, buf, size);

    hdr_wr_ptr++;
    if(hdr_wr_ptr==hdr_buf_len_sdec_scte)
    {
        hdr_wr_ptr = 0;
    }

    bs_wr_ptr += size;
    if(bs_wr_ptr==bs_buf_len_sdec_scte-1)
    {
        bs_wr_ptr = 0;
    }

    hdr_wr_ptr_sdec_scte =     hdr_wr_ptr;
    bs_wr_ptr_sdec_scte = bs_wr_ptr;

    osal_flag_set(scte_sdec_flag_id,SCTE_SDEC_MSG_DATA_AVAILABLE);

    osal_semaphore_release(scte_semaphore);
    return RET_SUCCESS;
}

static UINT32 lib_subt_atsc_task_buffer_req(struct scte_subt_msg *msg, UINT8 *buff)
{
    INT32 hdr_wr_ptr = 0;
    INT32 hdr_rd_ptr = 0;

    osal_semaphore_capture(scte_semaphore, OSAL_WAIT_FOREVER_TIME);
    if(scte_sdec_status != SCTE_SDEC_STATE_PLAY)
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
	if((NULL==msg)||(NULL==buff))
	{
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
    hdr_wr_ptr = hdr_wr_ptr_sdec_scte;
    hdr_rd_ptr = hdr_rd_ptr_sdec_scte;

    if((hdr_wr_ptr>=hdr_buf_len_sdec_scte) ||
        (hdr_rd_ptr>=hdr_buf_len_sdec_scte))
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
    if((bs_wr_ptr_sdec_scte >=bs_buf_len_sdec_scte) ||
       (bs_rd_ptr_sdec_scte >= bs_buf_len_sdec_scte)) // bs error
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
    if(hdr_wr_ptr == hdr_rd_ptr) //empty
    {
        osal_semaphore_release(scte_semaphore);
        return ATSC_RET_FAILURE;
    }
     // handler
    MEMCPY(msg, &(hdr_sdec_scte[hdr_rd_ptr]), sizeof(struct scte_subt_msg));
    if(msg->b_info_valid!= TRUE)
    {
      //libc_printf("rd msg->bInfoValid = FALSE\n");
		return ATSC_RET_FAILURE;//SDBBP();
    }
    msg->buff = buff;
    MEMCPY(msg->buff, hdr_sdec_scte[hdr_rd_ptr].buff, hdr_sdec_scte[hdr_rd_ptr].u_data_size);

    bs_rd_ptr_sdec_scte = (INT32)(hdr_sdec_scte[hdr_rd_ptr].buff) +
              hdr_sdec_scte[hdr_rd_ptr].u_data_size - (INT32)(bs_buf_sdec_scte);

    hdr_rd_ptr++;
    if(hdr_rd_ptr==hdr_buf_len_sdec_scte)
    {
        hdr_rd_ptr = 0;
    }
    hdr_rd_ptr_sdec_scte = hdr_rd_ptr;

    osal_semaphore_release(scte_semaphore);
    return RET_SUCCESS;
}

static INT32 lib_subt_atsc_task_start(__MAYBE_UNUSED__ INT32 param)
{
    UINT32 flgptn = 0 ;
    OSAL_ER result = 0;
	
	param=0;
	
    if(scte_sdec_status != SCTE_SDEC_STATE_IDLE)
    {
        return ERR_FAILUE;
    }

    osal_flag_set(scte_sdec_flag_id, SCTE_SDEC_CMD_START);
    flgptn = 0;
    result = osal_flag_wait(&flgptn,scte_sdec_flag_id,
             SCTE_SDEC_MSG_START_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
    if(OSAL_E_OK!=result)
    {
        return !RET_SUCCESS;
    }
    osal_flag_clear(scte_sdec_flag_id, SCTE_SDEC_MSG_START_OK);
    return RET_SUCCESS;
}

static INT32 lib_subt_atsc_task_stop(__MAYBE_UNUSED__ INT32 param)
{
    UINT32 flgptn  = 0;
    OSAL_ER result = 0;
    INT32 ret_func = 0;
	
	param=0;
	
    struct osd_device *subt_atsc_osd_dev = lib_subt_atsc_get_osd_handle();

    if(scte_sdec_status != SCTE_SDEC_STATE_PLAY)
    {
    	SUBT_ATSC_PRINTF("%s:scte_sdec_status != SCTE_SDEC_STATE_PLAY\n",__FUNCTION__);
        return ERR_FAILUE;
    }
    osal_flag_set(scte_sdec_flag_id, SCTE_SDEC_CMD_STOP);
    flgptn = 0;
    if (OSAL_INVALID_ID != lib_subt_atsc_task_id)
    {
        result = osal_flag_wait(&flgptn,scte_sdec_flag_id,
             SCTE_SDEC_MSG_STOP_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
        if(OSAL_E_OK!=result)
        {
        	SUBT_ATSC_PRINTF("%s--%d\n",__FUNCTION__,__LINE__);
            return !RET_SUCCESS;
        }
    }

    osal_flag_clear(scte_sdec_flag_id, SCTE_SDEC_MSG_STOP_OK);
	SUBT_ATSC_PRINTF("%s:osd_status=%d\n",__FUNCTION__,osd_status);
    if(1 == osd_status)
    {
        ret_func = lib_subt_atsc_clear_osd(subt_atsc_osd_dev);
		if(ret_func==ERR_FAILURE)
			return !RET_SUCCESS;	
    }
    return RET_SUCCESS;
}


static RET_CODE lib_subt_atsc_data_process(struct scte_subt_msg *msg, UINT8 *buff)
{
    RET_CODE ret = 0;
	UINT32 flgptn = 0 ;

    if((NULL == msg) || (NULL == buff))
    {
        return RET_FAILURE;
    }

    while( RET_SUCCESS== lib_subt_atsc_task_buffer_req(msg,buff))
    {
        //libc_printf("======>> requested data\n");
         osal_flag_wait(&flgptn,scte_sdec_flag_id, SCTE_SDEC_CMD_STOP, OSAL_TWF_ORW, 0);
 		 if(flgptn&SCTE_SDEC_CMD_STOP) 
 		 {
 			SUBT_ATSC_PRINTF("%s:receive stop1\n",__FUNCTION__);
 			break;
 		 }
        if(TRUE== msg->b_info_valid )
        {
            if(!msg->header2.immediate)
            {
                if(lib_subt_atsc_sync2(msg->header2.display_in_pts)
                    != TRUE)// Not procee this msg
                {
                   //libc_printf("++++++++++++++++
                   // Not process this msg because sync\n");
                    continue;
                }
            }
            // osd duration time is not out directly delete
            if (lib_subt_atsc_alarm_id != OSAL_INVALID_ID)
            {
                osal_timer_delete(lib_subt_atsc_alarm_id);
                lib_subt_atsc_alarm_id = OSAL_INVALID_ID;
            }
            /*close osd show*/
            ret=lib_subt_atsc_parse_simply_bitmap2(msg);
            if(ret!=RET_SUCCESS)
            {
                SUBT_ATSC_PRINTF("Fail(%x) to run lib_subt_atsc_parse_simply_bitmap2\n",ret);
            }
         }
     }

    return RET_FAILURE;//get message fail&C-WARNING

}



static void lib_subt_atsc_task(__MAYBE_UNUSED__ UINT32 param1,\
    __MAYBE_UNUSED__ UINT32 param2)
{
    OSAL_ER result = 0;
    UINT32 flgptn = 0 ;
    struct scte_subt_msg msg;
    UINT8 *buff = NULL;
    RET_CODE ret = 0;

	param1=0;
	param2=0;
    MEMSET(&msg,0,sizeof(struct scte_subt_msg));
    struct osd_device *subt_atsc_osd_dev = lib_subt_atsc_get_osd_handle();

    while(1)
    {
        if( SCTE_SDEC_STATE_IDLE==scte_sdec_status )
        {
            result = osal_flag_wait(&flgptn,scte_sdec_flag_id,
                     SCTE_SDEC_CMD_START, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);

            if(flgptn&SCTE_SDEC_CMD_START)
            {
                ret = lib_subt_atsc_task_buffer_init(0);
				ret=(ret==RET_SUCCESS)?RET_SUCCESS:RET_FAILURE;
                buff = MALLOC(3*1024);
                if(NULL == buff)//Malloc failed: need return to the initial station
                {
                    osal_flag_clear(scte_sdec_flag_id, SCTE_SDEC_CMD_START);
                }
                else
                {
                    osal_flag_clear(scte_sdec_flag_id, SCTE_SDEC_CMD_START);
                    osal_flag_set(scte_sdec_flag_id,SCTE_SDEC_MSG_START_OK);
                    msg.b_info_valid = FALSE;
                    scte_sdec_status = SCTE_SDEC_STATE_PLAY;
                }
            }
        }
        else if(SCTE_SDEC_STATE_PLAY == scte_sdec_status)
        {
            if(g_lib_subt_atsc_clean_osd)// First check clear osd or not
            {
                //libc_printf("========>> time out clear osd\n");
                ret = lib_subt_atsc_clear_osd(subt_atsc_osd_dev);
                g_lib_subt_atsc_clean_osd = FALSE;
            }
            // Then check stop flag or data available flag
            result = osal_flag_wait(&flgptn,scte_sdec_flag_id,
                          SCTE_SDEC_CMD_STOP|SCTE_SDEC_MSG_DATA_AVAILABLE,
                          OSAL_TWF_ORW, 100);

            if(OSAL_E_TIMEOUT==result)              // Flag wait time out
            {
                continue;
            }
			
			
            if(flgptn&SCTE_SDEC_MSG_DATA_AVAILABLE) // Data availabel flag
            {
            	SUBT_ATSC_PRINTF("SCTE_SDEC_MSG_DATA_AVAILABLE\n");
                ret =lib_subt_atsc_data_process(&msg,buff);
                //libc_printf("=======> not enought data\n");
                osal_flag_clear(scte_sdec_flag_id,SCTE_SDEC_MSG_DATA_AVAILABLE);
            }
			
			if(flgptn&SCTE_SDEC_CMD_STOP)           // Stop cmd flag
            {
                FREE(hdr_sdec_scte);
                FREE(buff);
                if(TRUE!=g_subt_atsc_config)
                {
                   FREE(bs_buf_sdec_scte);// bs_buf_sdec_scte from Malloc()
                }
				osal_flag_clear(scte_sdec_flag_id, SCTE_SDEC_CMD_STOP);
                             
                osal_flag_set(scte_sdec_flag_id,SCTE_SDEC_MSG_STOP_OK);
				scte_sdec_status = SCTE_SDEC_STATE_IDLE; 
				SUBT_ATSC_PRINTF("scte_sdec_status = SCTE_SDEC_STATE_IDLE\n");
                continue;
            }
        }
    }
}



BOOL create_lib_subt_atsc_task(void)
{
    OSAL_T_CTSK task;
	SUBT_ATSC_PRINTF("%s\n",__FUNCTION__);


	scte_sdec_flag_id = osal_flag_create(0x00000000);
    
	if(OSAL_INVALID_ID==scte_sdec_flag_id)
	{
		return ERR_FAILUE;
	}
    task.itskpri = OSAL_PRI_NORMAL;
	task.task = lib_subt_atsc_task;
	task.quantum = 30;
	task.stksz = 16*1024;
	lib_subt_atsc_task_id = osal_task_create(&task);
	if (lib_subt_atsc_task_id==OSAL_INVALID_ID)
    {
      	SUBT_ATSC_PRINTF("%s-Fail to create lib_subt_atsc_task()",__FUNCTION__);
       	return FALSE;
	}   
    g_task_terminate = FALSE;
	scte_sdec_status = SCTE_SDEC_STATE_IDLE;

    
    return TRUE;
}




BOOL lib_subt_atsc_create_task(void)
{   

    lib_subt_atsc_task_start(0);
    return TRUE;
}

BOOL lib_subt_atsc_terminate_task(void)
{
    g_task_terminate = TRUE;
	SUBT_ATSC_PRINTF("call %s\n",__FUNCTION__);
    lib_subt_atsc_task_stop(0);
   
    return TRUE;
}
#ifdef DUAL_ENABLE

#define SCTE_SDEC_SEC_PARSE_STATE_IDLE        1
#define SCTE_SDEC_SEC_PARSE_STATE_PLAY        2

#define SCTE_SDEC_SEC_PARSE_CMD_START				0x00000001
#define SCTE_SDEC_SEC_PARSE_CMD_STOP				0x00000002
#define SCTE_SDEC_SEC_PARSE_MSG_START_OK			0x00000004
#define SCTE_SDEC_SEC_PARSE_MSG_STOP_OK			0x00000008
#define SCTE_SDEC_SEC_PARSE_MSG_DATA_AVAILABLE	0x00000010
OSAL_ID atsc_subt_sec_parse_flag_id;
UINT32  scte_sdec_sec_parse_status=SCTE_SDEC_SEC_PARSE_STATE_IDLE;


INT32 lib_subt_atsc_sec_parse_task_start(INT32 param)
{
	UINT32 flgptn ;
	OSAL_ER result;
	SUBT_ATSC_PRINTF("\n Enter %s \n",__FUNCTION__);
    if(scte_sdec_sec_parse_status!=SCTE_SDEC_SEC_PARSE_STATE_IDLE)
    {
        return ERR_FAILUE;
    }
    
	osal_flag_set(atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_CMD_START); 
	flgptn = 0;
	result = osal_flag_wait(&flgptn,atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_MSG_START_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
	SUBT_ATSC_PRINTF("%s():sec_parse_flag_id 0x%x \n",__FUNCTION__,atsc_subt_sec_parse_flag_id);
	if(OSAL_E_OK!=result)
	{
        return !RET_SUCCESS;
	}
	osal_flag_clear(atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_MSG_START_OK);
	SUBT_ATSC_PRINTF("Leave %s \n",__FUNCTION__);
    return RET_SUCCESS;
}
	
INT32 lib_subt_atsc_sec_parse_task_stop(INT32 param)
{
	UINT32 flgptn ;
	OSAL_ER result; 
    
    SUBT_ATSC_PRINTF("\n Enter %s \n",__FUNCTION__);
    if(scte_sdec_sec_parse_status != SCTE_SDEC_SEC_PARSE_STATE_PLAY)
    {
        return ERR_FAILUE;
    }
	osal_flag_set(atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_CMD_STOP); 
	flgptn = 0;
	result = osal_flag_wait(&flgptn,atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_MSG_STOP_OK, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME); 
    SUBT_ATSC_PRINTF("%s():sec_parse_flag_id 0x%x \n",__FUNCTION__,atsc_subt_sec_parse_flag_id);
	if(OSAL_E_OK!=result)
	{
        return !RET_SUCCESS;
	}
	osal_flag_clear(atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_MSG_STOP_OK);
	SUBT_ATSC_PRINTF("Leave %s \n",__FUNCTION__);
	return RET_SUCCESS;
} 

static void lib_subt_atsc_sec_parse_task(UINT32 param1, UINT32 param2)
{
	OSAL_ER result;
	UINT32 flgptn ;
	//OSAL_T_CTIM 	t_dalm; 
	//RET_CODE ret;
	UINT32 length = 0;
	UINT8  *data;
	SUBT_ATSC_PRINTF("Enter %s() \n",__FUNCTION__);
	while(1)
	{ 
		if(scte_sdec_sec_parse_status==SCTE_SDEC_SEC_PARSE_STATE_IDLE)
		{
			result = osal_flag_wait(&flgptn,atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_CMD_START, OSAL_TWF_ORW, OSAL_WAIT_FOREVER_TIME);
	
			if(flgptn&SCTE_SDEC_SEC_PARSE_CMD_START)
			{ 
				MEMSET(g_sec_buf,0x00,g_sec_buf_len);
				atsc_subt_sec_parse_create();
				osal_flag_clear(atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_CMD_START);
				osal_flag_set(atsc_subt_sec_parse_flag_id,SCTE_SDEC_SEC_PARSE_MSG_START_OK);
	
				scte_sdec_sec_parse_status = SCTE_SDEC_SEC_PARSE_STATE_PLAY;
			} 
		}
		else if(scte_sdec_sec_parse_status == SCTE_SDEC_SEC_PARSE_STATE_PLAY)
		{
			//SUBT_ATSC_PRINTF("%s() get message \n",__FUNCTION__);
			

			#if 1
			result = osal_flag_wait(&flgptn,atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_CMD_STOP|SCTE_SDEC_SEC_PARSE_MSG_DATA_AVAILABLE, OSAL_TWF_ORW, 100);
			if(OSAL_E_TIMEOUT==result)				// Flag wait time out
			{
				continue;
			}
	

			if(flgptn&SCTE_SDEC_SEC_PARSE_MSG_DATA_AVAILABLE) // Data availabel flag
			{
				//libc_printf("%s() message data available \n",__FUNCTION__);
				length = 1024;
				while(RET_SUCCESS==atsc_subt_sec_parse_rd_req(&length,&data))
				{
					
					//libc_printf("%s():length %d,data=0x%x\n",__FUNCTION__,length,data);
					
					result = osal_flag_wait(&flgptn,atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_CMD_STOP, OSAL_TWF_ORW, 0);

					if(flgptn&SCTE_SDEC_SEC_PARSE_CMD_STOP)	
					{
						SUBT_ATSC_PRINTF("%s-%d\n",__FUNCTION__,__LINE__);
						break;
					}
					
					lib_subt_atsc_stream_identify(length,data);
					//MEMCPY(colin_test1,data,length);
					//colin_test1 +=length;
					MEMSET(data,0x00,1024);
					atsc_subt_sec_parse_rd_update(length);
					length = 1024;
				}
				osal_flag_clear(atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_MSG_DATA_AVAILABLE);
			}
			if(flgptn&SCTE_SDEC_SEC_PARSE_CMD_STOP)			// Stop cmd flag
			{
				SUBT_ATSC_PRINTF("%s() stop get message \n",__FUNCTION__);
				osal_flag_clear(atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_CMD_STOP);
				
				osal_flag_set(atsc_subt_sec_parse_flag_id,SCTE_SDEC_SEC_PARSE_MSG_STOP_OK);

				scte_sdec_sec_parse_status = SCTE_SDEC_SEC_PARSE_STATE_IDLE;
				
				continue;
			}
			
	        #endif
			
		}
	}

	SUBT_ATSC_PRINTF("Leave %s() \n",__FUNCTION__);
}

ID_THREAD lib_subt_atsc_sec_parse_task_id = OSAL_INVALID_ID;

BOOL create_lib_subt_atsc_section_parse_task(void)
{
	OSAL_T_CTSK task;
	SUBT_ATSC_PRINTF("enter %s\n",__FUNCTION__);
	

	
	atsc_subt_sec_parse_flag_id = osal_flag_create(0x00000000);
		
	if(OSAL_INVALID_ID==atsc_subt_sec_parse_flag_id)
	{
		return ERR_FAILUE;
	}
	task.itskpri = OSAL_PRI_NORMAL;
	task.task = lib_subt_atsc_sec_parse_task;
	task.quantum = 30;
	task.stksz = 16*1024;
	lib_subt_atsc_sec_parse_task_id = osal_task_create(&task);
	if (lib_subt_atsc_sec_parse_task_id==OSAL_INVALID_ID)
	{
		SUBT_ATSC_PRINTF("%s-Fail to create lib_subt_atsc_task()",__FUNCTION__);
		return FALSE;
	}	
	
	scte_sdec_sec_parse_status = SCTE_SDEC_SEC_PARSE_STATE_IDLE;
	
	
	return TRUE;
}
BOOL lib_subt_atsc_section_parse_create_task(void)
{
	
	lib_subt_atsc_sec_parse_task_start(0);
	return TRUE;
}



BOOL lib_subt_atsc_section_parse_terminate_task(void)
{
	
	lib_subt_atsc_sec_parse_task_stop(0);
	SUBT_ATSC_PRINTF("%s-scte_sdec_sec_parse_status(%d)\n",__FUNCTION__,scte_sdec_sec_parse_status);	
	
	return TRUE;
}

//static UINT8* atsc_sdec_bs_buf = NULL;
//static UINT32 atsc_sdec_bs_buf_len = 0;
//static UINT8* atsc_sdec_bs_end = NULL;

static UINT8* atsc_sdec_wr_ptr = NULL;
static UINT8* atsc_sdec_rd_ptr = NULL;

INT32 atsc_subt_sec_parse_create(void)
{
	SUBT_ATSC_PRINTF("%s : SBF buffer initialized!\n",__FUNCTION__);
	atsc_sdec_wr_ptr = g_sec_buf;//atsc_sdec_bs_buf;
	atsc_sdec_rd_ptr = g_sec_buf;//atsc_sdec_bs_buf;
	return RET_SUCCESS;
}
//note : buf need 1 byte remaining at least,that means full , or can not differ from empty when rd = wr
__ATTRIBUTE_RAM_
INT32 atsc_subt_sec_parse_wr_req(UINT32 * pSize, UINT8 ** ppData)
{
	UINT32 remain_size;
	UINT8 *cur_wr_ptr ;
	UINT8 *cur_rd_ptr ;

	cur_wr_ptr = atsc_sdec_wr_ptr;
	cur_rd_ptr = atsc_sdec_rd_ptr;
    SUBT_ATSC_PRINTF("%s():wr_ptr=0x%x,rd_ptr=0x%x\n",__FUNCTION__,atsc_sdec_wr_ptr,atsc_sdec_rd_ptr);
	if((cur_rd_ptr >= g_sec_buf_end) ||(cur_wr_ptr >= g_sec_buf_end) )
	{
		SUBT_ATSC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
		return RET_FAILURE;
	}
	if(cur_rd_ptr <= cur_wr_ptr)
	{
		if(cur_rd_ptr != g_sec_buf)
			remain_size = g_sec_buf_end- cur_wr_ptr;  // remain space
		else
			remain_size = g_sec_buf_end - cur_wr_ptr -1;
		//remain_size = g_sec_buf_len;
	}
	else
	{

		remain_size = cur_rd_ptr - cur_wr_ptr -1;//remain 1 space make wr not equal rd
		//SUBT_ATSC_PRINTF("%s():Not buffer because the last data not be read\n",__FUNCTION__);
	    //return RET_FAILURE;
	}

	if(((INT32)remain_size >= 0)&&(remain_size<1024))
	{
		SUBT_ATSC_PRINTF("%s : Warnning BS buffer full!\n",__FUNCTION__);
		return RET_FAILURE;
	}
	*pSize  = (remain_size >= *pSize)?(*pSize):remain_size;
	*ppData = cur_wr_ptr;	
	return RET_SUCCESS;
}


/**************************************************************************/
__ATTRIBUTE_RAM_
void atsc_subt_sec_parse_wr_update(UINT32 Size)
{
	UINT32 remain_size;
	UINT8 *cur_wr_ptr ;
	UINT8 *cur_rd_ptr ;

	cur_wr_ptr = atsc_sdec_wr_ptr;
	cur_rd_ptr = atsc_sdec_rd_ptr;

	if((cur_rd_ptr >= g_sec_buf_end) ||(cur_wr_ptr >= g_sec_buf_end) )
	{
		SUBT_ATSC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
		return;
	}
	
	
	if(cur_rd_ptr <= cur_wr_ptr)
	{
		if(cur_rd_ptr != g_sec_buf)
			remain_size = g_sec_buf_end - cur_wr_ptr;  // remain space
		else
			remain_size = g_sec_buf_end - cur_wr_ptr -1;
		//remain_size = g_sec_buf_end - cur_wr_ptr; 
	}
	else
	{
		SUBT_ATSC_PRINTF("%s():Not buffer because the last data not be read\n",__FUNCTION__);
	    return;
	}

	if(Size>remain_size)
	{
		SUBT_ATSC_PRINTF("%s : Error: BS buffer full!\n",__FUNCTION__);
		return;
	}
	cur_wr_ptr += 1024;
	if(cur_wr_ptr > g_sec_buf_end-1024)
		cur_wr_ptr = g_sec_buf;
	atsc_sdec_wr_ptr = cur_wr_ptr;
	return ;
}
/***********************************************************************/
__ATTRIBUTE_RAM_
INT32 atsc_subt_sec_parse_rd_req(UINT32 * pSize, UINT8 ** ppData)
{
	UINT32 remain_size;
	UINT8 *cur_wr_ptr ;
	UINT8 *cur_rd_ptr ;
	//libc_printf("r\n");
    UINT32 sec_length;
	cur_wr_ptr = atsc_sdec_wr_ptr;
	cur_rd_ptr = atsc_sdec_rd_ptr;
	//(byte1&0x0F)<<8|byte2
	//libc_printf("%s():wr_ptr=0x%x,rd_ptr=0x%x\n",__FUNCTION__,atsc_sdec_wr_ptr,atsc_sdec_rd_ptr);
	if((cur_rd_ptr >= g_sec_buf_end) ||(cur_wr_ptr >= g_sec_buf_end) )
	{
		SUBT_ATSC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
		return RET_FAILURE;
	}

	if(cur_rd_ptr <= cur_wr_ptr)
	{
		remain_size = cur_wr_ptr - cur_rd_ptr;
	}
	else// if(cur_rd_ptr > cur_wr_ptr)
	{
		remain_size = g_sec_buf_end - cur_rd_ptr;
		//SUBT_ATSC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
		//return RET_FAILURE;
	}

	if(ATSC_SUBT_TABLE_ID != cur_rd_ptr[0] )
	{
		//libc_printf("%s() not useful data\n",__FUNCTION__);
		return RET_FAILURE;
	}
	
	if(remain_size <1024)
	{
		//libc_printf("%s : Warnning BS buffer empty!\n",__FUNCTION__);
		return RET_FAILURE;
	}
    //((cur_rd_ptr[1]&0x0F)<<8)|cur_rd_ptr[2]
    sec_length = ((cur_rd_ptr[1]&0x0f)<<8)|cur_rd_ptr[2];
	sec_length += 3;
	*pSize  = sec_length;//(remain_size >= *pSize)?(*pSize):remain_size;
	SUBT_ATSC_PRINTF("%s():cur_rd_ptr 0x%x\n",__FUNCTION__,cur_rd_ptr);
	*ppData = cur_rd_ptr;
	//libc_printf("R\n");
	return RET_SUCCESS;
}

/************************************************************************/
__ATTRIBUTE_RAM_
void atsc_subt_sec_parse_rd_update(UINT32 Size)
{
	UINT32 remain_size;
	UINT8 *cur_wr_ptr ;
	UINT8 *cur_rd_ptr ;
	//libc_printf("a\n");

	cur_wr_ptr = atsc_sdec_wr_ptr;
	cur_rd_ptr = atsc_sdec_rd_ptr;

	if((cur_rd_ptr >= g_sec_buf_end) ||(cur_wr_ptr >= g_sec_buf_end) )
	{
		SUBT_ATSC_PRINTF("%s : Error: read or write ptr out of range!\n",__FUNCTION__);
		return;
	}	
	
	if(cur_rd_ptr <= cur_wr_ptr)
	{
		remain_size = cur_wr_ptr - cur_rd_ptr;
	}
	else// if(cur_rd_ptr > cur_wr_ptr)
	{
		remain_size = g_sec_buf_end - cur_rd_ptr;
	}
	
	//PRINTF("wr_ptr = %x\n",cur_wr_ptr);
	
	if(Size>remain_size)
	{
		SUBT_ATSC_PRINTF("%s : Error: data not enough!\n",__FUNCTION__);
		return;
	}
	
	cur_rd_ptr += 1024;
	if(cur_rd_ptr > g_sec_buf_end-1024)
		cur_rd_ptr = g_sec_buf;
	atsc_sdec_rd_ptr = cur_rd_ptr;
	//libc_printf("A\n");
	return ;
}

__ATTRIBUTE_RAM_
INT32 atsc_sdec_m3327_request_write(void * pdev,UINT32 uSizeRequested,UINT8** ppuData,UINT32* puSizeGot)
{
	UINT32 uReqSize;
	//libc_printf("w\n");
	if(scte_sdec_sec_parse_status==SCTE_SDEC_SEC_PARSE_STATE_IDLE)
	{
		SUBT_ATSC_PRINTF("%s() ATSC_SDEC_STATE_IDLE!\n",__FUNCTION__);
		return RET_STA_ERR;
	}
	uSizeRequested= (uSizeRequested>1024)?1024:(uSizeRequested);//request only 1K once

	uReqSize= uSizeRequested;
	SUBT_ATSC_PRINTF("\n %s D WR REQ(%x):\n",__FUNCTION__,uSizeRequested);
	if(atsc_subt_sec_parse_wr_req(&uReqSize, ppuData)==RET_FAILURE)
	{
		SUBT_ATSC_PRINTF("%s(),request write FAILURE\n",__FUNCTION__);
		return RET_FAILURE;
	}
	*puSizeGot = (uReqSize>uSizeRequested? uSizeRequested : uReqSize);
	//libc_printf("W\n");
	return RET_SUCCESS;

}
__ATTRIBUTE_RAM_
void atsc_sdec_m3327_update_write(void * pdev,UINT32 uSize)
{
	//libc_printf("u\n");
	if(scte_sdec_sec_parse_status==SCTE_SDEC_SEC_PARSE_STATE_IDLE)
	{
		SUBT_ATSC_PRINTF("%s() SDEC_STATE_IDLE!\n",__FUNCTION__);
		return;
	}
	if(!uSize)return ;

	//SUBT_ATSC_PRINTF("D WR UPD:\n");
	 atsc_subt_sec_parse_wr_update(uSize);
	 SUBT_ATSC_PRINTF("%s():wr upd = %d\n",__FUNCTION__,uSize);
	osal_flag_set(atsc_subt_sec_parse_flag_id, SCTE_SDEC_SEC_PARSE_MSG_DATA_AVAILABLE);
	//libc_printf("U\n");
	return ;
}
#endif
