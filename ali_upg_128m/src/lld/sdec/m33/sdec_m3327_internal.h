/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sdec_m3327_internal.h
*
*    Description: The file is to define the variable and function that use
     in subtitle decoder
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LLD_SDEC_M3327_INTERNAL_H__
#define __LLD_SDEC_M3327_INTERNAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

extern UINT32 sdec_bs_hdr_buf_len;
extern UINT8 *sdec_bs_buf;
extern UINT8 *sdec_bs_end;
extern UINT32 sdec_bs_buf_len;
extern OSAL_ID sdec_flag_id;
extern UINT16 rle_len;
extern UINT8 *rle_buf;
extern UINT8 cnt4bit;
extern UINT8 cnt2bit;

extern UINT16 g_composition_page_id ;
extern UINT16 g_ancillary_page_id ;

extern struct page_cb g_page_cb;
extern struct region_cb g_region_cb[MAX_REGION_IN_PAGE];
extern UINT8 g_region_idx;

extern UINT8 page_released;
extern UINT8 page_finished;

extern UINT8 subt_clut[256][4];
extern UINT8 subt_clut_id[MAX_REGION_IN_PAGE];

extern struct sdec_data_hdr *p_sdec_rd_hdr;
extern UINT32 sdec_temp_size;
extern UINT32 sdec_temp_size2;
extern BOOL subt_show_on;

extern UINT8 identifier_detected;
extern UINT8 sdec_stuffing_flag;
extern UINT8 sdec_flag_not_enough_data;
extern const UINT8 subt_defalt_clut[MAX_ENTRY_IN_CLUT][4];
extern UINT32 sdec_get_clut;
extern UINT8 *subt_data_buff;
extern UINT16 subt_data_buff_len;

extern BOOL g_is_hdvideo;

extern struct sdec_m3327_private *g_sdec_priv;
extern UINT8 *sdec_temp_buf;
extern UINT32 sdec_temp_buf_len;
extern struct sdec_data_hdr *sdec_bs_hdr_buf;
extern UINT8 subt_transparent_color;
extern UINT8  sdec_tsk_qtm;
extern UINT8  sdec_tsk_pri;

#if 0
//Subt memmory map
#define __MM_SUB_BS_START_ADDR		(__MM_DB_UPDATE_BUF_ADDR -__MM_SUB_BS_LEN)
#define __MM_SUB_PB_START_ADDR		(__MM_SUB_BS_START_ADDR-__MM_SUB_PB_LEN)
#define __MM_SUB_TMP_ADDR		    (__MM_SUB_PB_START_ADDR-__MM_SUB_TMP_LEN)
#define __MM_SUB_DATA_HEAD_ADDR		(__MM_SUB_TMP_ADDR-__MM_SUB_DATA_HEAD_LEN)
#else
//#define __MM_SUB_BS_START_ADDR		0//(__MM_DB_UPDATE_BUF_ADDR -__MM_SUB_BS_LEN)
//#define __MM_SUB_PB_START_ADDR		0//(__MM_SUB_BS_START_ADDR-__MM_SUB_PB_LEN)
#define __MM_SUB_TMP_ADDR		    		0//(__MM_SUB_PB_START_ADDR-__MM_SUB_TMP_LEN)
#define __MM_SUB_DATA_HEAD_ADDR		0//(__MM_SUB_TMP_ADDR-__MM_SUB_DATA_HEAD_LEN)

#endif

//Subt memmory len
#define __MM_SUB_BS_HEAD_LEN 100

//#define __MM_SUB_BS_LEN		0x8800 //0x8800 //0X4800
//#define __MM_SUB_PB_LEN		0
#define __MM_SUB_TMP_LEN    0xa000 //0x2000
#define __MM_SUB_DATA_HEAD_LEN    (20*__MM_SUB_BS_HEAD_LEN)



#ifdef __cplusplus
}
#endif

#endif

