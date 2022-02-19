/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_adc.c
*
*    Description:
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#include <api/libpub29/lib_as.h>
#else
#include <api/libpub/lib_frontend.h>
#endif
#include <api/libosd/osd_lib.h>
#include <api/libfs2/stdio.h>
#include <api/libc/string.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/fcntl.h>
#include <hld/nim/nim.h>
#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_signal.h"
#include "platform/board.h"
#include "win_adc.h"

#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

/*******************************************************************************
*   Objects definition
*******************************************************************************/
static VACTION adc_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT adc_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION adc_item_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT adc_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION adc_freq_edit_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT adc_freq_edit_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT adc_update_progress(UINT8 progress_prec);


#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX    WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX   WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_07_HD
#define CON_ENTER_IDX   WSTL_BUTTON_02_HD

#define TXT_SH_IDX   WSTL_BUTTON_01_HD
#define TXT_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD

#define SEL_SH_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD


#define PROGRESS_TXT_SH_IDX             WSTL_BUTTON_01_HD
#define PROGRESSBAR_SH_IDX              WSTL_BARBG_01_HD
#define PROGRESSBAR_MID_SH_IDX          WSTL_NOSHOW_IDX
#define PROGRESSBAR_PROGRESS_SH_IDX     WSTL_BAR_04_HD

#ifndef SUPPORT_CAS_A  /* define in win_com_menu_define.h already */
#define W_L         248
#define W_T         98
#define W_W         692
#define W_H         488
#endif

#define CON_L       (W_L+20)
#define CON_T       (W_T+4)
#define CON_W       (W_W - 60)
#define CON_H       40
#define CON_GAP     12

#define TXT_L_OF    10
#define TXT_W       250
#define TXT_H       CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF    (TXT_L_OF + TXT_W)
#define SEL_W       (CON_W-TXT_W-20)
#define SEL_H       CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4

#define BAR_TXT_W   80
#define BAR_TXT_H   46
#define BAR_L   (CON_L)
#define BAR_T   (CON_T + (CON_H + CON_GAP)*4)
#define BAR_W   (W_W - BAR_TXT_W - 80)
#define BAR_H   24

#define BAR_TXT_L (BAR_L + BAR_W+5 )
#define BAR_TXT_T (CON_T + (CON_H + CON_GAP)*4-10)


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    adc_item_keymap,adc_item_callback,  \
    conobj, ID,1)


#define LDEF_TXT_MSG(root,var_txt,nxt_obj,l,t,w,h)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,display_strs[7])


#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

#define LDEF_EDIT(root, var_num, nxt_obj, ID, l, t, w, h,style,cursormode,pat,suffix,str) \
    DEF_EDITFIELD(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    adc_freq_edit_keymap,adc_freq_edit_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,pat,10,cursormode,NULL,suffix,str)

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    adc_item_keymap,adc_item_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 30,0,style,ptabl,cur,cnt)

 #define LDEF_MENU_ITEM_EDT(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,\
                        l,t,w,h,res_id,style,cur_mode,pat,sub,str) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_EDIT(&var_con,var_num,NULL,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur_mode,pat,sub,str)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MENU_ITEM(root,var_con,nxt_obj,var_txt,var_num,var_line,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,&var_num,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id)    \
    LDEF_MSEL(&var_con,var_num,NULL/*&varLine*/   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_MENU_ITEM_TXT(root,var_con,nxt_obj,var_txt,var_line,ID,idu,idd,\
                        l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_TXT(&var_con,var_txt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,res_id) \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    adc_keymap,adc_callback,  \
    nxt_obj, focus_id,0)

#define LDEF_PROGRESS_BAR(root,var_bar,nxt_obj,l,t,w,h,style,rl,rt,rw,rh) \
    DEF_PROGRESSBAR(var_bar, root, nxt_obj, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX, PROGRESSBAR_SH_IDX,\
        NULL, NULL, style, 0, 0, PROGRESSBAR_MID_SH_IDX, PROGRESSBAR_PROGRESS_SH_IDX, \
        rl,rt , rw, rh, 1, 100, 100, 1)

#define LDEF_PROGRESS_TXT(root,var_txt,nxt_obj,l,t,w,h)       \
    DEF_TEXTFIELD(var_txt,root,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,PROGRESS_TXT_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT| C_ALIGN_VCENTER, 0,0,0,display_strs[7])

#define WIN  g_win_adc
static char fpat_char[]  = "f32";
static  UINT16 adc_edt_freq_suffix[] = {0x4d00, 0x4800, 0x7a00,  0x0};
static UINT16 adc_bandwidth_ids[] =
{
    RS_6MHZ,
    RS_7MHZ,
    RS_8MHZ,
};

LDEF_MENU_ITEM_EDT(WIN,adc_item1,&adc_item2,adc_txt1,adc_edt1, adc_line1, 1, 3, 2,
           CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_DISPLAY_FREQUENCY,
           NORMAL_EDIT_MODE, CURSOR_SPECIAL, fpat_char,adc_edt_freq_suffix, display_strs[20])

LDEF_MENU_ITEM(WIN,adc_item2,&adc_item3,adc_txt2,adc_sel1, adc_line2, 2, 1, 3, \
        CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_INSTALLATION_BANDWIDTH, STRING_ID, 0, 3, adc_bandwidth_ids)


LDEF_MENU_ITEM_TXT(WIN,adc_item3,NULL,adc_txt3, adc_line3, 3, 2, 1, \
        CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_COMMON_START)

LDEF_PROGRESS_BAR(&adc_item3,adc_bar_progress,&adc_txt_progress,    \
        BAR_L, BAR_T, BAR_W, BAR_H, PROGRESSBAR_HORI_NORMAL | PBAR_STYLE_RECT_STYLE,2,5,BAR_W - 4,BAR_H - 10)

LDEF_PROGRESS_TXT(&adc_item3, adc_txt_progress, &adc_txt4, BAR_TXT_L, BAR_TXT_T, BAR_TXT_W, BAR_TXT_H)

LDEF_TXT_MSG(NULL,adc_txt4,NULL,CON_L,(CON_T + (CON_H + CON_GAP)*4),CON_W,CON_H)

LDEF_WIN(WIN,&adc_item1,W_L, W_T, W_W, W_H,1)

#define FRE_ID      1
#define BWIDTH_ID      2
#define ADC_START_ID    3

#define BANDWIDTH_CNT            (adc_sel1.n_count)
#define BANDWIDTH_IDX              (adc_sel1.n_sel)

static UINT32 i_freq = 58600;

#define ADC_MSG_L       CON_L
#define ADC_MSG_T   (CON_T + (CON_H + CON_GAP)*5)
#define ADC_MSG_W   CON_W
#define ADC_MSG_H   CON_H
#define ADC_MSG_GAP 4
#define ADC_MSG_MAX_LINE  4

static UINT16 adc_msg_buf[ADC_MSG_MAX_LINE + 1][64];
static int adc_msg_line = 0;


static void win_adc_msg_update(UINT8 *msg, UINT32 msg_id)
{
    UINT32 i = 0;
    OSD_RECT rect;
    UINT8 *str = NULL;
    UINT16 tmp[256] = {0};

    com_asc_str2uni(msg, tmp);
    if(NULL == msg)
    {
        str = osd_get_unicode_string(msg_id);
    }
    else
    {
        str = (UINT8 *)tmp;
    }

    if(ADC_MSG_MAX_LINE == adc_msg_line)
    {
        for(i = 0; i < ADC_MSG_MAX_LINE - 1; i++)
        {
            com_uni_str_copy_char((UINT8 *)adc_msg_buf[i], (UINT8 *)adc_msg_buf[i + 1]);
        }
        com_uni_str_copy_char((UINT8 *)adc_msg_buf[i], str);
    }
    else
    {
        com_uni_str_copy_char((UINT8 *)adc_msg_buf[adc_msg_line++], str);
    }

    //draw msg infos
    for(i=0; i<ADC_MSG_MAX_LINE; i++)
    {
        rect.u_left  = ADC_MSG_L;
        rect.u_top   = ADC_MSG_T +(ADC_MSG_H + ADC_MSG_GAP)*i;
        rect.u_width = ADC_MSG_W;
        rect.u_height= ADC_MSG_H;

        osd_set_rect2(&adc_txt4.head.frame, &rect);
        osd_set_text_field_content(&adc_txt4, STRING_UNICODE, (UINT32)adc_msg_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&adc_txt4, C_UPDATE_ALL);
    }
}

static void win_adc_msg_clear(void)
{
    UINT8 i = 0;
    OSD_RECT rect;

    for(i=0;i<ADC_MSG_MAX_LINE;i++)
    {
        adc_msg_buf[i][0] = 0;

        rect.u_left  = ADC_MSG_L;
        rect.u_top   = ADC_MSG_T +(ADC_MSG_H + ADC_MSG_GAP)*i;
        rect.u_width = ADC_MSG_W;
        rect.u_height= ADC_MSG_H;

        osd_set_rect2(&adc_txt4.head.frame, &rect);
        osd_set_text_field_content(&adc_txt4, STRING_ANSI, (UINT32)adc_msg_buf[i]);
        osd_draw_object( (POBJECT_HEAD)&adc_txt4, C_UPDATE_ALL);
    }

    adc_msg_line = 0;
}

//joey, 20111107, for the filename include more information.
static UINT8 adc_dump_file_name[100];

//joey, 20111107, for the filename include more information.
static INT32 nim_adcdma_dump_rf_data(struct nim_device *dev, UINT32 freq, UINT32 bandwidth)
{
    UINT32 max_tmp_buf_cur = 0x2000;
    UINT8 *tmp_buf = NULL;
    UINT32 buf_cur = 0;
    UINT8 *tmp_txt_buf = NULL;

//for adc2dma function.
    UINT32 tmp_len = 0;

    UINT8 file_idx=0;

    FILE *data_file = NULL;

//for the filename include more information.
    snprintf((char *)adc_dump_file_name, 100, "/mnt/uda1/adc_dump_%d_%lu_%lum.dat", file_idx, freq, (bandwidth/1000));
    file_idx++;

    /* If device not running, exit */
    if (0 == (dev->flags & HLD_DEV_STATS_UP))
    {
        return ERR_DEV_ERROR;
    }
    tmp_buf=malloc(8*1024);
    if (NULL == tmp_buf)
    {
        return ERR_DEV_ERROR;
    }
    tmp_txt_buf=malloc(1024);
    if(NULL == tmp_txt_buf)
    {
        free(tmp_buf);
        return ERR_DEV_ERROR;
    }

    data_file = fopen((const char *)adc_dump_file_name, "wb+");
    if( NULL == data_file)
    {
        free(tmp_buf);
        free(tmp_txt_buf);
        //libc_printf("create data_file fail\n");
        return ERR_DEV_ERROR;
    }
    //libc_printf("%s : data_file %s\n", __FUNCTION__, adc_dump_file_name);

//for ADC2DMA length get, from low layer driver.
    nim_io_control(dev, NIM_DRIVER_ADC2MEM_SEEK_SET, (UINT32)(&tmp_len));
    //libc_printf("len 2: %d!\n", tmp_len);

    buf_cur=0;


    //libc_printf("%s : temp_buf: 0x%08x\n", __FUNCTION__, tmp_buf);
    //tmp_len = ((__MM_COFDM_S3811_ISDBT_BUF_LEN) >> 13) * 8192;

#if 0
    tmp_len = ((__MM_COFDM_S3811_ISDBT_BUF_LEN) >> 13) * 8192;
#else
    //tmp_len = ((70*1024*1024) >> 13) * 8192;

// for the safe usage of memory. move this length get to up-code.
    //tmp_len = ((0xA8000000 - __MM_MP_BUFFER_ADDR) >> 13) * 8192; // 8K uinit.

#endif

    while(buf_cur<tmp_len)
    {

        nim_io_control(dev, NIM_DRIVER_ADC2MEM_READ_8K, (UINT32)tmp_buf);
        buf_cur+=max_tmp_buf_cur;

        //libc_printf("\t write: 0x%08x to file.\n", buf_cur);

        #if 1
        fwrite(tmp_buf, max_tmp_buf_cur, 1, data_file);
        fflush(data_file);
        #else
        txt_buf_cur=0;
        tmp_buf_cur=0;
        while(tmp_buf_cur<max_tmp_buf_cur)
        {
            UINT8 j;
            MEMCPY(convert_buf, tmp_buf+tmp_buf_cur, 10);
            tmp_buf_cur+=10;

            j=0xff;
            for(i=0; i<80; i+=8)
            {
                if(j != (UINT8)(i/10))
                {
                    j=(i/10);
                    tmp[j]=convert_buf[i/8];
                }
            }
        }
        #endif
    }

    //libc_printf("%s : dump data to file finished.\n", __FUNCTION__);
    fclose(data_file);

    free(tmp_buf);
    free(tmp_txt_buf);
    fs_sync("/c");
    return 0;
}
/*******************************************************************************
*   Callback and Keymap Functions
*******************************************************************************/
static VACTION adc_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
        case V_KEY_EXIT:
        case V_KEY_MENU:
            act = VACT_CLOSE;
            break;
        default:
            act=VACT_PASS;
            break;
    }

    return act;

}


static PRESULT adc_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    switch (event)
    {
        case EVN_PRE_OPEN:
            wincom_i_to_mbs_with_dot(display_strs[20],i_freq, 5, 0x02);
            osd_set_container_focus((CONTAINER*)p_obj,1);
            break;
        case EVN_POST_OPEN:

            break;
        case EVN_PRE_CLOSE:
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;

            break;
        case EVN_POST_CLOSE:

            break;
        case EVN_UNKNOWNKEY_GOT:

            break;
        case EVN_UNKNOWN_ACTION:

            break;
        case EVN_MSG_GOT:


            break;
    default:
        break;
    }
    return ret;
}

static VACTION adc_freq_edit_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION action = VACT_PASS;

    switch (key)
    {
        case V_KEY_LEFT:
        action = VACT_EDIT_LEFT;
            break;
        case V_KEY_RIGHT:
        action = VACT_EDIT_RIGHT;
            break;
        case V_KEY_0:
        case V_KEY_1:
        case V_KEY_2:
        case V_KEY_3:
        case V_KEY_4:
        case V_KEY_5:
        case V_KEY_6:
        case V_KEY_7:
        case V_KEY_8:
        case V_KEY_9:
        action = key - V_KEY_0 + VACT_NUM_0;
            break;
        default:
            break;
    }

    return action;
}
static PRESULT adc_freq_edit_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}

static VACTION adc_item_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_DECREASE;
        break;
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}
static PRESULT adc_item_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = VACT_PASS;
    UINT8 b_id = 0;
    UINT8 prec1=10;
    UINT8 prec2=40;
    UINT8 prec3=80;
    UINT8 prec4=100;
    UINT32 bandwidth=0;
    struct nim_device *nim_dev = NULL;
    union ft_xpond xponder;
    struct ft_frontend ft;
    UINT8 dot_pos = 0;
    UINT32 freq = 0;
    char msg[64] = {0};
    struct dmx_device *dmx_dev0 = NULL;
    struct dmx_device *dmx_dev1 = NULL;
    struct vpo_device *vpo_dev = NULL;

    b_id = osd_get_obj_id(p_obj);
    MEMSET(&xponder, 0, sizeof(union ft_xpond));
    MEMSET(&ft, 0, sizeof(struct ft_frontend));

    if (0 == BANDWIDTH_IDX)
    {
        bandwidth=6000;
    }
    else if (1 == BANDWIDTH_IDX)
    {
        bandwidth=7000;
    }
    else if ((1+1) == BANDWIDTH_IDX)
    {
        bandwidth=8000;
    }

    switch (event)
    {

    case EVN_UNKNOWN_ACTION:
    case EVN_KEY_GOT:
        unact = (VACTION)(param1>>16);

        if((VACT_ENTER == unact) && (ADC_START_ID == b_id))
        {
            wincom_mbs_to_i_with_dot(display_strs[20], &freq, &dot_pos);
            freq *= 10;
            win_adc_msg_clear();

            snprintf(msg, 64, "Set the freq to %lu", freq);
            win_adc_msg_update((UINT8 *)msg, 0);
            ret=adc_update_progress(prec1);

            if((FRONTEND_TYPE_T == board_get_frontend_type(0)) || (FRONTEND_TYPE_ISDBT == board_get_frontend_type(0)))
            {
                nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
            }
            else
            {
                nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
            }
#ifdef DVBT_SUPPORT
            if(FRONTEND_TYPE_T == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM) && !board_frontend_is_isdbt(0))
            {

                xponder.t_info.type = FRONTEND_TYPE_T;
                xponder.t_info.frq = freq;
                xponder.t_info.band_width = bandwidth;
                xponder.t_info.usage_type = 0x02;
                frontend_tuning(nim_dev, NULL, &xponder, 1);
            }

#endif
#ifdef ISDBT_SUPPORT


            if(FRONTEND_TYPE_ISDBT == dev_get_device_sub_type(nim_dev, HLD_DEV_TYPE_NIM) && board_frontend_is_isdbt(0))
            {

                ft.nim = nim_dev;
                ft.xpond.t_info.type = FRONTEND_TYPE_ISDBT;
                ft.xpond.t_info.frq = freq;
                ft.xpond.t_info.band_width = bandwidth; // KHz
                ft.xpond.t_info.usage_type = 0x02;
                uich_chg_aerial_signal_monitor(&ft);
            }

#endif
            osal_task_sleep(100);
            win_adc_msg_update((UINT8 *)"Start ADC2DMA...", 0);
            ret=adc_update_progress(prec2);

// for the safe usage of memory.
            // vdec+audio already stopped. close dmx/vpo.
            dmx_dev0 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
            dmx_stop(dmx_dev0);

            dmx_dev1 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
            dmx_stop(dmx_dev1);

            vpo_dev = (struct vpo_device*)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
            vpo_win_onoff(vpo_dev, FALSE);  // close logo
         
            nim_adcdma_start(nim_dev, 0);

            osal_task_sleep(100);
            win_adc_msg_update((UINT8 *)"Dump the data for file...", 0);
            ret=adc_update_progress(prec3);

// for the filename include more information.
            nim_adcdma_dump_rf_data(nim_dev, freq, bandwidth);

            //after dump data finished, we can let COFDM normal work.
            nim_adcdma_stop(nim_dev);

            win_adc_msg_update((UINT8 *)"ADC2DMA done!", 0);
            osal_task_sleep(1000);
            ret=adc_update_progress(prec4);

//after use other IP's memory, re-enable the dmx, but no vpo(if so, color background).
            //dmx_start(dmx_dev0);
            //dmx_start(dmx_dev1);
            //vpo_win_onoff(vpo_dev, TRUE);  // dispaly logo.

        }

        break;
    case EVN_MSG_GOT:

        break;
    default:
        break;
    }
    return ret;
}


static PRESULT adc_update_progress(UINT8 progress_prec)
{
    UINT8 result = PROC_LOOP;

    PROGRESS_BAR *bar = NULL;
    TEXT_FIELD *txt = NULL;

    UINT8 progress_prec_check_1 = 150;
    UINT8 progress_prec_check_2 = 3;

    bar = &adc_bar_progress;
    txt = &adc_txt_progress;
    if(progress_prec == progress_prec_check_1)
    {
        progress_prec = 100;
    }

    osd_set_progress_bar_pos(bar, progress_prec);
    if(progress_prec < progress_prec_check_2)
    {
        bar->w_tick_fg = WSTL_NOSHOW_IDX;
    }
    else
    {
        bar->w_tick_fg = PROGRESSBAR_PROGRESS_SH_IDX;
    }
    osd_set_text_field_content(txt, STRING_NUM_PERCENT,progress_prec);
    {
        osd_draw_object( (POBJECT_HEAD)bar, C_UPDATE_ALL);
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);
    }

    return result;
}


