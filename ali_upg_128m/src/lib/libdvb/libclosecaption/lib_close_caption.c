/****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: lib_close_caption.c

   *    Description: control cc/dtv cc decode functions
   *    History:
   *      Date         Author          Version   Comment
   *      ====         ======         =======   =======
   *    1.  2007.09.26           HB Chen           0.2.000       create for atsc
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/
/*******************
* INCLUDE FILES    *
********************/
#include <osal/osal.h>
#include <basic_types.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/dis/vpo.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libtsi/si_types.h>
#include <api/libclosecaption/lib_closecaption.h>
#include <api/libclosecaption/atsc_dtvcc_dec.h>
#include <hld/sdec/sdec.h>
#include "lib_closecaption_internal.h"

/**********************
* PUBLIC FUNCTION  *
***********************/

static void cc_output_proc(UINT8 field_parity , UINT16 data)
{
    struct vpo_io_cc cc;
    RET_CODE ret = 0;

    MEMSET(&cc,0,sizeof(struct vpo_io_cc));

    cc.field_parity = field_parity;
    cc.data= data;
    ret = vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_WRITE_CC, (UINT32)&cc);
	ret=(ret==RET_SUCCESS)?RET_SUCCESS:RET_FAILURE;
}


void disable_process_cc(BOOL b_on)
{
    ccg_disable_process_cc=b_on; //vicky20110304 fix
    osal_task_sleep(50);
}
/********************************************
* NAME: disable_vbioutput
*       set vbi output or not
*
* Returns : none
* Parameter                     Flow    Description
*
* ------------------------------------------------------------------------------
* BOOL bOn                  IN  Disable vbi output. TRUE: Disable, False: vbi output
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
//void disable_vbioutput(BOOL b_on)
//{
//    g_disable_vbioutput=b_on;
//}

//vicky20110113
/********************************************
* NAME: cc_vbi_show_on
*       set vbi display or not
*
* Returns : none
* Parameter                     Flow    Description
*
* ------------------------------------------------------------------------------
* BOOL bOn                  IN  Show CC via VBI. TRUE: Show, False: Not Show
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void cc_vbi_show_on(BOOL b_on)
{
    UINT16 bdata1 = 0;
    UINT16 bdata2 = 0;

    if(b_on)
    {
        disable_vbioutput(FALSE);
    }
    else
    {
        disable_vbioutput(TRUE);
        //Erase Channel 1
        bdata1=CMD_ERASE_DISPLAYED_MEMORY_NONDIS_CH1;
        bdata2=CMD_ERASE_DISPLAYED_MEMORY_DIS;
        cc_output_proc(0,(bdata2<<8)|bdata1); // 1:top, 0:bot
        cc_output_proc(1,(bdata2<<8)|bdata1); // 1:top, 0:bot
        //Erase Channel 2
        bdata1=CMD_ERASE_DISPLAYED_MEMORY_NONDIS_CH2;
        bdata2=CMD_ERASE_DISPLAYED_MEMORY_DIS;
        cc_output_proc(0,(bdata2<<8)|bdata1); // 1:top, 0:bot
        cc_output_proc(1,(bdata2<<8)|bdata1); // 1:top, 0:bot
    }
}
/********************************************
* NAME: cc_show_onoff
*       Enable cc on/off
*
* Returns : INT32
*              1        : FAIL
*              0        : SUCCESS
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* BOOL bOn                  IN  Enable cc on/off
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
#ifdef  CC_BY_OSD
INT32 atsc_cc_show_onoff(BOOL b_on)
{
    //#ifndef DUAL_ENABLE
    //if((b_on==TRUE)&&(0 ==sys_data_get_cc_control())&&(0==sys_data_get_dtvcc_service())) //fix bug 03371
    //{
    //    return SUCCESS;
    //}
    //#endif
    if ( FALSE== get_eas_onoff())
    {
        CC_VSCR_CLEAN();
        cc_osd_leave();
        if(b_on)
        {
            cc_osd_enter();
            disable_process_cc(FALSE);
        }
        else
        {
            cc_osd_leave();
            disable_process_cc(TRUE);
        }
    }
    return SUCCESS;
}
#endif

/********************************************
* NAME: get_vbi_cgmsa
* Returns : ccvbi_cgmsa
* bit4~3 cgms-a data bit2~1 aps data,spec is CEA-608-B
* Parameter  Flow    Description
* ------------------------------------------------------------------------------*/
UINT8 get_vbi_cgmsa(void)//for cgmsa
{
     return ccvbi_cgmsa;
}

void set_vbi_cgmsa_none()//for cgmsa
{
     ccvbi_cgmsa = 0x00;
     clean_cc_buffer=TRUE;
}
/********************************************
* NAME: get_vbi_rating
*      get vbi rating
*
* Returns : void
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT32* rating                IN/OUT  get vbi rating
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
UINT32 get_vbi_rating(void)
{
     return ccvbi_rating;
}
/********************************************
* NAME: set_vbi_rating_none
*      reset vbi rating
*
* Returns : void
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* None
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void set_vbi_rating_none()
{
     ccvbi_rating=0x00;
     ccpre_vbi_rating=0x00;
     clean_cc_buffer=TRUE;
}
/********************************************
* NAME: get_vbi_cc
*      get vbi cc valid or not
*
* Returns : void
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* BOOL* cc_valid                In/Out  return CC valid or not
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
BOOL get_vbi_cc(void)
{
     return ccvbi_cc;
}
/********************************************
* NAME: set_vbi_cc_invalid
*      reset vbi cc
*
* Returns : void
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* None
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void set_vbi_cc_invalid()
{
     ccvbi_cc=FALSE;
     clean_cc_buffer=TRUE;
}
/********************************************
* NAME: set_cc
*      set cc channel
*
* Returns : INT32
*              1        : FAIL
*              0        : SUCCESS
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT8 cc_channel          0:off, 1-8: cc1~tt4
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
INT32 set_cc(UINT8 cc_channel)
{
    if(cc_channel > CLOSE_CAPTION_NORMAL_8)
    {
        return !SUCCESS;
    }
    if(cc_channel != *ccg_vbi27_cc_by_osd)
    {
        *ccg_vbi27_cc_by_osd = cc_channel;
    }
    return SUCCESS;
}
/********************************************
* NAME: set_dtv_cc
*      set dtv cc otption
*
* Returns : INT32
*              1        : FAIL
*              0        : SUCCESS
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT16 dtv_cc_channel bit0-2:reserved
*                       bit3:dtv cc style, 0:user define, 1:by program
*                       bit4-5: dtv cc font fize: 0:large, 1:middle, 2:small
*                       bit6-8:dtv cc text color:   0:Black,1:Green,2:  Blue,3:Cyan,4:Red,5:Yellow,6:Magenta
*                       bit9-10:dtv cc text opacity: 0:Solid,1:Translucent,2:Transparent
*                       bit11-13:dtv cc bg color:
*                       bit 14-15:dtv cc bg opacity:
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
INT32 set_dtv_cc(UINT16 dtv_cc_channel)
{
    if(dtv_cc_channel != *ccg_vbi27_dtvcc_by_osd)
    {
        *ccg_vbi27_dtvcc_by_osd = dtv_cc_channel;
    }
    return SUCCESS;
}
/********************************************
* NAME: set_dtv_cc_service
*      set dtv cc service
*
* Returns : INT32
*              1        : FAIL
*              0        : SUCCESS
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT8 dtv_cc_service          In  for selecting dtv_cc_service
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
INT32 set_dtv_cc_service(UINT8 dtv_cc_service)
{
    if(dtv_cc_service != g_dtv_cc_service)
    {
        g_dtv_cc_service = dtv_cc_service;
    }
    return SUCCESS;
}
/********************************************
* NAME: set_dtvcc_war
*      set dtv cc wide aspect ratio
*
* Returns : void
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* BOOL dtv_cc_war           In  for setting dtv cc wide aspect ratio
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
void set_dtvcc_war(BOOL dtv_cc_war)//wide aspect ratio
{
     g_dtvcc_war=dtv_cc_war;
}
#ifdef CC_MONITOR_CS
/********************************************
* NAME: set_dtv_cc_service
*      set dtv cc service
*
* Returns : INT32
*              1        : FAIL
*              0        : SUCCESS
* Parameter                     Flow    Description
* ------------------------------------------------------------------------------
* UINT8 dtv_cc_service          In  for selecting dtv_cc_service
* ------------------------------------------------------------------------------
* Additional information:
******************************************/
UINT8 get_dtv_cs_number(void)
{
    return bsn_cc[0];
}

UINT32 get_dtv_cs_list1(void)
{
    UINT8 btotal=bsn_cc[0];
    UINT32 dwlist=0;
    UINT32 dwtemp=0;
    UINT8 bi = 0;

    for(bi=0;bi<btotal;bi++)
    {
        if((bsn_cc[bi+1]>0)&&(bsn_cc[bi+1]<=32))
        {
            dwtemp=0x01;
            dwtemp<<=(bsn_cc[bi+1]-1);
            dwlist|=dwtemp;
        }
    }
    //libc_printf("T(%d)-Cs list1:%x\n",btotal,dwlist);
    return dwlist;
}

UINT32 get_dtv_cs_list2(void)
{
    UINT8 btotal=bsn_cc[0];
    UINT32 dwlist=0;
    UINT32 dwtemp=0;
    UINT8 bi = 0;

    for(bi=0;bi<btotal;bi++)
    {
        if((bsn_cc[bi+1]>0)&&(bsn_cc[bi+1]>32))
        {
            dwtemp=0x01;
            dwtemp<<=(bsn_cc[bi+1]-32-1);
            dwlist|=dwtemp;
        }
    }
    //libc_printf("T(%d)-Cs list2:%x\n",btotal,dwlist);
    return dwlist;
}
#else
UINT8 get_dtv_cs_number(void)
{
    return 0;
}
UINT32 get_dtv_cs_list1(void)
{
    return 0;
}
UINT32 get_dtv_cs_list2(void)
{
    return 0;
}
#endif

#ifdef CC_MONITOR_CC
UINT8 get_cc_control_byte(void)
{
    return bcontrol;
}
#else
UINT8 get_cc_control_byte(void)
{
    return 1;
}
#endif

