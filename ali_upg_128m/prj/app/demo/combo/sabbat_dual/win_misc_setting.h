/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_misc_setting.h
*
*    Description: misc set menu(mainmenu -> system setup -> other).
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _WIN_MISC_SETTING_H_
#define _WIN_MISC_SETTING_H_
//win_misc_setting.h

#ifdef __cplusplus
extern "C"
{
#endif
extern CONTAINER g_win_misc_setting;

extern CONTAINER miscset_item0;
extern CONTAINER miscset_item1;
extern CONTAINER miscset_item2;
extern CONTAINER miscset_item3;
extern CONTAINER miscset_item4;
extern CONTAINER miscset_item5;
extern CONTAINER miscset_item6;
extern CONTAINER miscset_item7;
extern CONTAINER miscset_item8;
extern CONTAINER miscset_item9;
extern CONTAINER miscset_item10;
extern CONTAINER miscset_item11;

extern TEXT_FIELD miscset_txt0;
extern TEXT_FIELD miscset_txt1;
extern TEXT_FIELD miscset_txt2;
extern TEXT_FIELD miscset_txt3;
extern TEXT_FIELD miscset_txt4;
extern TEXT_FIELD miscset_txt5;
extern TEXT_FIELD miscset_txt6;
extern TEXT_FIELD miscset_txt7;
extern TEXT_FIELD miscset_txt8;
extern TEXT_FIELD miscset_txt9;
extern TEXT_FIELD miscset_txt10;
extern TEXT_FIELD miscset_txt11;

extern TEXT_FIELD miscset_line0;
extern TEXT_FIELD miscset_line1;
extern TEXT_FIELD miscset_line2;
extern TEXT_FIELD miscset_line3;
extern TEXT_FIELD miscset_line4;
extern TEXT_FIELD miscset_line5;
extern TEXT_FIELD miscset_line6;
extern TEXT_FIELD miscset_line7;
extern TEXT_FIELD miscset_line8;
extern TEXT_FIELD miscset_line9;
extern TEXT_FIELD miscset_line10;
extern TEXT_FIELD miscset_line11;

extern MULTISEL  miscset_sel0;
extern MULTISEL  miscset_sel1;
extern MULTISEL  miscset_sel2;
extern MULTISEL  miscset_sel3;
extern MULTISEL  miscset_sel4;
extern MULTISEL  miscset_sel5;
extern MULTISEL  miscset_sel6;
extern MULTISEL  miscset_sel7;
extern MULTISEL  miscset_sel8;
extern MULTISEL  miscset_sel9;
extern MULTISEL  miscset_sel10;
extern MULTISEL  miscset_sel11;

#ifdef PLSN_SUPPORT
extern  CONTAINER   miscset_item12;
extern  TEXT_FIELD  miscset_txt12;
extern  TEXT_FIELD  miscset_line12;
extern  MULTISEL    miscset_sel12;
#endif

extern UINT16 onoff_ids[]; //

extern UINT16 lnbpower_sw_ids[]; // lnb power on or off
extern UINT16 antenna_power_sw_ids[]; //   antenna power on or off
extern UINT16 chan_sw_ids[]; // channel play type
extern UINT16 chtype_ids[];  // channel change type
extern UINT16 auto_stdy_onoff_ids[]; // auto standby on or off
extern UINT16 ram_tms_onoff_ids[]; //
extern CHAR  *dvbc_mode_stritem[]; //
extern CHAR  *version_str[]; //
extern UINT16 fsc_onoff_ids[];
extern UINT16 flash_sp_onoff_ids[];

#ifdef __cplusplus
}
#endif

#endif

