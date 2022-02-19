/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_m3327.h

   *    Description:define the function ,MACRO, variable using by low layer VBI
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef __LLD_VBI_M3327_H__
#define __LLD_VBI_M3327_H__

#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>

#ifdef VBI_DEBUG
#define VBI_ASSERT             ASSERT
#define VBI_PRINTF            libc_printf
#define CCDEC_PRITNF       libc_printf
#else
#define VBI_ASSERT(...)      do{}while(0)
#define VBI_PRINTF(...)      do{}while(0)
#define CCDEC_PRINTF(...)  do{}while(0)
#endif

#define VBI_CMD_START                            0x00000001
#define VBI_CMD_STOP                              0x00000002
#define VBI_MSG_START_OK                      0x00000100
#define VBI_MSG_STOP_OK                        0x00000200
#define VBI_MSG_DATA_AVAILABLE          0x00010000

#define VBI_STATE_IDLE        0
#define VBI_STATE_PLAY        1

#define RET_OK            0x00
#define RET_NO_DATA    0x01
#define RET_QUEUE_FULL    0x02

#define MAX_CHANNEL_DATA_SIZE 127 //xing 20070522 for DTVCC start
#define MAX_SERVICE_DATA_SIZE 31 //xing 20070522 for DTVCC start

#define CC_VALID_MASK 0x04
#define CC_FIELD_MASK 0x03
#define CC_FIELD_MASK2 0x00
#define VBI_NORMAL_3 3
#define VBI_NORMAL_7 7
#define VBI_XDS_INVALID_VALUE 0x0f
#define VBI_PACKET26_VALUE_1 0x5d
#define VBI_PACKET26_VALUE_2 0x5b
#define USER_DATA_LENGTH_MAX    104//xing for DTVCC
#define USER_DATA_ARRAY_MAX    10

struct vbidata_hdr
{
    BOOL b_info_valid;

    UINT8 u_pes_start;
    // Bit map to specify
    //which parameter in the structure is valid: "1" valid, "0" not invlid .
    // Bit0: STC_ID section valid or not;
    // bit1: Data discontinue or not
    // Bit2: PTS section valid or not;
    // Bit3: uVobuNum and uVobuBaseTime section valid or not.
    // Bit4: Video trick mode or not

    //UINT8    uCtrlByte;
    UINT8    u_stc_id;    // System timer counter identify:0: STC 0;1: STC 1;
    UINT32    u_pts;    // Corresponding PTS value for a data block.

    UINT8 *pu_start_point;
    UINT32 u_data_size;     // data size
};

struct header
{
    UINT8 page;
    UINT8 s1;
    UINT8 s2;
    UINT8 s3;
    UINT8 s4;
    UINT16 control_bits; //bit 4 - 14 (4->c4,5->c5,...,14->c14)
};

#define VBI_QUEUE_DEPTH    (800*5)
struct vbi_data_array_t
{
    //UINT8 valid;
    UINT8 vbi_field;
    UINT8 unit_id;
    UINT8 unit_len;
    UINT8 vbi_data[44];
};

struct vbi_m3327_private
{
    void *priv_addr;
    UINT8 status;

    struct vbidata_hdr  init_hdr;
    struct vbidata_hdr *p_vbiwr_hdr;
    struct vbidata_hdr *p_vbird_hdr;

    UINT32    u_remain_data_size;
    UINT8    *pu_remain_data_ptr ;
    UINT8    u_not_enough_data_flag;
    UINT8    tve_dev_id;

    struct vbi_config_par config_par;
    UINT8    ttx_by_osd;/*TURE : display ttx by OSD*/
    UINT8    cc_by_osd;/*dispaly atsc cc by OSD, 0:off*/ /*!=0xff:dispaly,  */
    UINT16    dtvcc_by_osd;
         /*dispaly atsc dtv cc by OSD, 0:off*/ /*!0xff:dispaly, */ //hbchen
    void     (*init_ttx_decoder)(void);
    void        (*init_cc_decoder)(struct vbi_device *dev);
    BOOL      (*vbi_line21_push_by_cc)(UINT16 data);
    BOOL      (*vbi_line21_push_by_dtvcc)(UINT16 data);
             //xing 20070522 for DTVCC
    BOOL    (*cc_push_field)(UINT8);
};
/*===============xing 20070522 for DTVCC start===============================*/
struct caption_channel_packet
{
    UINT8 sequence_number:2;
    UINT8 packet_size:6;
    UINT8 packet_data[MAX_CHANNEL_DATA_SIZE];

};

struct extend_service_block
{
    UINT8 block_size:5;
    UINT8 service_number:3;
    UINT8 extended_service_number:6;
    UINT8 null_fill:2;
    UINT8 block_data[MAX_SERVICE_DATA_SIZE];

};

struct service_block
{
    UINT8 block_size:5;
    UINT8 service_number:3;
    UINT8 block_data[MAX_SERVICE_DATA_SIZE];

};

extern void request_vbidata(UINT8 field_polar);
extern void vbidec_task(UINT32 param1,UINT32 param2);

#ifdef SEE_CPU
extern struct vbi_device * g_vbi_device;
extern void ttx_eng_update_page_cb(UINT16 param1,UINT8 param2);
#endif

/*===============xing 20070522 for DTVCC end=================================*/

#ifdef __cplusplus
extern "C"
{
#endif

INT32 vbi_m3327_init();
INT32 vbi_m33_attach(struct vbi_config_par *config_par);

/*api for VBI feature, for OSD usage*/
void vbi_enable_ttx_by_osd(struct vbi_device*dev);
void vbi_enable_cc_by_osd(struct vbi_device*dev);

INT32 vbi_m3327_open(struct vbi_device *dev);
INT32 vbi_m3327_close(struct vbi_device *dev);
INT32 vbi_m3327_request_write(struct vbi_device *dev,UINT32 u_size_requested,
    struct control_block* p_tdata_ctrl_blk,UINT8** ppu_data,UINT32* pu_size_got);

void vbi_m3327_update_write(struct vbi_device *dev,UINT32 data_size);
void vbi_m3327_setoutput(struct vbi_device *dev,t_vbirequest *p_vbirequest);

INT32  vbi_m3327_start(struct vbi_device *dev,t_ttxdec_cbfunc p_cbfunc);
INT32  vbi_m3327_stop(struct vbi_device *dev);
RET_CODE vbi_m3327_ioctl(struct vbi_device *dev,UINT32 cmd,UINT32 param);

/*ttx function*/
void parse_ttx_data(UINT8 *data);
INT32 ttx_m3327_request_page(struct vbi_device *dev,UINT16 page_id , struct PBF_CB ** cb );
INT32 ttx_m3327_request_page_up(struct vbi_device *dev,UINT16 page_id , struct PBF_CB ** cb );
INT32 ttx_m3327_request_page_down(struct vbi_device *dev,UINT16 page_id , struct PBF_CB ** cb );
void ttx_m3327_default_g0_set(struct vbi_device *dev, UINT8 default_g0_set);
//static INT32 vbi_m3327_ioctl(struct net_device *dev, INT32 cmd);
#ifdef __cplusplus
}
#endif

BOOL get_b_save_subpage();
void enable_vbi_transfer(BOOL enable);
UINT16 get_inital_page(void);
UINT8 get_inital_page_status(void);

void vbi_set_vbi_data_addr(struct vbi_data_array_t *p_vbi_data_array,UINT32 *cnt_add );
void vbi_queue_set_rd(UINT16 rd);
void set_vbi_status_to_see(UINT32 status);
void set_ttx_by_vbi(BOOL b_on);
void set_tve_id(UINT16 id);

#endif /*__LLD_VBI_M3327_H__ */
