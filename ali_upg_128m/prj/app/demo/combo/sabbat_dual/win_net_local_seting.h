 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_net_local_setting.h
*
*    Description:   The setting of local network environment
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_NET_LOCAL_SETING_H_
#define _WIN_NET_LOCAL_SETING_H_
//win_net_local_seting.h
#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER g_win_localsetting;

extern CONTAINER localset_item_con1;
extern CONTAINER localset_item_con2;
extern CONTAINER localset_item_con3;
extern CONTAINER localset_item_con4;
extern CONTAINER localset_item_con5;
extern CONTAINER localset_item_con6;
extern CONTAINER localset_item_con7;

extern TEXT_FIELD localset_item_txtname1;
extern TEXT_FIELD localset_item_txtname2;
extern TEXT_FIELD localset_item_txtname3;
extern TEXT_FIELD localset_item_txtname4;
extern TEXT_FIELD localset_item_txtname5;
extern TEXT_FIELD localset_item_txtname6;
extern TEXT_FIELD localset_item_txtname7;

extern MULTISEL localset_item_dhcp;
extern EDIT_FIELD localset_item_ip_addr;
extern EDIT_FIELD localset_item_subnet_mask;
extern EDIT_FIELD localset_item_gateway;
extern EDIT_FIELD localset_item_dns1;
extern EDIT_FIELD localset_item_dns2;

extern TEXT_FIELD localset_item_line1;
extern TEXT_FIELD localset_item_line2;
extern TEXT_FIELD localset_item_line3;
extern TEXT_FIELD localset_item_line4;
extern TEXT_FIELD localset_item_line5;
extern TEXT_FIELD localset_item_line6;
extern TEXT_FIELD localset_item_line7;

#ifdef __cplusplus
 }
#endif
#endif//_WIN_NET_LOCAL_SETING_H_

