/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: lib_close_caption_remote.c

   *    Description: remote call for LIB_CLOSE_CAPTION module
   *    History:
   *      Date         Author          Version   Comment
   *      ====         ======         =======   =======
   *    1.  20110126 Vicky Hsu        0.1.000        Initial
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/


/****************************************************************************
*
*  ALi Corporation, All Rights Reserved. 2007 Copyright (C)
*
*  File: lib_close_caption_remote.c
*
*  Description: remote call for LIB_CLOSE_CAPTION module
*  History:
*      Date         Author          Version   Comment
*      ====         ======         =======   =======
*  1.  20110126 Vicky Hsu        0.1.000        Initial
****************************************************************************/
/*******************
* INCLUDE FILES    *
********************/
#include <sys_config.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/vbi/vbi_dev.h>
#include <hld/dis/vpo.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libtsi/si_types.h>
#include <api/libclosecaption/lib_closecaption.h>
#include <api/libclosecaption/atsc_cc_init.h>
#include <api/libclosecaption/closecaption_osd.h>
#include <hld/sdec/sdec.h>


#ifdef DUAL_ENABLE
#include <modules.h>
    #if 0
    enum LIB_ATSC_CC_FUNC
    {
        FUNC_VBI_CC_INIT=0,
        FUNC_ATSC_CC_SHOW_ONOFF,
        FUNC_CC_VBI_SHOW_ON,
        FUNC_LIB_ATSC_CC_ATTACH,
        FUNC_SET_CC,
        FUNC_SET_DTV_CC_SERVICE,
        FUNC_SET_DTV_CC,
        FUNC_GET_DTV_CS_NUMBER,
        FUNC_GET_DTV_CS_LIST1,
        FUNC_GET_DTV_CS_LIST2,
        FUNC_GET_CC_CONTROL_BYTE,
        FUNC_SET_EAS_ONOFF,
        FUNC_GET_EAS_ONOFF,
        FUNC_GET_VBI_RETING,
        FUNC_GET_VBI_CC,
        FUNC_GET_VBI_CGMSA, //for cgmsa
        FUNC_DISABLE_VBIOUTPUT,
        FUNC_DISABLE_PROCESS_CC,
        FUNC_SET_VBI_RETING_NONE,
        FUNC_SET_VBI_CGMSA_NONE, //for cgmsa
        FUNC_SET_VBI_CC_INVALID,
        FUNC_SET_DTVCC_WAR,
    };
    #endif
enum LIB_ATSC_CC_FUNC
{
    FUNC_DISABLE_VBIOUTPUT = 0,
    FUNC_ENABLE_CC_BY_VBI,
    FUNC_CCVBI_M33_SET_OUTPUT,
    FUNC_CCVBI_M33_STOP,
    FUNC_CCVBI_M33_CLOSE,
};



#ifndef _LIB_CLOSE_CAPTION_REMOTE       //See Parts

void disable_vbioutput(BOOL b_on);
void set_cc_by_vbi(BOOL b_on);
void ccvbi_m3327_setoutput(struct vbi_device *dev,t_vbirequest *p_vbi_request);
INT32 vbi_cc_stop(struct vbi_device *dev);
INT32 vbi_cc_close(struct vbi_device *dev);


static UINT32 lib_atsc_cc_entery[]=
{
    (UINT32)disable_vbioutput,
    (UINT32)set_cc_by_vbi,
    (UINT32)ccvbi_m3327_setoutput,
	(UINT32)vbi_cc_stop,
	(UINT32)vbi_cc_close,
};

#if 0
    INT32 vbi_cc_init();
    INT32 atsc_cc_show_onoff(BOOL b_on);
    void cc_vbi_show_on(BOOL b_on);
    void lib_atsc_cc_attach(struct atsc_cc_config_par *pcc_config_par);
    INT32 set_cc(UINT8 cc_channel);
    INT32 set_dtv_cc_service(UINT8 dtv_cc_service);
    INT32 set_dtv_cc(UINT16 dtv_cc_channel);
    UINT8 get_dtv_cs_number(void);
    UINT32 get_dtv_cs_list1(void);
    UINT32 get_dtv_cs_list2(void);
    UINT8 get_cc_control_byte(void);
    void set_eas_onoff(BOOL on_off, UINT8 *text, UINT16 len);
    BOOL get_eas_onoff(void);
    UINT32 get_vbi_rating(void);
    BOOL get_vbi_cc(void);
    UINT8 get_vbi_cgmsa(void);//for cgmsa
    void disable_vbioutput(BOOL b_on);
    void disable_process_cc(BOOL b_on);
    void set_vbi_rating_none();
    void set_vbi_cgmsa_none();//for cgmsa
    void set_vbi_cc_invalid();
    void set_dtvcc_war(BOOL dtv_cc_war);

    static UINT32 lib_atsc_cc_entery[]=
    {
        (UINT32)vbi_cc_init,
        (UINT32)atsc_cc_show_onoff,
        (UINT32)cc_vbi_show_on,
        (UINT32)lib_atsc_cc_attach,
        (UINT32)set_cc,
        (UINT32)set_dtv_cc_service,
        (UINT32)set_dtv_cc,
        (UINT32)get_dtv_cs_number,
        (UINT32)get_dtv_cs_list1,
        (UINT32)get_dtv_cs_list2,
        (UINT32)get_cc_control_byte,
        (UINT32)set_eas_onoff,
        (UINT32)get_eas_onoff,
        (UINT32)get_vbi_rating,
        (UINT32)get_vbi_cc,
        (UINT32)get_vbi_cgmsa, //for cgmsa
        (UINT32)disable_vbioutput,
        (UINT32)disable_process_cc,
        (UINT32)set_vbi_rating_none,
        (UINT32)set_vbi_cgmsa_none, //for cgmsa
        (UINT32)set_vbi_cc_invalid,
        (UINT32)set_dtvcc_war,
    };
#endif
    void lib_close_caption_callee(UINT8 *msg)
    {
        os_hld_callee((UINT32)lib_atsc_cc_entery,msg);
    }
#endif

#ifdef _LIB_CLOSE_CAPTION_REMOTE        //CPU Parts
#if 0
    static UINT32 desc_cc_fg_para[] =
    { //desc of pointer para
      1, DESC_STATIC_STRU(0, sizeof(struct atsc_cc_config_par)),
      1, DESC_P_PARA(0, 0, 0),
      //desc of pointer ret
      0,
      0,
    };
#endif

    static UINT32 desc_ccvbi_m33_p_uint32[] =
    { //desc of pointer para
      1, DESC_OUTPUT_STRU(0, 4),
      1, DESC_P_PARA(0, 1, 0),
      //desc of pointer ret
      0,
      0,
    };


void disable_vbioutput(BOOL b_on)
{
    jump_to_func(NULL, os_hld_caller, b_on, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_DISABLE_VBIOUTPUT, NULL);
}

void set_cc_by_vbi(BOOL b_on)
{
    jump_to_func(NULL, os_hld_caller, b_on, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_ENABLE_CC_BY_VBI, NULL);
}    


void ccvbi_m3327_setoutput(struct vbi_device *dev,t_vbirequest *p_vbi_request)
{
    jump_to_func(NULL, os_hld_caller, dev, (LIB_CLOSE_CAPTION_MODULE<<24)|(2<<16)|FUNC_CCVBI_M33_SET_OUTPUT, desc_ccvbi_m33_p_uint32);
}

INT32 vbi_cc_stop(struct vbi_device *dev)
{
	jump_to_func(NULL, os_hld_caller, dev, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_CCVBI_M33_STOP, NULL);
}

INT32 vbi_cc_close(struct vbi_device *dev)
{
    jump_to_func(NULL, os_hld_caller, dev, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_CCVBI_M33_CLOSE, NULL);
}

    #if 0
        INT32 vbi_cc_init(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_VBI_CC_INIT, NULL);
        }
        INT32 atsc_cc_show_onoff(BOOL b_on)
        {

            //#ifdef CC_BY_OSD
            //if((b_on ==TRUE) &&(0 ==sys_data_get_cc_control())&&(0==sys_data_get_dtvcc_service())) //fix bug 02565
            //{
            //    return 0;
            //}
           // #endif
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_ATSC_CC_SHOW_ONOFF, NULL);
        }
        void cc_vbi_show_on(BOOL b_on)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_CC_VBI_SHOW_ON, NULL);
        }
        void lib_atsc_cc_attach(struct atsc_cc_config_par *pcc_config_par)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|     \
                FUNC_LIB_ATSC_CC_ATTACH, desc_cc_fg_para);
        }
        INT32 set_cc(UINT8 cc_channel)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_SET_CC, NULL);
        }
        INT32 set_dtv_cc_service(UINT8 dtv_cc_service)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_SET_DTV_CC_SERVICE, NULL);
        }
        INT32 set_dtv_cc(UINT16 dtv_cc_channel)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_SET_DTV_CC, NULL);
        }
        UINT8 get_dtv_cs_number(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_GET_DTV_CS_NUMBER, NULL);
        }
        UINT32 get_dtv_cs_list1(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_GET_DTV_CS_LIST1, NULL);
        }
        UINT32 get_dtv_cs_list2(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_GET_DTV_CS_LIST2, NULL);
        }
        UINT8 get_cc_control_byte(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_GET_CC_CONTROL_BYTE, NULL);
        }
        void set_eas_onoff(BOOL on_off, UINT8 *text, UINT16 len)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(3<<16)|FUNC_SET_EAS_ONOFF, NULL);
        }
        BOOL get_eas_onoff(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_GET_EAS_ONOFF, NULL);
        }
        UINT32 get_vbi_rating(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_GET_VBI_RETING, NULL);
        }
        BOOL get_vbi_cc(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_GET_VBI_CC, NULL);
        }
        UINT8 get_vbi_cgmsa(void)//for cgmsa
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_GET_VBI_CGMSA, NULL);
        }
       
        void disable_process_cc(BOOL b_on)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_DISABLE_PROCESS_CC, NULL);
        }
        void set_vbi_rating_none(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_SET_VBI_RETING_NONE, NULL);
        }
        void set_vbi_cgmsa_none(void) //for cgmsa
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_SET_VBI_CGMSA_NONE, NULL);
        }
        void set_vbi_cc_invalid(void)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(0<<16)|FUNC_SET_VBI_CC_INVALID, NULL);
        }
        void set_dtvcc_war(BOOL dtv_cc_war)
        {
            jump_to_func(NULL, os_hld_caller, null, (LIB_CLOSE_CAPTION_MODULE<<24)|(1<<16)|FUNC_SET_DTVCC_WAR, NULL);
        }
    #endif
#endif
#endif

