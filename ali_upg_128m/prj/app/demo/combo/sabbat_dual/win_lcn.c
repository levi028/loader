#include <sys_config.h>

#include <types.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com.h"

#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_hde.h>
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>
#endif

#include "menus_root.h"

#ifdef _LCN_ENABLE_
//#ifndef DVBS_SUPPORT
/*******************************************************************************
*    Objects definition
*******************************************************************************/
extern CONTAINER g_win_lcn;

extern CONTAINER lcn_item_con1;
extern TEXT_FIELD lcn_item_txtname1;
extern TEXT_FIELD lcn_item_txtset1;


static VACTION lcn_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT lcn_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION lcn_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT lcn_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);
static BOOL current_frontend_is_isdbt=FALSE;
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#endif

#define CON_L        (W_L+20)
#define CON_T        (W_T + 4)
#define CON_W        (W_W - 60)
#define CON_H        40
#define CON_GAP        12

#define TXT_L_OF      10
#define TXT_W          300
#define TXT_H        CON_H
#define TXT_T_OF    ((CON_H - TXT_H)/2)

#define SEL_L_OF      (TXT_L_OF + TXT_W)
#define SEL_W          (CON_W-320)
#define SEL_H        CON_H
#define SEL_T_OF    ((CON_H - SEL_H)/2)

#define LINE_L_OF    0
#define LINE_T_OF      (CON_H+4)
#define LINE_W        CON_W
#define LINE_H         4


#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    lcn_item_con_keymap,lcn_item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_TXTNAME(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTSET(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,res_id,str)


#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt_name,var_txtset,ID,idu,idd,l,t,w,h,name_id,setstr)    \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt_name,1)    \
    LDEF_TXTNAME(&var_con,var_txt_name,&var_txtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,name_id,NULL)    \
    LDEF_TXTSET(&var_con,var_txtset,  NULL        ,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)

LDEF_MM_ITEM(g_win_lcn,lcn_item_con1, NULL,lcn_item_txtname1,lcn_item_txtset1,1,1,1,    \
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H,    RS_INSTALLATION_LCN,NULL)

DEF_CONTAINER(g_win_lcn,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    lcn_con_keymap,lcn_con_callback,  \
    (POBJECT_HEAD)&lcn_item_con1, 1,0)


/*******************************************************************************
*    Local functions & variables define
*******************************************************************************/
CONTAINER* lcn_items[] =
{
    &lcn_item_con1,
};

POBJECT_HEAD lcn_item_name[] =
{
    (POBJECT_HEAD)&lcn_item_txtname1,
};

POBJECT_HEAD lcn_item_set[] =
{
    (POBJECT_HEAD)&lcn_item_txtset1,
};


void win_lcn_set_string(UINT8 input)
{
    if(input==0)
        osd_set_text_field_content(&lcn_item_txtset1, STRING_ID,RS_COMMON_OFF);
    else
        osd_set_text_field_content(&lcn_item_txtset1, STRING_ID,RS_COMMON_ON);
}

/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static VACTION lcn_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_RIGHT:
        act = VACT_INCREASE;
        break;
    case V_KEY_LEFT:
        act = VACT_DECREASE;
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


PRESULT comlist_menu_lcn_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	//UINT16 i =0,w_top =0;
	//UINT8 id =0;
	//char str_len =0;
    	OBJLIST* ol = NULL;
	//UINT8 temp[4] ={0};

    PRESULT cb_ret = PROC_PASS;

        ol = (OBJLIST*)p_obj;

    if(event==EVN_PRE_DRAW)
    {
    }
    else if(event == EVN_POST_CHANGE)
        cb_ret = PROC_LEAVE;

    return cb_ret;
}


void win_lcn_left_right_key(POBJECT_HEAD p_obj,UINT8 id)
{
    sys_data_set_lcn(!sys_data_get_lcn());
    win_lcn_set_string(sys_data_get_lcn());
    osd_track_object(p_obj,C_UPDATE_ALL);
}

static PRESULT lcn_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	UINT8	unact =0;//,input=0;
    UINT8    id = osd_get_obj_id(p_obj);

    switch(event)
    {
    case EVN_FOCUS_PRE_GET:
        break;

    case EVN_FOCUS_PRE_LOSE:
        break;

    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16) ;
        if((unact == VACT_INCREASE) || (unact == VACT_DECREASE))
        {
            win_lcn_left_right_key(p_obj,id);
        }
        if(sys_data_get_lcn())
            sort_prog_node(PROG_LCN_SORT);
        else
            sort_prog_node(PROG_DEFAULT_SORT);
        update_data();
        ret = PROC_LOOP;
        break;
    default:
        break;
    }
    return ret;

}

static VACTION lcn_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT lcn_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
	//UINT8	unact =0;
	UINT32 i=0,j=0;
	//BOOL subt_flag = FALSE;
	//CONTAINER* cont = (CONTAINER*)p_obj;
	//UINT8	id = osd_get_obj_id(p_obj);
	UINT16 title_id =0;
	UINT8 group_type =0,  group_pos=0;
	UINT16 channel=0,ch_cnt=0;
	UINT8 av_flag =0;
    static P_NODE p_node1;
    P_NODE p_node2;
    UINT8    tmp_flag = 1;
    MEMSET(&p_node2, 0, sizeof(p_node2));
    switch(event)
    {
    case EVN_PRE_OPEN:
        if(param2 != MENU_OPEN_TYPE_STACK)
        {
            if((param2 & MENU_FOR_ISDBT) == MENU_FOR_ISDBT)
                current_frontend_is_isdbt = TRUE;
            else
                current_frontend_is_isdbt = FALSE;
        }
        if(current_frontend_is_isdbt)
            title_id = RS_INSTALLATION_VCN;
        else
             title_id = RS_INSTALLATION_LCN;
        wincom_open_title((POBJECT_HEAD)&g_win_lcn,title_id, 0);
        osd_set_text_field_content(&lcn_item_txtname1, STRING_ID, title_id);

        win_lcn_set_string(sys_data_get_lcn());
        osd_draw_object((POBJECT_HEAD )&lcn_item_con1,C_UPDATE_ALL);
        sys_data_get_curprog_info(&p_node1);
        break;

    case EVN_POST_OPEN:
        break;

    case EVN_PRE_CLOSE:
        sys_data_save(1);
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;

    case EVN_POST_CLOSE:
        sys_data_get_cur_mode_group_infor(sys_data_get_cur_group_index(),&group_type, &group_pos, &channel);
        av_flag = sys_data_get_cur_chan_mode();
        ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);

        for( i=0; i<ch_cnt; i++ )
        {
            get_prog_at( i, &p_node2 );
            for( j=0; j<2*(MAX_SERVICE_NAME_LENGTH + 1); j++ )
            {
                if( p_node1.service_name[j] != p_node2.service_name[j])
                {
                    tmp_flag = 0;
                    break;
                }
            }
            if(tmp_flag)
            {
                sys_data_set_cur_group_channel(i);
                break;
            }
            else
            {
                tmp_flag = 1;
            }
        }
        break;
    default:
        break;
    }
    return ret;
}

//#endif
#endif
