 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_copying.c
*
*    Description: copying feature
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#ifdef USB_MP_SUPPORT
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <api/libmp/media_player_api.h>
#include <errno.h>
#include <api/libfs2/libfc.h>
#include "osdobjs_def.h"
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "control.h"
#include "win_com_popup.h"
#include "win_copying.h"
#include "media_control.h"

/*******************************************************************************
*    WINDOW's objects declaration
*******************************************************************************/

/*******************************************************************************
*    Macro definition
*******************************************************************************/
#define COPY_CON_SH_IDX    WSTL_POP_WIN_01_HD
#define COPY_TITLE_SH_IDX    WSTL_POP_TXT_SH_HD
#define B_BG_SH_IDX            WSTL_BARBG_01_8BIT
#define B_MID_SH_IDX        WSTL_NOSHOW_IDX
#define B_SH_IDX            WSTL_BAR_01_8BIT//WSTL_BAR_01_HD
#define COPY_TXT_SH_IDX        WSTL_TEXT_04_HD
#define VACT_EXIT_COPY (VACT_PASS + 1)
#define F_CP_END            0
#define F_CP_FAIL            1
#define F_CP_NEW_FILE        2
#define F_CP_PROGRESS        3
#ifndef SD_UI
#define    WIN_COPY_L            224
#define    WIN_COPY_T            220//200
#define    WIN_COPY_W            560
#define    WIN_COPY_H             200//240
#define    COPY_TITLE_L        (WIN_COPY_L+180)
#define    COPY_TITLE_T        (WIN_COPY_T+20)
#define    COPY_TITLE_W        200
#define    COPY_TITLE_H         40
#define    COPY_FILE_L        (WIN_COPY_L+20)
#define    COPY_FILE_T        (COPY_TITLE_T+COPY_TITLE_H+20)
#define    COPY_FILE_W        (WIN_COPY_W-40)
#define    COPY_FILE_H     40
#define    COPY_BAR_L        (WIN_COPY_L+20)
#define    COPY_BAR_T        (WIN_COPY_T+WIN_COPY_H-60)
#define    COPY_BAR_W        (WIN_COPY_W-130)
#define    COPY_BAR_H         24
#define COPY_TXTP_L        (COPY_BAR_L + COPY_BAR_W)
#define COPY_TXTP_T     (COPY_BAR_T - 6)
#define COPY_TXTP_W     90
#define COPY_TXTP_H     40
#else
#define    WIN_COPY_L            120
#define    WIN_COPY_T            220//200
#define    WIN_COPY_W            400
#define    WIN_COPY_H             200//240
#define    COPY_TITLE_L        (WIN_COPY_L+180)
#define    COPY_TITLE_T        (WIN_COPY_T+20)
#define    COPY_TITLE_W        200
#define    COPY_TITLE_H         40
#define    COPY_FILE_L        (WIN_COPY_L+20)
#define    COPY_FILE_T        (COPY_TITLE_T+COPY_TITLE_H+20)
#define    COPY_FILE_W        (WIN_COPY_W-40)
#define    COPY_FILE_H     40
#define    COPY_BAR_L        (WIN_COPY_L+20)
#define    COPY_BAR_T        (WIN_COPY_T+WIN_COPY_H-60)
#define    COPY_BAR_W        (WIN_COPY_W-130)
#define    COPY_BAR_H         16
#define COPY_TXTP_L        (COPY_BAR_L + COPY_BAR_W)
#define COPY_TXTP_T     (COPY_BAR_T - 6)
#define COPY_TXTP_W     90
#define COPY_TXTP_H     40
#endif

//typedef void(*cp_callback_func)(unsigned long type, unsigned long param); //definition in /api\libfs2\libfc.h



/*******************************************************************************
*    Function decalare
*******************************************************************************/
static void win_filecopy_callback(unsigned long type, unsigned long param);
static void win_copying_init(void);
static PRESULT     copyfile_unkown_act_proc(VACTION act);
static PRESULT filecopy_message_proc(UINT32 msg_type, UINT32 msg_code);
static VACTION copying_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT copying_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

/*******************************************************************************
*    Objects Macro
*******************************************************************************/
#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
        DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, COPY_CON_SH_IDX,COPY_CON_SH_IDX,COPY_CON_SH_IDX,COPY_CON_SH_IDX,   \
        copying_keymap,copying_callback,  \
        nxt_obj, focus_id,0)

#define LDEF_TITLE_TXT(root,var_txt,nxt_obj,l,t,w,h,sh,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_COPY_FILE(root,var_txt,nxt_obj,l,t,w,h,sh,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_COPY_BAR(root,var_bar,nxt_obj,l,t,w,h,rcl,rct,rcw,rch)    \
        DEF_PROGRESSBAR(var_bar, root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0,0,0,0,0, l,t,w,h, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX, B_BG_SH_IDX,\
        NULL, NULL, \
        PROGRESSBAR_HORI_NORMAL|PBAR_STYLE_RECT_STYLE, 0, 0, B_MID_SH_IDX, B_SH_IDX, \
        rcl,rct,rcw,rch, 1, 100, 100, 1)

#define LDEF_COPY_TXT(root,var_txt,nxt_obj,l,t,w,h,sh,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT| C_ALIGN_VCENTER, 0,0,0,str)

/*******************************************************************************
*   WINDOW's objects defintion
*******************************************************************************/
/*******************************************************************************
*variables definition
*******************************************************************************/
static file_list_handle copyfile_cp_list = NULL;
static UINT32 copyfile_cp_idx = 0;
char copyfile_dest_path[FULL_PATH_SIZE] = {0};
BOOL g_from_copyfile = FALSE;

LDEF_WIN(g_win_copying, &copying_title, WIN_COPY_L, WIN_COPY_T, WIN_COPY_W, WIN_COPY_H, 0)

LDEF_TITLE_TXT(&g_win_copying, copying_title, &copying_file,\
        COPY_TITLE_L,  COPY_TITLE_T,  COPY_TITLE_W,  COPY_TITLE_H,  COPY_TITLE_SH_IDX,  RS_COPYING)

LDEF_COPY_FILE(&g_win_copying, copying_file, &copying_bar,\
        COPY_FILE_L,  COPY_FILE_T,  COPY_FILE_W,  COPY_FILE_H,  COPY_TXT_SH_IDX,  display_strs[0])
#ifndef SD_UI
LDEF_COPY_BAR(&g_win_copying,  copying_bar, &copying_percent, \
        COPY_BAR_L, COPY_BAR_T, COPY_BAR_W, COPY_BAR_H, \
        2, 5, COPY_BAR_W, COPY_BAR_H-10)
#else
LDEF_COPY_BAR(&g_win_copying,  copying_bar, &copying_percent, \
        COPY_BAR_L, COPY_BAR_T, COPY_BAR_W, COPY_BAR_H, \
        2, 2, COPY_BAR_W, COPY_BAR_H-4)
#endif
LDEF_COPY_TXT(&g_win_copying, copying_percent, NULL,\
        COPY_TXTP_L,  COPY_TXTP_T,  COPY_TXTP_W,  COPY_TXTP_H,  COPY_TXT_SH_IDX,  display_strs[1])

/*******************************************************************************
*   function defintion
*******************************************************************************/
void set_copy_file_list(file_list_handle *handle)
{
    copyfile_cp_list = handle;
}

void set_copy_file_index(UINT32 index)
{
    copyfile_cp_idx = index;
}
static VACTION copying_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = 0;

    switch(key)
    {
    case V_KEY_EXIT:
        act = VACT_EXIT_COPY;
        break;
    default:
        act = VACT_PASS;
        break;
    }
    return act;
}

static PRESULT copying_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = 0;
    RET_CODE func_ret = RET_SUCCESS;

    switch(event)
    {
       case EVN_PRE_OPEN:
        win_copying_init();
        break;
       case EVN_POST_OPEN:
            func_ret=copy_files_from_file_list(copyfile_cp_list, copyfile_cp_idx, copyfile_dest_path, \
                                            4, win_filecopy_callback);
             switch(func_ret)
             {
                    case RET_FAILURE:
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_COPYFILE_ERR, 1, TRUE);
                       break;
                case RET_STA_ERR:
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_COPYFILE_CANT_COPY, 1, TRUE);
                       break;
                case -16:
                    ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_COPYFILE_OVERWRITE, 1, TRUE);
                           break;
                    default:
                          break;
             }
        break;
    case EVN_PRE_CLOSE:
        g_from_copyfile = TRUE;
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = copyfile_unkown_act_proc(unact);

        break;
    case EVN_MSG_GOT:
        ret = filecopy_message_proc(param1,param2);
        break;
    default:
        break;
    }

    return ret;
}

static void win_copying_init(void)
{
    TEXT_FIELD *p_txt = NULL;
    PROGRESS_BAR *p_bar = NULL;

    p_txt = &copying_file;
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"");
    p_txt = &copying_percent;
    osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"");
    p_bar = &copying_bar;
    osd_set_progress_bar_pos(p_bar,0);
}

static PRESULT     copyfile_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    UINT8 back_saved = 0;

    switch(act)
    {
    case VACT_EXIT_COPY:
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        #ifndef SD_UI
        win_compopup_set_frame(496, 330, 286, 130);
        #else
        win_compopup_set_frame(200, 330, 286, 130);
        #endif
        win_compopup_set_msg("Copy canceled!", NULL,0);
        win_compopup_open_ext(&back_saved);
        f_cancel_cp();
        osal_task_sleep(500);
        win_compopup_smsg_restoreback();
        ret = PROC_LEAVE;
        break;
    default:
        break;
    }
    return ret;
}

static void win_filecopy_callback(unsigned long type, unsigned long param)
{
    UINT32 precent = 0;
    UINT32 newfile = 0;

    switch(type)
    {
        case F_CP_END:
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
            break;
        case F_CP_FAIL:
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_COPYFILE_ERR, param, TRUE);
            break;
        case F_CP_NEW_FILE:
            newfile = (unsigned long)param;
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_COPYFILE_NEWFILE,newfile,FALSE);
            break;
        case F_CP_PROGRESS:
            precent = (unsigned long)param;
            ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_COPYFILE_PROGRESS,precent,FALSE);
            break;
        default:
            break;
    }
}

static PRESULT    filecopy_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
    UINT8 back_saved = 0;
    TEXT_FIELD *p_file = NULL;
    TEXT_FIELD *p_txt = NULL;
    PROGRESS_BAR *p_bar = NULL;
    char *str_buff = NULL;
    win_popup_choice_t choice = WIN_POP_CHOICE_NULL;
    RET_CODE func_ret = RET_SUCCESS;
       UINT32 msg_code_max = 100;

     switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_CMD_EXIT:
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        #ifndef SD_UI
        win_compopup_set_frame(496, 330, 286, 130);
        #else
        win_compopup_set_frame(200, 330, 286, 130);
        #endif
        win_compopup_set_msg("Copy finished!", NULL,0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
        ret = PROC_LEAVE;
        break;

    case CTRL_MSG_SUBTYPE_STATUS_COPYFILE_ERR:
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        #ifndef SD_UI
        win_compopup_set_frame(496, 330, 286, 130);
        #else
        win_compopup_set_frame(200, 330, 286, 130);
        #endif
        if ((UINT32)(-ENOSPC) == msg_code)
             {
            win_compopup_set_msg("No space, copy failed!", NULL,0);
             }
        else
        {
            win_compopup_set_msg(NULL, NULL,RS_MSG_FILE_COPY_FAILED);
             }
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
        ret = PROC_LEAVE;
        break;

    case CTRL_MSG_SUBTYPE_STATUS_COPYFILE_CANT_COPY:
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        #ifndef SD_UI
        win_compopup_set_frame(496, 330, 286, 130);
        #else
        win_compopup_set_frame(200, 330, 286, 130);
        #endif
        win_compopup_set_msg("Can't copy!", NULL,0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(1000);
        win_compopup_smsg_restoreback();
        ret = PROC_LEAVE;
        break;

    case CTRL_MSG_SUBTYPE_STATUS_COPYFILE_OVERWRITE:
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg("Do you want to overwrite?",NULL,0);
        choice = win_compopup_open_ext(&back_saved);

        if(WIN_POP_CHOICE_YES == choice)
        {
            func_ret = copy_files_from_file_list(copyfile_cp_list, copyfile_cp_idx,\
                                                    copyfile_dest_path, 8,win_filecopy_callback);
            if(-1 == func_ret)
            {
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
                #ifndef SD_UI
                win_compopup_set_frame(496, 330, 286, 130);
                #else
                win_compopup_set_frame(200, 330, 286, 130);
                #endif
                win_compopup_set_msg(NULL, NULL,RS_MSG_FILE_COPY_FAILED);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(1000);
                win_compopup_smsg_restoreback();
                ret = PROC_LEAVE;
            }
        }
        else
        {
            ret = PROC_LEAVE;
        }
        break;

    case CTRL_MSG_SUBTYPE_STATUS_COPYFILE_NEWFILE:
             str_buff = (char *)malloc(sizeof(char)*FULL_PATH_SIZE);
             if(NULL == str_buff)
             {
                 libc_printf("malloc fail\n");
                 ASSERT(0);
                 return RET_FAILURE;
             }
             MEMSET(str_buff,0,FULL_PATH_SIZE);
        p_file = &copying_file;
        win_get_display_dirname(str_buff, FULL_PATH_SIZE, (char *)msg_code);
        osd_set_text_field_content(p_file,STRING_UTF8,(UINT32)str_buff);
        osd_draw_object((POBJECT_HEAD)p_file,C_UPDATE_ALL);

        p_txt = &copying_percent;
        osd_set_text_field_content(p_txt,STRING_ANSI,(UINT32)"");
        osd_draw_object((POBJECT_HEAD)p_txt,C_UPDATE_ALL);
        p_bar = &copying_bar;
        osd_set_progress_bar_pos(p_bar,0);
        osd_draw_object((POBJECT_HEAD)p_bar,C_UPDATE_ALL);
        break;

    case CTRL_MSG_SUBTYPE_STATUS_COPYFILE_PROGRESS:
        if(msg_code > msg_code_max)
             {
            msg_code = 0;
           }
        p_bar = &copying_bar;
        osd_set_progress_bar_pos(p_bar,msg_code);
        osd_draw_object((POBJECT_HEAD)p_bar,C_UPDATE_ALL);
        p_txt = &copying_percent;
        osd_set_text_field_content(p_txt, STRING_NUM_PERCENT,msg_code);
        osd_draw_object((POBJECT_HEAD)p_txt,C_UPDATE_ALL);
        break;

    default:
        break;
     }
       if(str_buff != NULL)
        {
            free(str_buff);
            str_buff = NULL;
        }
    return ret;
}

#endif

