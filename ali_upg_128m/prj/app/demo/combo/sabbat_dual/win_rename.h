/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_rename.c
*
*    Description: To realize the fuction to rename the channel
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _WIN_RENAME_H_
#define _WIN_RENAME_H_

#ifdef __cplusplus
extern "C"{
#endif

extern CONTAINER g_win_chan_rename;

extern CONTAINER chanrn_item1;  /* Name */

extern TEXT_FIELD chanrn_title;
//TEXT_FIELD chanrn_txt1;
extern TEXT_FIELD chanrn_btntxt_ok;
extern TEXT_FIELD chanrn_btntxt_cancel;
extern TEXT_FIELD chanrn_btntxt_caps;
extern TEXT_FIELD chanrn_btntxt_del;

extern BITMAP  chanrn_btnbmp_caps;
extern BITMAP  chanrn_btnbmp_del;
extern BITMAP  chanrn_btnbmp_ok;
extern BITMAP  chanrn_btnbmp_cancel;

extern EDIT_FIELD chanrn_edt1;  /* Name */

extern BITMAP     name_editflag_bmp;
extern TEXT_FIELD name_editfla_txt;

typedef INT32 (*check_name_func)(UINT16* set_name);


UINT32 win_rename_open(UINT16* set_name,UINT8** ret_name,check_name_func check_func );

UINT32 win_chanrename_open(UINT32 prog_id,UINT8** name);
UINT32 win_recrename_open(UINT16* prog_name,UINT8** name);

BOOL win_keyboard_rename_open(UINT16* set_name,UINT8** ret_name,check_name_func check_func);

UINT32 win_makefolder_open(UINT16 *set_name,UINT8 **ret_name,check_name_func check_func );
void draw_caps_flag(UINT16 x, UINT16 y, UINT32 mode); /*  mode : 0 -hide, 1 -draw*/
void draw_caps_del_colbtn(UINT16 x, UINT16 y, UINT32 mode);
void draw_ok_cancel_colbtn(UINT16 x, UINT16 y, UINT32 mode);
void draw_caps_colbtn    (UINT16 x, UINT16 y, UINT32 mode);
void draw_del_colbtn    (UINT16 x, UINT16 y, UINT32 mode);
void draw_ok_colbtn        (UINT16 x, UINT16 y, UINT32 mode);
void draw_cancel_colbtn(UINT16 x, UINT16 y, UINT32 mode);
void draw_ok_colbtn_ext(UINT16 x, UINT16 y, UINT32 mode,UINT16 bmpidx);
void draw_del_colbtn_ext(UINT16 x, UINT16 y, UINT32 mode,UINT16 bmpidx);
void draw_caps_colbtn_ext(UINT16 x, UINT16 y, UINT32 mode,UINT16 bmpidx);
void draw_cancel_colbtn_ext(UINT16 x, UINT16 y, UINT32 mode,UINT16 bmpidx);

#ifdef __cplusplus
}
#endif

#endif//_WIN_RENAME_H_

