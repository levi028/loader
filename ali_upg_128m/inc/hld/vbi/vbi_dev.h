/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: vbi_dev.h

   *    Description:define the MACRO, the variable and structure uisng by VBI
        device.
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef    __HLD_VBI_DEV_H__
#define __HLD_VBI_DEV_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

#define  VBI_IO  0x00000000

#define IO_VBI_WORK_MODE_SEPERATE_TTX_SUBT              (VBI_IO + 1)
#define IO_VBI_WORK_MODE_HISTORY                        (VBI_IO + 2)
#define IO_VBI_ENGINE_OPEN                              (VBI_IO + 3)
#define IO_VBI_ENGINE_CLOSE                             (VBI_IO + 4)
#define IO_VBI_ENGINE_UPDATE_PAGE                       (VBI_IO + 5)
#define IO_VBI_ENGINE_SHOW_ON_OFF                       (VBI_IO + 6)
#define IO_VBI_ENGINE_SEND_KEY                          (VBI_IO + 7)
#define IO_VBI_ENGINE_GET_STATE                         (VBI_IO + 8)
#define IO_VBI_ENGINE_UPDATE_INIT_PAGE                  (VBI_IO + 9)
#define IO_VBI_ENGINE_UPDATE_SUBT_PAGE                  (VBI_IO + 10)
#define IO_VBI_ENGINE_SET_CUR_LANGUAGE                  (VBI_IO + 11)
#define IO_TTX_USER_DSG_FONT                              (VBI_IO + 12)
#define IO_VBI_SELECT_OUTPUT_DEVICE                     (VBI_IO + 13)
// VBI output to TV Encoder N, N = 0 or 1

#define CALC_POS(page, line, i) ((page-100)*(25*20)+line*20+(i>>1))



typedef void (*t_ttxdec_cbfunc)(UINT16 param1,UINT8 param2);

/*size of PBF_CB = 32 bytes < 36bytes
    reorder some field to delete the hole and decrease the structure size*/
typedef struct PBF_CB
{
        UINT8   valid;
        //header
        UINT8 erase_page : 1; //c4
        UINT8 newsflash : 1; //c5
        UINT8 subtitle : 1; // c6
        UINT8 suppress_header : 1; //c7
        UINT8 update_indicator : 1; //c8
        UINT8 interrupted_sequence : 1; //c9
        UINT8 inhibit_display : 1;  //c10
        UINT8 magazine_serial : 1; //c11

        UINT8 nation_option; // c14    -c12

        UINT8 g0_set;
        UINT8 second_g0_set;
        //UINT8 pack24en;
        UINT8 pack24_exist : 1;
        UINT8 reserved : 7;

        UINT16 page_id;   //100 ~ 899

        //link
        UINT16 link_red;        //100 ~ 899
        UINT16 link_green;
        UINT16 link_yellow;
        UINT16 link_cyan;
        UINT16 link_next;
        UINT16 link_index;

        UINT8 *buf_start;
        //UINT8 complete;  //for rd request
        //UINT8 released;       //for wr request

        UINT32 complement_line;

        UINT16 sub_page_id; //cloud: 0 ~ 79
        UINT32 page_update_tick;
        // store page update time point for some special ts tnntc
} PBF_CB;


/*memory info structure of VBI driver*/
struct vbi_mem_map
{
    /*stream buffer info*/
    UINT32    sbf_start_addr;
    UINT32    sbf_size;

    /*control buffer info*/
    UINT32    data_hdr;

    /*page buffer info*/
    UINT32    pbf_start_addr;
    UINT32    pbf_size;

    /*ttx sub page mem info*/
    UINT32    sub_page_start_addr;
    UINT32    sub_page_size;

    /*packet26 mem info*/
    UINT32    p26_nation_buf_start_addr;
    UINT32    p26_nation_buf_size;
    UINT32    p26_data_buf_start_addr;
    UINT32    p26_data_buf_size;

};

/*feature config structure of VBI driver*/
struct vbi_config_par
{
    BOOL    ttx_by_vbi;
    BOOL     cc_by_vbi;
    BOOL     vps_by_vbi;
    BOOL    wss_by_vbi;

    BOOL     hamming_8_4_enable;
    BOOL    hamming_24_16_enable;

    BOOL     erase_unknown_packet;
    BOOL    parse_packet26_enable;
    BOOL    ttx_sub_page;
    BOOL    user_fast_text;

    struct vbi_mem_map mem_map;
};

struct ttx_page_info
{
    UINT8 num;
    UINT32 page_addr;
};

struct vbi_device
{
    struct vbi_device  *next;  /*next device */
    INT32  type;
    INT8  name[32];
    INT32  flags;

    INT32 busy;

    void *priv;        /* Used to be 'private' but that upsets C++ */

    INT32 (*init) ();
    INT32 (*init_ext)(struct vbi_config_par *);
    INT32 (*open) (struct vbi_device *);
    INT32 (*close) (struct vbi_device *);
    INT32 (*ioctl)(struct vbi_device *, UINT32 , UINT32);
    INT32 (*request_write)
    (struct vbi_device *, UINT32 ,struct control_block *,UINT8 **,UINT32 *);
    void (*update_write)(struct vbi_device *, UINT32);
    void (*setoutput)(struct vbi_device *,t_vbirequest *);

    INT32 (*start) (struct vbi_device *,t_ttxdec_cbfunc);
    INT32 (*stop) (struct vbi_device *); 
    INT32 (*request_page)(struct vbi_device *, UINT16,struct PBF_CB **);
    INT32 (*request_page_up)(struct vbi_device *, UINT16,struct PBF_CB **);
    INT32 (*request_page_down)(struct vbi_device *, UINT16,struct PBF_CB **);
    void (*default_g0_set)(struct vbi_device *, UINT8 );

};

#ifdef __cplusplus
 }
#endif


#endif /*__HLD_VBI_DEV_H__*/


