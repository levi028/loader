/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: cec_operation.c
*
* Description:
*     Processing HDMI CEC operation.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include "cec_private.h"

#include <api/libosd/osd_lib.h>    // due to com_api.h used the OSD typedef

#define CEC_TUNER_ADDR_CNT    4

/************************************************************************************************************
* NAME: cec_act_logical_address_allocation
* Returns : SUCCESS/ERR_FAILUE
*
* Parameter     Flow    Description
* ------------------------------------------------------------------------------
* phy_addr     IN        physical address read from EDID.
* Additional information:
* Execute this function when HDMI plug in and get EDID VSDB block (physical address)
************************************************************************************************************/
INT32 cec_act_logical_address_allocation(void)
{
    UINT8 i = 0;
    UINT8 tryindex = 0;
    UINT8 tunerlogicaladdr[CEC_TUNER_ADDR_CNT] = {0};
    BOOL    bfound = FALSE;
    UINT16     physical_address = 0;
    
    CEC_PRINTF("CEC_ACT_Logical_Address_Allocation()\n");

    physical_address = api_cec_get_device_physical_address();

    CEC_PRINTF("Physical Address is %.1x.%.1x.%.1x.%.1x\n", (physical_address&0xF000) >> 12, \
        (physical_address&0x0F00) >> 8, (physical_address&0x00F0) >> 4, physical_address&0x000F);

    if(0xFFFF == physical_address)
    {
        return ERR_FAILUE;    // Device doesn't have avalid physical address (not F.F.F.F)
    }

    tunerlogicaladdr[0]=CEC_LA_TUNER_1;
    tunerlogicaladdr[1]=CEC_LA_TUNER_2;
    tunerlogicaladdr[2]=CEC_LA_TUNER_3;
    tunerlogicaladdr[3]=CEC_LA_TUNER_4;

    //Get Last CEC Logical Address
    switch(api_cec_get_device_logical_address())
    {
        case CEC_LA_TUNER_4:
            tryindex=3;
            break;

        case CEC_LA_TUNER_3:
            tryindex=2;
            break;

        case CEC_LA_TUNER_2:
            tryindex=1;
            break;

        default:
        case CEC_LA_TUNER_1:
            tryindex=0;
            break;
    }

    // Allocate the logical address. Ref. CEC 10.2.1 (Polling from last Logical Address)
    for(i=0; i<CEC_TUNER_ADDR_CNT; i++)
    {
        // [LIB_CEC]: set logical address
        api_cec_set_device_logical_address(tunerlogicaladdr[((tryindex+i)%CEC_TUNER_ADDR_CNT)]);
        // [LIB_HDMI]: set logical address
        api_set_logical_address(tunerlogicaladdr[((tryindex+i)%CEC_TUNER_ADDR_CNT)]);

        if(SUCCESS != tx_cec_msg_polling_message(api_cec_get_device_logical_address()))
        {
            bfound=TRUE;
            break;
        }

    }

    if(bfound)
    {
        api_cec_set_device_physical_address(physical_address);
        CEC_PRINTF("Acquired CEC Logical Address = %d\n",    api_cec_get_device_logical_address());

        // Report the association between its logical and physical addresses by broadcasting <Report Physical Address>
        ap_cec_msg_report_physical_address();
        ap_cec_msg_device_vendor_id();
        ap_cec_msg_give_system_audio_mode_status();
        return SUCCESS;
    }
    else
    {
        // [LIB_CEC]: set logical address
        // Failed All addresses(TUNER 1~TUNER 4) had been allocated!
        api_cec_set_device_logical_address(CEC_LA_BROADCAST); 
        // [LIB_HDMI]: set logical address
        api_set_logical_address(CEC_LA_BROADCAST);

        libc_printf(" Allocate the CEC logical address Failed!!\n");
        return ERR_FAILUE;
    }
}

INT32 cec_feature_one_touch_play(void)
{
    // Send either <Image View On> or <Text View On>
#if 1
    ap_cec_msg_image_view_on(CEC_LA_TV);
#else
    ap_cec_msg_text_view_on(CEC_LA_TV);
#endif

    ap_cec_msg_active_source();

    ap_cec_msg_menu_status(CEC_LA_TV, api_cec_link_get_current_menu_activate_status());

    return RET_SUCCESS;
}


INT32 cec_feature_system_standby_feature_mode(void)
{
    CEC_PRINTF("_CEC_Feature_System_Standby_Feature_Mode()\n");
    api_cec_set_source_active_state(CEC_SOURCE_STATE_IDLE);
    //CECT 11.2.3-1: STB should broadcast <standby> when invoke system standby feature
    tx_cec_msg_system_standby(CEC_LA_BROADCAST);
    tx_cec_msg_inactive_source();
    if(api_cec_get_system_audio_mode_status())
    {
        api_cec_set_system_audio_mode_status(FALSE);
        tx_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, FALSE);
    }
    return SUCCESS;
}

INT32 cec_feature_local_standby_mode(void)
{
    CEC_PRINTF("cec_feature_local_standby_mode()\n");

    api_cec_set_source_active_state(CEC_SOURCE_STATE_IDLE);

    //Use Blocking Mode when enter standby
    tx_cec_msg_inactive_source();

    if(api_cec_get_system_audio_mode_status())
    {
        api_cec_set_system_audio_mode_status(FALSE);
        tx_cec_msg_set_system_audio_mode(CEC_LA_BROADCAST, FALSE);
    }
    return SUCCESS;
}

