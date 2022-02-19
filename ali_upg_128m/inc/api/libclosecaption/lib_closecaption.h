/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: lib_closecaption.h

   *    Description:control cc/dtv cc decode functions
   *  History:
   *      Date            Author             Version   Comment
   *      ====            ======         =======   =======
   *  1.  2007.09.26           HB Chen            0.2.000       create for atsc
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/
#ifndef __LIB_CLOSE_CAPTION_H__
#define __LIB_CLOSE_CAPTION_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <basic_types.h>
#include <api/libclosecaption/closecaption_osd.h>

extern INT32 set_cc(UINT8 cc_channel);
extern INT32 set_dtv_cc(UINT16 dtv_cc_channel);
extern INT32 set_dtv_cc_service(UINT8 dtv_cc_service);
extern void disable_process_cc(BOOL b_on);
extern void enable_osd_cc(BOOL b_on);
extern INT32 atsc_cc_show_onoff(BOOL b_on);//vicky20110120
extern UINT8 get_dtv_cs_number(void);//vicky20110210
extern UINT32 get_dtv_cs_list1(void);
extern UINT32 get_dtv_cs_list2(void);
extern UINT8 get_cc_control_byte(void);//vicky20110217
extern void cc_vbi_show_on(BOOL b_on);
extern INT32 vbi_cc_init();
extern void lib_atsc_cc_attach(struct atsc_cc_config_par *pcc_config_par);
extern void set_eas_onoff(BOOL on_off, UINT8 *text, UINT16 len);
extern BOOL get_eas_onoff(void);
extern UINT32 get_vbi_rating(void);
extern BOOL get_vbi_cc(void);
extern UINT8 get_vbi_cgmsa(void);//for cgmsa
extern void disable_vbioutput(BOOL b_on);
extern void set_cc_by_vbi(BOOL b_on);
extern void set_vbi_rating_none(void);
extern void set_vbi_cgmsa_none(void); //for cgmsa
extern void set_vbi_cc_invalid(void);
extern void set_dtvcc_war(BOOL dtv_cc_war);

#ifdef __cplusplus
 }
#endif

#endif /*__LIB_CLOSE_CAPTION_H__  */

