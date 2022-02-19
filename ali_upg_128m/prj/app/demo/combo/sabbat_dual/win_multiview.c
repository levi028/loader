/****************************************************************************
*
*  ALi (Shanghai) Corporation, All Rights Reserved. 2003 Copyright (C)
*
*  File: win_multiview.c
*
*  Description: m/p
*
*  History:
*      Date                Author             Version       Comment
*      ====                ======          =======       =======
*  1.  2006.05.30      Sunny Yin         0.1.000       Initial
*
****************************************************************************/
#include <types.h>
#include <osal/osal.h>

#include <hld/dis/vpo.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libosd/osd_lib.h>
//#include <api/libpub29/lib_pub27.h>
#include "osdobjs_def.h"
//#include <api/libtsi/si_monitor.h>
#include "osd_config.h"
#include "string.id"
#include "images.id"
#include "key.h"

#include "win_com.h"
#include "win_multiview.h"
#include "control.h"
#include "win_multiview_password.h"
#include "win_signalstatus.h"
#include "menus_root.h"
#include "ctrl_util.h"
#include "win_prog_name.h"
#include "win_tvsys.h"

extern void ap_pid_change(BOOL need_chgch);
/*******************************************************************************
* WINDOW's objects declaration
*******************************************************************************/
MATRIX_BOX_NODE mb_node_mv_name[MULTI_VIEW_COUNT];

CONTAINER    g_cnt_multiview;
MATRIX_BOX    g_mtb_mv_name;
BITMAP        g_bmp_mv_load;
BITMAP        g_bmp_mv_signal;
TEXT_FIELD g_bmp_mv_signal_txt;

static BOOL g_key_exit = FALSE;
static BOOL g_key_ok = FALSE;
UINT16*    str_mv_prog_name;
static UINT32 system_tv_mode=0;
static BOOL g_age_lock = FALSE;
static UINT32 lock_play = 0;
static BOOL pwd_open_state = FALSE;

extern void win_multiview_ext_set_lock(BOOL flag);
static VACTION win_mv_mtb_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT win_mv_ctn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT win_mv_mtb_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION win_com_con_keymap(POBJECT_HEAD obj, UINT32 key);


#define WSTL_TRANS_IDX            WSTL_TRANS_IX
#define MBX_SH_IDX                WSTL_BUTTON_POP_SH_HD//WSTL_TEXT_04
#define MBX_HL_IDX                WSTL_BUTTON_POP_HL_HD//WSTL_TEXT_08//??
#define IM_LOADING_02            IM_LOADING
#define IM_GROUP_LOCK            IM_INFORMATION_ICON_6_09//IM_LOCK//IM_NINE_LOCK
#define IM_SIGNAL_LEVEL_01        IM_EPG_COLORBUTTON_BLUE//IM_ORANGE_ARROW_S//IM_NINE_NOSIGNAL
#define IM_GROUP_MONEY            IM_INFORMATION_ICON_MONEY//IM_NINE_MONEY

CONTAINER    g_cnt_multiview = {
                                    {    OT_CONTAINER, //object type
                                        C_ATTR_ACTIVE,   // defined in form C_ATTR_XXX
                                        C_FONT_DEFAULT,    // defined in form C_FONT_XXX
                                            0,      // 0: for component without focus

                                        0,//UINT8 bLeftID;
                                        0,//UINT8 bRightID;
                                        0,//UINT8 bUpID;
                                        0,//UINT8 bDownID;

                                        {CNT_MV_L, CNT_MV_T, CNT_MV_W,CNT_MV_H},//OSD_RECT frame;
                                        {0,0,0,0},//COLOR_STYLE style;

                                        win_com_con_keymap,//PFN_KEY_MAP pfnKeyMap;
                                        (PFN_CALLBACK)win_mv_ctn_callback,//PFN_CALLBACK pfnCallback;

                                        NULL,//POBJECT_HEAD pNext;
                                        NULL//POBJECT_HEAD pRoot;
                                    },
                                    (POBJECT_HEAD)&g_mtb_mv_name,
                                    1,
                                    0
};


MATRIX_BOX    g_mtb_mv_name = {
                                    {    OT_MATRIXBOX, //object type
                                        C_ATTR_ACTIVE,   // defined in form C_ATTR_XXX
                                        C_FONT_DEFAULT,    // defined in form C_FONT_XXX
                                            1,      // 0: for component without focus

                                        1,//UINT8 bLeftID;
                                        1,//UINT8 bRightID;
                                        1,//UINT8 bUpID;
                                        1,//UINT8 bDownID;

                                        {MTB_MV_L,MTB_MV_T,MTB_MV_W,MTB_MV_H},//COLOR_STYLE style;
//                                {WSTL_BUTTON_POP_SH_HD,WSTL_BUTTON_01_HD,0,0},//COLOR_STYLE style;
                                        {MBX_SH_IDX,MBX_HL_IDX,0,0},//COLOR_STYLE style;

                                        win_mv_mtb_keymap,//PFN_KEY_MAP pfnKeyMap;
                                        win_mv_mtb_callback,//PFN_CALLBACK pfnCallback;

                                        (POBJECT_HEAD)&g_bmp_mv_signal_txt,//POBJECT_HEAD pNext;
                                        (POBJECT_HEAD)&g_cnt_multiview//POBJECT_HEAD pRoot;
                                        },
                                        MATRIX_ROW_WRAP,//UINT8 bStyle;
                                        MATRIX_TYPE_WCHAR,//UINT8 bCellType;
                                        MULTI_VIEW_COUNT,//INT16 nCount;
                                        mb_node_mv_name,//    PMATRIX_BOX_NODE pCellTable;

                                        MULTI_VIEW_X_COUNT,//INT16 nRow;
                                        (MULTI_VIEW_COUNT/MULTI_VIEW_X_COUNT),//INT16 nCol;
                                        C_ALIGN_CENTER | C_ALIGN_VCENTER,//    UINT8 bAlign;
                                        6,//INT8 bIntervalX;
                                        (TXT_MV_Y_SHIFT-MTB_MV_H),//INT8 bIntervalY;
                                        0,//INT8 bIntervalT;
                                        0,//INT8 bIntervalZ;
                                        0,//INT8 bIntervalT;
                                        0,//INT8 bIntervalZ;
                                        0//INT16 nPos;            // the item index of current position
};

BITMAP    g_bmp_mv_load = {
                                    {    OT_BITMAP, //object type
                                        C_ATTR_ACTIVE,   // defined in form C_ATTR_XXX
                                        C_FONT_DEFAULT,    // defined in form C_FONT_XXX
                                            0,      // 0: for component without focus

                                        0,//UINT8 bLeftID;
                                        0,//UINT8 bRightID;
                                        0,//UINT8 bUpID;
                                        0,//UINT8 bDownID;

                                        {BMP_LOAD_L, BMP_LOAD_T,BMP_LOAD_W,BMP_LOAD_H},//OSD_RECT frame;
                                        {WSTL_TRANS_IDX,0,0,0},//COLOR_STYLE style;

                                        NULL,//PFN_KEY_MAP pfnKeyMap;
                                        NULL,//PFN_CALLBACK pfnCallback;

                                        NULL,//POBJECT_HEAD pNext;
                                        NULL//POBJECT_HEAD pRoot;
                                        },
                                         C_ALIGN_LEFT | C_ALIGN_VCENTER,//UINT8 bAlign;
                                         0,//UINT8 bX;
                                         0, //UINT8 bY;
                                         IM_LOADING_02, //UINT16 wIconID;
};

BITMAP    g_bmp_mv_signal = {
                                    {    OT_BITMAP, //object type
                                        C_ATTR_ACTIVE,   // defined in form C_ATTR_XXX
                                        C_FONT_DEFAULT,    // defined in form C_FONT_XXX
                                            0,      // 0: for component without focus

                                        0,//UINT8 bLeftID;
                                        0,//UINT8 bRightID;
                                        0,//UINT8 bUpID;
                                        0,//UINT8 bDownID;

                                        {BMP_SIG_L, BMP_SIG_T,BMP_SIG_W,BMP_SIG_H},//OSD_RECT frame;
                                        {WSTL_TRANS_IDX,0,0,0},//COLOR_STYLE style;

                                        NULL,//PFN_KEY_MAP pfnKeyMap;
                                        NULL,//PFN_CALLBACK pfnCallback;

                                        NULL,//POBJECT_HEAD pNext;
                                        NULL//POBJECT_HEAD pRoot;
                                        },
                                         C_ALIGN_LEFT | C_ALIGN_VCENTER,//UINT8 bAlign;
                                         0,//UINT8 bX;
                                         0, //UINT8 bY;
                                         IM_LOADING_02, //UINT16 wIconID;
};

#define LDEF_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)    \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
        0,0,0,0,0, l,t,w,h, WSTL_TEXT_10_HD, WSTL_TEXT_10_HD, WSTL_TEXT_10_HD,WSTL_TEXT_10_HD,   \
        NULL, NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,str)

LDEF_TXT(&g_cnt_multiview,g_bmp_mv_signal_txt,NULL, TXT_L, TXT_T,TXT_W, TXT_H, 0, 0);

static UINT16   g_mv_top, g_mv_total, g_mv_page_cnt;


static volatile UINT32 g_load_posi;
static BOOL b_lock;
static volatile BOOL check_prog_finised = 0;
static UINT32 mv_state = BOX_SCAN_START;
static struct multi_view_para    mv_para;

BOOL win_multiview_get_lock(void)
{
    return b_lock;
}
void win_multiview_set_lock(BOOL flag)
{
    b_lock=flag;
}
void win_multiview_set_age_lock(BOOL age_lock)
{
    g_age_lock = age_lock;
}
BOOL win_multiview_get_age_lock(void)
{
    return g_age_lock;
}
BOOL win_multiview_get_state(void)
{
    return ((mv_state == BOX_SCAN_START)||!check_prog_finised ||((uimultiview_get_status()&MV_SCAN_SCREEN)==MV_SCAN_SCREEN));
}

static PRESULT win_mv_message_proc(UINT32 msg_type, UINT32 msg_code);

/*******************************************************************************
* Function definition
*******************************************************************************/
#ifdef NEW_DEMO_FRAME
/* Get ft and dev list before enter mv */
static void win_mv_pre_enter(P_NODE *p_node, struct cc_es_info *es, struct ft_frontend *ft, struct cc_device_list *dev_list)
{
    struct cc_param param;
    BOOL ci;

    MEMSET(&param, 0, sizeof(struct cc_param));
    api_pre_play_channel(p_node, ft, &param, &ci, FALSE);
    MEMCPY(es, &(param.es), sizeof(struct cc_es_info));
    es->sim_onoff_enable = 0;
    MEMCPY(dev_list, &(param.dev_list), sizeof(struct cc_device_list));
}
#endif
static void mv_dpid_callback(UINT16 u_index)
{
    P_NODE p_node;
#ifdef NEW_DEMO_FRAME
    struct ft_frontend frontend;
    struct cc_es_info es;
    struct cc_device_list dev_list;
#endif
    get_prog_at(mv_para.u_prog_info[u_index], &p_node);
    win_mv_pre_enter(&p_node, &es, &frontend, &dev_list);
    MEMCPY(&(mv_para.mv_param[u_index].es), &es, sizeof(struct cc_es_info));
    MEMCPY(&(mv_para.mv_param[u_index].device_list), &dev_list, sizeof(struct cc_device_list));
    MEMCPY(&(mv_para.mv_param[u_index].ft), &frontend, sizeof(struct ft_frontend));
    mv_update_play_para(&mv_para);
}
static UINT32 win_mv_cb(UINT16 u_index)
{
    BOOL b_send_cmd = FALSE;
    UINT32 timeout = 0;

    check_prog_finised = 0;
    g_load_posi = u_index;
    P_NODE p_node;

    MEMSET(&p_node, 0, sizeof(P_NODE));
    get_prog_at(g_mv_top+(UINT8)u_index, &p_node);

    do
    {
         b_send_cmd = ap_send_msg_ext(CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, u_index, FALSE);
         if((!b_send_cmd) && (!get_mv_exit()))
         {
            osal_task_sleep(10);
         }
    }while(!b_send_cmd && !get_mv_exit());

    while(!check_prog_finised && !get_mv_exit()&&!(timeout>10)/*&& !get_mv_cancel()*/)
    {
        //soc_printf("wait\n");
        osal_task_sleep(10);
        if(!(pwd_open_state&&(sys_data_get_channel_lock() && (p_node.lock_flag || p_node.provider_lock))))
        {
        	timeout++;
        }
    }

    if(b_lock/*|| get_mv_cancel()*/)
    {
        return BOX_PLAY_ABORT;
    }
    else
    {
        return BOX_PLAY_NORMAL;
    }
}

static void win_mv_load_page(UINT16 first_idx, UINT32 page_cnt)
{
    UINT32                i,j;
    UINT16            width, height;
    struct mvinit_para     t_mv_init_para;
    UINT16            tmp_name[MAX_DISP_STR_LEN];
    UINT8                tmp_idx[10];
    //struct MultiViewPara    mv_para;
    P_NODE p_node;

#ifdef NEW_DEMO_FRAME
    struct ft_frontend frontend;
    struct cc_es_info es;
    struct cc_device_list dev_list;
#endif
    //height = (TV_MODE_PAL == tv_mode) ? MV_VIDEO_HEIGHT_P : MV_VIDEO_HEIGHT_N;
    UINT8 tv_mode;
    UINT32 pipmode=0;
    tv_mode = api_video_get_tvout();

    if(TV_MODE_720P_50 == tv_mode || TV_MODE_1080I_25 == tv_mode
        || TV_MODE_576P == tv_mode || TV_MODE_PAL == tv_mode
        || TV_MODE_1080P_25 == tv_mode || TV_MODE_1080P_50 == tv_mode
        || TV_MODE_1080P_24 == tv_mode
        || TV_MODE_PAL_N == tv_mode)
    {
        pipmode = 576*5;
    }
    else if(TV_MODE_720P_60 == tv_mode || TV_MODE_480P == tv_mode
        || TV_MODE_1080I_30 == tv_mode || TV_MODE_NTSC358 == tv_mode
        || TV_MODE_1080P_30 == tv_mode || TV_MODE_1080P_60 == tv_mode
        || TV_MODE_NTSC443 == tv_mode|| TV_MODE_PAL_M == tv_mode )
    {
        pipmode = 480*6;
    }



    mv_para.u_box_num = page_cnt;
    mv_para.u_box_active_idx = osd_get_mtrxbox_cur_pos(&g_mtb_mv_name) + 1;
    mv_para.mvmode = MV_9_BOX;
    mv_para.multi_view_buf_addr= __MM_PVR_VOB_BUFFER_ADDR;
    mv_para.multi_view_buf_size = __MM_PVR_VOB_BUFFER_LEN;

    width = TXT_MV_X_SHIFT;//MV_VIDEO_WIDTH ;//+ MV_VIDEO_WIDTH % 16;
    height = TXT_MV_Y_SHIFT;//MV_VIDEO_HEIGHT;// + MTB_MV_H * OSD_MULTI_HEIGHT/576;
    //height = height - height % 2; CNT_MV_L+ TXT_MV_X_SHIFT * i

    for(i = 0;  i<page_cnt; i++)
    {
        t_mv_init_para.pu_box_posi[i].u_x = MV_VIDEO_LEFT*3/2 + (i%MULTI_VIEW_X_COUNT) * width*3/2;
        t_mv_init_para.pu_box_posi[i].u_y = MV_VIDEO_TOP*3/2 + (i/MULTI_VIEW_X_COUNT) * height*3/2;
        //
        t_mv_init_para.t_mpip_box_posi[i].u_x = MV_VIDEO_LEFT*9/16 + (i%MULTI_VIEW_X_COUNT) * width*9/16;
        t_mv_init_para.t_mpip_box_posi[i].u_y = MV_VIDEO_TOP*pipmode/720 + (i/MULTI_VIEW_X_COUNT) * height*pipmode/720;

#ifdef BIDIRECTIONAL_OSD_STYLE
        if (osd_get_mirror_flag() == TRUE)
        {
            t_mv_init_para.pu_box_posi[i].u_x = 1280*3/2 - t_mv_init_para.pu_box_posi[i].u_x - width*3/2;
            t_mv_init_para.t_mpip_box_posi[i].u_x = 1280*9/16 - t_mv_init_para.t_mpip_box_posi[i].u_x - width*9/16;

        }
#endif
        snprintf((char *)tmp_idx, 10,"%d-",(int)(first_idx+i+1));
        com_asc_str2uni(tmp_idx, (UINT16*)mb_node_mv_name[i].str);
        //api_get_channel_name(first_idx+i, tmp_name);
        get_prog_at(first_idx+i,&p_node);
        if(p_node.ca_mode)
        {
            set_uni_str_char_at(tmp_name,'$',0);
            j = 1;
        }
        else
            j = 0;
        com_uni_str_copy_char((UINT8*)&tmp_name[j],p_node.service_name);

        mv_para.u_prog_info[i] = first_idx+i;
        win_mv_pre_enter(&p_node, &es, &frontend, &dev_list);
        MEMCPY(&(mv_para.mv_param[i].es), &es, sizeof(struct cc_es_info));
        MEMCPY(&(mv_para.mv_param[i].device_list), &dev_list, sizeof(struct cc_device_list));
        MEMCPY(&(mv_para.mv_param[i].ft), &frontend, sizeof(struct ft_frontend));
        com_uni_str_cat((UINT16*)mb_node_mv_name[i].str, (const UINT16*)tmp_name);
    }

    //if(TV_MODE_PAL == tv_mode)
    //{
    t_mv_init_para.t_mp_size.u_width=MV_MP_WIDTH*3/2;
    t_mv_init_para.t_mp_size.u_height=MV_MP_HEIGHT*3/2;

    t_mv_init_para.t_pip_size.u_width=MV_VIDEO_WIDTH*3/2;//MV_VIDEO_WIDTH*3/2;
    t_mv_init_para.t_pip_size.u_height=MV_VIDEO_HEIGHT*3/2;//MV_VIDEO_HEIGHT*3/2;

    t_mv_init_para.t_mpip_size.u_width=MV_VIDEO_WIDTH*9/16+1;
    t_mv_init_para.t_mpip_size.u_height=MV_VIDEO_HEIGHT*pipmode/720+20;
    //}
    //else
    //{
    //    tMvInitPara.tMpSize.uHeight=480;
    //    tMvInitPara.tPipSize.uHeight=height-24;
    //}

    t_mv_init_para.callback=    win_mv_cb;
    t_mv_init_para.avcallback= (simcallback)mv_dpid_callback;
    t_mv_init_para.u_box_num= page_cnt;

    uimultiview_init(&t_mv_init_para,MV_9_BOX);
    uimultiview_enter(&mv_para);
}

/*********************************************************************************
*
*    Window handle's open,proc and handle
*
**********************************************************************************/
static void win_mv_scale_screen()
{
    struct osdrect     multi_full_rect;
    //enum    osdsys    e_osdsys;
    struct osdrect    r;
    UINT8    i;
    //UINT8 tv_mode;
    //enum tvsystem tvsys;
    //struct osdpara    t_open_para;
    //struct osdrect t_open_rect;

    osd_get_rect_on_screen(&r);
    r.u_left= r.u_top= 0;
    osd_draw_frame(&r, OSD_TRANSPARENT_COLOR, NULL);

    multi_full_rect.u_top =    0;//OSD_MULTI_TOP;
    multi_full_rect.u_left = 0;//OSD_MULTI_LEFT;
    multi_full_rect.u_width = 1280;//OSD_MULTI_WIDTH;
    multi_full_rect.u_height = 720;//OSD_MULTI_HEIGHT;
    osd_set_rect_on_screen(&multi_full_rect);

    //reset OSD rect and scale
//    tv_mode = api_video_get_tvout();//sys_data_get_tv_mode();
//    if(TV_MODE_PAL == tv_mode)
//        eOSDSys = OSD_PAL;
//    else
//        eOSDSys = OSD_NTSC;
//    OSD_Scale(OSD_VSCALE_DVIEW, (UINT32)&eOSDSys);


    osd_draw_fill(CNT_MV_L, CNT_MV_T, TXT_MV_X_SHIFT*MULTI_VIEW_X_COUNT+6, 4, 0xffff, NULL);
    osd_draw_fill(CNT_MV_L, CNT_MV_T+4, 6,TXT_MV_Y_SHIFT*MULTI_VIEW_X_COUNT+4, 0xffff, NULL);
    for(i = 1 ; i <= MULTI_VIEW_X_COUNT; i++)
    {
        osd_draw_fill(CNT_MV_L,CNT_MV_T+4+TXT_MV_Y_SHIFT * i,TXT_MV_X_SHIFT*MULTI_VIEW_X_COUNT, 4, 0xffff, NULL);
        osd_draw_fill(CNT_MV_L+ TXT_MV_X_SHIFT * i , CNT_MV_T+4, 6, TXT_MV_Y_SHIFT*MULTI_VIEW_X_COUNT+4, 0xffff, NULL);
    }
}

static BOOL enter_mv_mode = FALSE;
static BOOL win_mv_page_shift(UINT16 start, UINT16 count, INT16 shift)
{
    INT16    first_channel = start;
    //struct osdrect    r;
    if((count < shift) ||(count < (0-shift)))
        return FALSE;
    else
    {
        first_channel += shift;

        if(first_channel < 0)
        {
            if(0 == start)
                first_channel= count - g_mv_page_cnt;
            else
                first_channel= 0;
        }
        else if(first_channel >= count)
        {
            if((count - g_mv_page_cnt) == start)
                first_channel = 0;
            else
                first_channel= count - g_mv_page_cnt;
        }
        else if((first_channel + g_mv_page_cnt >=  count) && (count > g_mv_page_cnt))
        {//when channels after first one is no more than 9
            if(enter_mv_mode)
            {
                enter_mv_mode = FALSE;
                osd_set_mtrxbox_cur_pos(&g_mtb_mv_name, (first_channel + g_mv_page_cnt - count));
            }
            first_channel = count - g_mv_page_cnt;
        }
    }
    g_mv_top = first_channel;

    win_mv_load_page(first_channel, g_mv_page_cnt);
    return TRUE;
}


static void win_mv_init()
{
    system_tv_mode=sys_data_get_aspect_mode();
    if(TV_ASPECT_RATIO_169==system_tv_mode)
    {
        //MV_DEBUG("%s-->TvMode=%d\n",__FUNCTION__,SystemTvMode);
        vpo_aspect_mode( (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), TV_4_3, NORMAL_SCALE);
    }
    UINT16    i;
    UINT16    cur_channel;

    if(get_pause_state())
        set_pause_on_off(FALSE);

    //alloc string
    for(i = 0; i < MULTI_VIEW_COUNT; i++)
    {
        mb_node_mv_name[i].str = (UINT32)display_strs[i];//(UINT32)api_alloc_static_str(API_STATIC_STR_LEN);
        mb_node_mv_name[i].num = 0;
        mb_node_mv_name[i].b_attr = C_ATTR_ACTIVE;
    }

    //init parametre
    g_mv_total =get_prog_num(VIEW_ALL | TV_CHAN, 0);;//get_node_num(TYPE_PROG_NODE, NULL);
    cur_channel = sys_data_get_cur_group_cur_mode_channel();

    if(g_mv_total <= MULTI_VIEW_COUNT)
    {
        g_mv_top = 0;
        osd_set_mtrxbox_count(&g_mtb_mv_name, g_mv_total, MULTI_VIEW_X_COUNT, MULTI_VIEW_X_COUNT);
        osd_set_mtrxbox_cur_pos(&g_mtb_mv_name, cur_channel);
        g_mv_page_cnt = g_mv_total;
    }
    else
    {
        g_mv_top = cur_channel;
        g_mv_page_cnt = MULTI_VIEW_COUNT;
        osd_set_mtrxbox_count(&g_mtb_mv_name, g_mv_page_cnt, MULTI_VIEW_X_COUNT, MULTI_VIEW_X_COUNT);
        osd_set_mtrxbox_cur_pos(&g_mtb_mv_name, 0);
    }
}


static void win_mv_update_signal(BOOL onoff,UINT16 idx)
{
    short    x, y;
    P_NODE    p_node;
    signal_lock_status lock_flag;
    signal_scramble_status scramble_flag;
    signal_lnbshort_status lnbshort_flag;
    signal_parentlock_status parrentlock_flag;
    //struct ft_frontend frontend;
    //struct cc_param chchg_param;
    //BOOL ci_start_service = FALSE;
    //BOOL b_force=0;

    //GetSignalStatus(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag);
#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag;
    get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag);
#else
    get_signal_status(&lock_flag,&scramble_flag,&lnbshort_flag,&parrentlock_flag);
#endif
    get_prog_at(g_mv_top+idx, &p_node);

    if((b_lock)&& onoff)
    {
        osd_set_bitmap_content(&g_bmp_mv_signal, IM_GROUP_LOCK);
        osd_set_text_field_content(&g_bmp_mv_signal_txt, STRING_ID,RS_SYSTME_PARENTAL_LOCK);
    }
    else if((SIGNAL_STATUS_UNLOCK == lock_flag)&& onoff)
    {
        osd_set_bitmap_content(&g_bmp_mv_signal, IM_SIGNAL_LEVEL_01);
        osd_set_text_field_content(&g_bmp_mv_signal_txt, STRING_ID, RS_MSG_NO_SIGNAL);
    }
    else if((((p_node.ca_mode == 1)&&(MV_SCAN_SCREEN & uimultiview_get_status()))||(SIGNAL_STATUS_SCRAMBLED == scramble_flag))&& onoff)
    {
        osd_set_bitmap_content(&g_bmp_mv_signal, IM_GROUP_MONEY);
        osd_set_text_field_content(&g_bmp_mv_signal_txt, STRING_ID, RS_SYSTME_CHANNEL_SCRAMBLED);
    }
    else
    {
        osd_set_bitmap_content(&g_bmp_mv_signal, 0);
        osd_set_text_field_content(&g_bmp_mv_signal_txt, STRING_ID, 0);
    }

    x = BMP_SIG_L + ((idx)%MULTI_VIEW_X_COUNT * TXT_MV_X_SHIFT);
    y = BMP_SIG_T + ((idx)/MULTI_VIEW_X_COUNT* TXT_MV_Y_SHIFT) ;
    osd_move_object((POBJECT_HEAD)&g_bmp_mv_signal, x, y, FALSE);
    osd_move_object((POBJECT_HEAD)&g_bmp_mv_signal_txt, x-(TXT_W-BMP_SIG_W)/2, y+BMP_SIG_H, FALSE);

    osd_draw_object((POBJECT_HEAD)&g_bmp_mv_signal, C_UPDATE_ALL);
    osd_draw_object((POBJECT_HEAD)&g_bmp_mv_signal_txt, C_UPDATE_ALL);

}

static void win_mv_load_sign_onoff(BOOL onoff, UINT16 idx)
{
    short    x, y;

    x = BMP_LOAD_L + ((idx)%MULTI_VIEW_X_COUNT * TXT_MV_X_SHIFT);
    y = BMP_LOAD_T + ((idx)/MULTI_VIEW_X_COUNT* TXT_MV_Y_SHIFT) ;
    osd_move_object((POBJECT_HEAD)&g_bmp_mv_load, x, y, FALSE);

    if(onoff)
        osd_draw_object((POBJECT_HEAD)&g_bmp_mv_load, C_UPDATE_ALL);
    else
    {
        osd_hide_object((POBJECT_HEAD)&g_bmp_mv_load, 0);
        win_mv_update_signal(FALSE,idx);
    }

}

static VACTION win_mv_mtb_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION    act = VACT_PASS;

    switch(key)
    {
        case V_KEY_P_UP:
            act = VACT_CURSOR_PGUP;
            break;
        case V_KEY_P_DOWN:
            act = VACT_CURSOR_PGDN;
            break;
        //case V_KEY_MUTE:
        case V_KEY_V_UP:
        case V_KEY_V_DOWN:
            act = VACT_NO_RESPOND;
            break;
        default:
            act = osd_matrixbox_key_map(obj, key);
            break;
    }

    return act;
}

static UINT32 chg_time_out = 0;

static PRESULT win_mv_mtb_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    //UINT32 i;
    P_NODE playing_pnode;
    static UINT32    exit_key[] =
            {
                V_KEY_UP, V_KEY_DOWN,
                V_KEY_C_UP, V_KEY_C_DOWN,
                //V_KEY_P_UP, V_KEY_P_DOWN,
                V_KEY_LEFT,V_KEY_RIGHT,
                V_KEY_MENU,V_KEY_EXIT
            };

    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
        {
            VACTION vact = (VACTION)(param1>>16);
            if((VACT_CURSOR_PGUP == vact) || (VACT_CURSOR_PGDN == vact))
            {
                INT16 shift =(VACT_CURSOR_PGUP == vact) ? (0-MULTI_VIEW_COUNT) : MULTI_VIEW_COUNT ;
                win_mv_page_shift(g_mv_top,g_mv_total, shift );
                osd_track_object(p_obj, C_UPDATE_ALL);
                sys_data_set_cur_group_channel(g_mv_top+osd_get_mtrxbox_cur_pos(&g_mtb_mv_name));
            }
            else if(VACT_ENTER == vact)
            {
                return PROC_LEAVE;
            }
            else if(VACT_NO_RESPOND == (UINT8)vact)
                return PROC_LOOP;
            break;
        }
        case EVN_ITEM_PRE_CHANGE:
        {
            UINT8        multi_status = uimultiview_get_status();
            if((MV_SCAN_SCREEN & multi_status))// || !check_prog_finised || MV_IDLE &multi_status)
                return PROC_LOOP;
            set_box_play_status(FALSE);
            chg_time_out = 0;
            win_multiview_set_age_lock(FALSE);
            lock_play = 0;
            break;
        }
        case EVN_ITEM_POST_CHANGE:
        {
            UINT16    cur_channel = g_mv_top+param1;
            UINT8        multi_status = uimultiview_get_status();
            check_prog_finised = 0;
            b_lock = FALSE;
            if(MV_SCAN_SCREEN & multi_status)
                return PROC_LOOP;
            else
            {
                BOOL age_check;
                P_NODE p_node;
                
                //SYSTEM_DATA* p_sys_data = sys_data_get();
                MEMSET(&p_node,0,sizeof(P_NODE));
                get_prog_at(cur_channel, &p_node);
                MEMCPY(&playing_pnode, &p_node, sizeof(P_NODE));
                ap_set_playing_pnode(&playing_pnode);
                age_check = FALSE;

                uimultiview_draw_one_box(param1);
                sys_data_set_cur_group_channel(cur_channel);
                key_pan_display_channel(cur_channel);

                if((sys_data_get_channel_lock() && (p_node.lock_flag || p_node.provider_lock))|| age_check)
                {
                    g_key_ok = FALSE;
                    pwd_open_state = TRUE;
                    if(win_mv_pwd_open(exit_key, (sizeof(exit_key)/sizeof(UINT32)) ) )
                    {
                        b_lock = FALSE;
                        g_key_ok = TRUE;
                        set_channel_parrent_lock(SIGNAL_STATUS_PARENT_UNLOCK);
                        check_prog_finised = 1;

                    }
                    else
                    {
                        g_key_ok = FALSE;
                        b_lock = TRUE;
                        set_channel_parrent_lock(SIGNAL_STATUS_PARENT_LOCK);
                        check_prog_finised = 1;
                    }
                    win_mv_load_sign_onoff(0, param1);
                    win_mv_update_signal(TRUE,param1);
                }
                pwd_open_state = FALSE;
            }
            break;
        }
        case EVN_UNKNOWNKEY_GOT:
        {
            break;
        }
        default:
            break;
    }

    return PROC_PASS;
}


static VACTION win_com_con_keymap(POBJECT_HEAD obj, UINT32 key)
{
    VACTION act;

    switch(key)
    {
    case V_KEY_YELLOW:
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
        break;
    default:
        act = osd_container_key_map(obj,key);
    }

    return act;
}


static PRESULT win_mv_ctn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 tv_mode;
    struct osdrect    rect;
    //UINT16    i;
    UINT16    channel;
    enum    osdsys    e_osdsys;
    //P_NODE p_node;
    SYSTEM_DATA* sys_data=sys_data_get();

	if(0 == e_osdsys)
	{
		;
	}
#ifdef _INVW_JUICE
int inview_code, ret_code;
#endif
    switch(event)
    {
    case EVN_PRE_OPEN:
#ifdef _INVW_JUICE

if (inview_is_app_paused() == FALSE)
{
ret_code = inview_code_map(V_KEY_ENTER, &inview_code);
inview_pause(FALSE, inview_code);
}
#endif
        if(RADIO_CHAN== sys_data_get_cur_chan_mode())
        {
            ret = PROC_LEAVE;
            break;
        }
        g_key_exit = FALSE;
        b_lock = FALSE;
        win_multiview_ext_set_lock(FALSE);
        enter_mv_mode = TRUE;
        set_box_play_status(FALSE);
        api_osd_mode_change(OSD_NO_SHOW);
        //sys_data_set_palette(1);
#ifdef PARENTAL_SUPPORT
        clear_pre_ratinglock();
#endif
        api_set_system_state(SYS_STATE_9PIC);
        api_stop_play(0);
#ifdef DVR_PVR_SUPPORT//DVR_PVR_SUPPORT
//        if(pvr_info.tms_r_handle != 0)
//            api_pvr_tms_proc(FALSE);
#endif
        win_mv_init();
        win_mv_page_shift(g_mv_top,g_mv_total, 0 );
        break;
    case EVN_PRE_DRAW:
        win_mv_scale_screen();
        break;
    case EVN_PRE_CLOSE:
        api_set_system_state(SYS_STATE_NORMAL);
        lock_play = 0;
        ap_multiview_osd_switch( FALSE );

#if 0
        if(TV_MODE_PAL == tv_mode)
            e_osdsys = OSD_PAL;
        else
            e_osdsys = OSD_NTSC;
#endif
        uimultiview_exit();
        osd_get_rect_on_screen(&rect);
        rect.u_left = 0;
        rect.u_top = 0;
        osd_draw_frame(&rect, OSD_TRANSPARENT_COLOR, NULL);

        rect.u_left = ((1280 - 1008)>>1);//OSD_STARTCOL;
        rect.u_top =    (TV_MODE_PAL == tv_mode)  ? OSD_STARTROW_P : OSD_STARTROW_N ;
        rect.u_width = 1008;//608;
        rect.u_height = 640;//430;

        e_osdsys = OSD_PAL;
        osd_set_rect_on_screen(&rect);
        //OSD_Scale(OSD_VSCALE_OFF, (UINT32)&eOSDSys);
#ifdef PARENTAL_SUPPORT
        clear_pre_ratinglock();
#endif
        si_monitor_register((on_pid_change_t)ap_pid_change);

        if(NULL != (UINT32*)param2)
        {
            UINT32*    evn = (UINT32*)param2;
            *evn &= (!C_CLOSE_CLRBACK_FLG);
        }
        //UIChChgStopProg(TRUE);
        break;
    case EVN_POST_CLOSE:
        g_key_exit = FALSE;
        set_box_play_status(FALSE);
        if(TV_ASPECT_RATIO_169==system_tv_mode)
        {
            win_av_set_ratio(sys_data,3);
            sys_data_set_display_mode(&(sys_data->avset));
        }
        sys_data_set_palette(1);
#ifdef _INVW_JUICE
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)CHANNEL_BAR_HANDLE, FALSE);

#ifdef DISABLE_ALI_INFORBAR
                    ret_code = inview_code_map(V_KEY_ENTER, &inview_code);

                    inview_resume(inview_code);

                    inview_handle_ir(inview_code);
                    //proc_ret = PROC_PASS;



#endif


#else
    {
        channel = sys_data_get_cur_group_cur_mode_channel();
        show_and_playchannel = 0;
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win2_progname, FALSE);
        api_play_channel(channel, TRUE, TRUE,FALSE);
    }
#endif
        break;
    case EVN_MSG_GOT:
        ret =  win_mv_message_proc(param1,param2);
        break;
    default:
        ;
    }

    return ret;
}
static PRESULT win_mv_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;

    UINT8        focus = (UINT8)msg_code;
    UINT8        multi_status;
    //UINT16        cur_posi;
    BOOL        load_status;
    P_NODE p_node;
    BOOL    age_check = FALSE;
    SYSTEM_DATA* p_sys_data;
    UINT32 i;
    UINT32 u_channel_index;

    /*static UINT32    exit_key[] =
            {
                V_KEY_UP, V_KEY_DOWN,
                V_KEY_C_UP, V_KEY_C_DOWN,
                //V_KEY_P_UP, V_KEY_P_DOWN,
                V_KEY_LEFT,V_KEY_RIGHT,
                V_KEY_MENU,V_KEY_EXIT
            };*/
	if(NULL == p_sys_data)
	{
		;
	}
    p_sys_data = sys_data_get();
    multi_status = uimultiview_get_status();

    signal_lock_status lock_flag;
    signal_scramble_status scramble_flag;
    signal_lnbshort_status lnbshort_flag;
    signal_parentlock_status parrentlock_flag;

#ifdef PARENTAL_SUPPORT
    signal_ratinglock_status ratinglock_flag;
    get_signal_status(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag,&ratinglock_flag);
#else
    get_signal_status(&lock_flag,&scramble_flag,&lnbshort_flag,&parrentlock_flag);
#endif

    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW:
         focus = (UINT8)msg_code;
        load_status = !(msg_code&BOX_FINISHED);

        if(msg_code & BOX_SCAN_START)
        {
            for(i=0;i<g_mv_page_cnt;i++)
                win_mv_load_sign_onoff(0,i);
            check_prog_finised = 1;
            mv_state = BOX_SCAN_START;
        }
        else if(load_status)
        {
            //key_set_signal_check_flag(SIGNAL_CHECK_RESET);
            get_prog_at(focus + g_mv_top, &p_node);
#ifdef AGE_LIMIT_FEATURE

            //SYSTEM_DATA*    p_sys_data = sys_data_get();
            age_check = FALSE;
            if(p_sys_data->rating_sel)
            {
                if(rating_check(focus + g_mv_top,1))
                {
                        age_check = TRUE;
                }
            }
#endif
            if((sys_data_get_channel_lock() && (p_node.lock_flag || p_node.provider_lock))|| age_check)
            {
                if((MV_SCAN_SCREEN & multi_status) && g_mv_page_cnt!= 1)
                    b_lock = TRUE;
            }
            else
                b_lock = FALSE;

            win_mv_load_sign_onoff(load_status, focus);
            check_prog_finised = 1;
            mv_state = BOX_SCAN_START;
        }
        else if(msg_code & BOX_FOCUS)//load finish or other circumstance
        {
            get_prog_at(focus + g_mv_top, &p_node);
            win_mv_load_sign_onoff(0, focus);
            if((p_node.lock_flag || p_node.provider_lock))
            {
                uimultiview_draw_one_box(focus);
                sys_data_set_cur_group_channel(focus + g_mv_top);
                key_pan_display_channel(focus + g_mv_top);

            }
            if(msg_code & BOX_FINISHED)
            {
                b_lock = FALSE;
            #ifdef PARENTAL_SUPPORT
                clear_pre_ratinglock();
            #endif
            }
            check_prog_finised = 1;
            mv_state = BOX_FINISHED;
        }
        else if(msg_code & BOX_FINISHED)
        {
            if((MV_SCAN_SCREEN & multi_status)||(MV_ACTIVE & multi_status))
            {

                P_NODE p_node;
                p_sys_data = sys_data_get();
                get_prog_at(focus + g_mv_top, &p_node);
                if((sys_data_get_channel_lock() && (p_node.lock_flag || p_node.provider_lock))&&!g_key_ok)
                    b_lock = TRUE;
                win_mv_update_signal(TRUE,focus);
            }
            check_prog_finised = 1;
            mv_state = BOX_FINISHED;

        }
        else
        {
                win_mv_load_sign_onoff(load_status, focus);
                if(MV_SCAN_SCREEN & multi_status)
                    win_mv_update_signal(TRUE,focus);
                check_prog_finised = 1;
                mv_state = BOX_FINISHED;
        }

        break;
    case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
        if( !(MV_SCAN_SCREEN & multi_status))
        {
            //osal_task_sleep(1);
            chg_time_out++;
        }
        load_status = !(g_load_posi& BOX_FINISHED);
        if( !(MV_SCAN_SCREEN & multi_status) && !load_status && check_prog_finised && (chg_time_out >= 1))
        {
            chg_time_out = 0;
            P_NODE p_node;
            p_sys_data = sys_data_get();
            focus = osd_get_mtrxbox_cur_pos(&g_mtb_mv_name);
            get_prog_at(focus + g_mv_top, &p_node);
            if(get_box_play_status())
                win_mv_update_signal(TRUE,focus);
#ifdef AGE_LIMIT_FEATURE
            if(p_sys_data->rating_sel)
            {
                if(win_multiview_get_age_lock())
                {
                        age_check = TRUE;
                        b_lock = TRUE;
                        if(get_box_play_status())
                            win_mv_update_signal(TRUE,focus);
                }
                else if((sys_data_get_channel_lock() && (p_node.lock_flag || p_node.provider_lock))&&!g_key_ok)
                {
                    b_lock = TRUE;
                    if(get_box_play_status())
                        win_mv_update_signal(TRUE,focus);
                }
                else
                {
                    age_check = FALSE;
                    b_lock = FALSE;
                }
            }

            if(age_check && lock_play != 1)
            {
                //UIChChgLockProg(focus + g_mv_top, 0);
                lock_play = 1;
            }
            else if(!age_check && (lock_play != 2)&& (lock_play != 0))
            {
                lock_play = 2;
                uimultiview_draw_one_box(focus);
                win_multiview_set_age_lock(FALSE);
            }
#else
            if((sys_data_get_channel_lock() && (p_node.lock_flag || p_node.provider_lock))&&!g_key_ok)
                b_lock = TRUE;
            if(get_box_play_status())
                win_mv_update_signal(TRUE,focus);
#endif
        }
        break;
    case CTRL_MSG_SUBTYPE_CMD_AUDIO_PIDCHANGE:
    case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE:
        //soc_printf("CTRL_MSG_SUBTYPE_CMD_PIDCHANGE\n");
        load_status = !(g_load_posi& BOX_FINISHED);
        if( !(MV_SCAN_SCREEN & multi_status) && !load_status && check_prog_finised)
        {
            focus = osd_get_mtrxbox_cur_pos(&g_mtb_mv_name);
            u_channel_index = g_mv_top + focus;
            //soc_printf("uChannelIndex=%d\n",uChannelIndex);
            if(get_prog_at(u_channel_index,&p_node) == SUCCESS && !b_lock)
            {
                //soc_printf("play channel\n");
                //api_play_channel(uChannelIndex, TRUE, !b_lock, FALSE);
                uimultiview_draw_one_box(focus);

                //cc_stop_channel(0,0);
                //cc_play_channel(p_node.prog_id, 0);
            }
        }
        else if( !(MV_SCAN_SCREEN & multi_status) && !b_lock)
        {
            //soc_printf("Resend CTRL_MSG_SUBTYPE_CMD_PIDCHANGE\n");
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PIDCHANGE, msg_code,FALSE);
        }
        break;

    case CTRL_MSG_SUBTYPE_CMD_AGE_LIMIT_UPDATED:
    default:
        ret = PROC_PASS;
    }

    return ret;
}


