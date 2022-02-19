/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: atsc_cc_init.h

   *    Description:define the data, structure uisng by the specification of
        ATSC CLOSE CAPTION METHODS FOR BROADCAST CABLE
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef __LLD_VBI_M3327_H__
#define __LLD_VBI_M3327_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>

#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>

#ifdef VBI_DEBUG
#define VBI_ASSERT           ASSERT
#define VBI_PRINTF          libc_printf
#define CCDEC_PRITNF       libc_printf
#else
#define VBI_ASSERT(...)   do{}while(0)
#define VBI_PRINTF(...)   do{}while(0)
#define CCDEC_PRINTF(...)  do{}while(0)
#endif
#define VBI_QUEUE_DEPTH 600//500//400//300


struct vbidata_hdr
{
    BOOL b_info_valid;

    UINT8 u_pes_start;
    // Bit map to specify which parameter in the structure is valid: "1" valid, "0" not invlid .
    // Bit0: STC_ID section valid or not;
    // bit1: Data discontinue or not
    // Bit2: PTS section valid or not;
    // Bit3: uVobuNum and uVobuBaseTime section valid or not.
    // Bit4: Video trick mode or not

    //UINT8 uCtrlByte;
    UINT8   u_stc_id; // System timer counter identify:0: STC 0;1: STC 1;
    UINT32  u_pts;   // Corresponding PTS value for a data block.

    UINT8 *pu_start_point;
    UINT32 u_data_size;   // data size
};

struct vbi_m3327_private
{
    void *priv_addr;
    UINT8 status;

    struct vbidata_hdr  init_hdr;
    struct vbidata_hdr *p_vbiwr_hdr;
    struct vbidata_hdr *p_vbird_hdr;

    UINT32  u_remain_data_size;
    UINT8   *pu_remain_data_ptr ;
    UINT8   u_not_enough_data_flag;

    struct vbi_config_par config_par;
    UINT8   ttx_by_osd;/*TURE : display ttx by OSD*/
    UINT8   cc_by_osd;/*dispaly atsc cc by OSD, 0:off*/ /*!=0xff:dispaly,  */
    UINT16  dtvcc_by_osd;/*dispaly atsc dtv cc by OSD, 0:off*/ /*!0xff:dispaly, */ //hbchen
    void    (*init_ttx_decoder)(void);
    void        (*init_cc_decoder)(struct vbi_device *dev);
    BOOL      (*vbi_line21_push_by_cc)(UINT16 data);
    BOOL      (*vbi_line21_push_by_dtvcc)(UINT16 data); //xing 20070522 for DTVCC
};

/*for cc/dtvcc/eas using==================*/
void ccvbi_m3327_setoutput(struct vbi_device *dev,t_vbirequest *p_vbirequest);   //vicky20101229 dbg rename
INT32 vbi_cc_open(struct vbi_device *dev);
INT32 vbi_cc_stop(struct vbi_device *dev);
INT32 vbi_cc_close(struct vbi_device *dev);
void cc_parsing(void);
void cc_parsing_dtvcc(void);
void eas_text_scroll();
BOOL vbi_line21_push(UINT16 data);
BOOL vbi_line21_push_dtvcc(UINT16 data);
void vbi_m3327_init_cc_decoder(struct vbi_device *dev);
/*===============================*/
INT32 vbi_cc_init(void);

#ifdef __cplusplus
 }
#endif



#endif /*__LLD_VBI_M3327_H__ */
