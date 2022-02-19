/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_country_net.c
*
*    Description: The realize of country setting
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

#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"
#include "menus_root.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_search.h"
#include "win_countryband.h"

//#ifdef SHOW_WELCOME_SCREEN

/*
typedef struct
{
    UINT32 sym;
    UINT32 qam_mode;
}SCAN_NET;
*/

/*******************************************************************************
*   Objects definition
*******************************************************************************/
CONTAINER win_country_net_con;
#define GERM_TYPE_COUNT  4

#ifndef SD_UI
#define MAIN_W_L        74//210 - 136
#define MAIN_W_T        98//138 - 40
#ifdef SUPPORT_CAS_A
#define MAIN_W_W        886
#else
#define MAIN_W_W        866
#endif
#define MAIN_W_H        488
#else
#define MAIN_W_L        17//210 - 136
#define MAIN_W_T        57//138 - 40
#define MAIN_W_W        570
#define MAIN_W_H        370
#endif

static VACTION item_sel_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT item_sel_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2);

static VACTION con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT con_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2);
static VACTION item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT item_con_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX    WSTL_WIN_BODYRIGHT_01_HD
// WSTL_NL_MENU_BG_IDX //WSTL_WIN_BODYRIGHT_01_HD

#define CON_SH_IDX   WSTL_BUTTON_01_HD//WSTL_NL_NOSHOW_IDX
#define CON_HL_IDX   WSTL_BUTTON_05_HD//WSTL_NL_NOSHOW_IDX
#define CON_SL_IDX   WSTL_BUTTON_01_HD//WSTL_NL_NOSHOW_IDX
#define CON_GRY_IDX  WSTL_BUTTON_07_HD//WSTL_NL_NOSHOW_IDX


#define TXT_SH_IDX   WSTL_BUTTON_01_FG_HD//WSTL_NL_TXT_HL_IDX
#define TXT_HL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_NL_TXT_HL_IDX
#define TXT_SL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_NL_TXT_HL_IDX
#define TXT_GRY_IDX  WSTL_BUTTON_07_HD//WSTL_NL_TXT_SH_IDX


#define SEL_SH_IDX   WSTL_BUTTON_01_HD//WSTL_BUTTON_01_FG_HD
#define SEL_HL_IDX   WSTL_BUTTON_04_HD//WSTL_NL_BUTTON_SELECT_HD
#define SEL_SL_IDX   WSTL_BUTTON_01_FG_HD//WSTL_NL_BUTTON_03_FG_HD
#define SEL_GRY_IDX  WSTL_BUTTON_07_HD//WSTL_NL_BUTTON_03_GRY_HD


#define TXT_BUTTON_IDX      WSTL_BUTTON_05_8BIT//WSTL_NL_BUTTON_01_HD

//#define WIN_SH_IDX        WSTL_NL_MENU_BG_IDX //WSTL_WIN_BODYRIGHT_01_HD
//#define MTXT_SH_IDX       WSTL_NL_TXT_HL_IDX

#define BTN_SH_IDX   WSTL_BUTTON_SHOR_HD
//WSTL_NL_BUTTON_03_FG_HD //WSTL_BUTTON_SHOR_HD
#define BTN_HL_IDX   WSTL_BUTTON_SHOR_HI_HD
//WSTL_NL_BUTTON_01_HD //WSTL_BUTTON_SHOR_HI_HD
#define BTN_SL_IDX   WSTL_BUTTON_01_FG_HD //WSTL_BUTTON_01_FG_HD
#define BTN_GRY_IDX  WSTL_BUTTON_SHOR_GRY_HD//WSTL_BUTTON_SHOR_GRY_HD


#define BTN_W       160
#ifndef SD_UI
#define BTN_H       44
#else
#define BTN_H       32
#endif
/*
static char *country_str[] =
{
    "Germany",
    "Ausria",
    "Switzerland",
    "Italy",
    "Greece",
    "Portugal",
};
*/

static UINT16 country_str[] =
{
    RS_COUNTRY_GERMANY,
    RS_COUNTRY_AUSRIA,
    RS_COUNTRY_SWITZERLAND,
    RS_COUNTRY_ITALY,
    RS_COUNTRY_GREECE,
    RS_COUNTRY_PORTUGAL,
    RS_REGION_ARG,
    RS_REGION_DEN,
    RS_REGION_FRA,
    RS_REGION_POL,
    RS_REGION_RUS,
    RS_REGION_SPA,
};

/*
static char *network_str[] =
{
    "6.900MS - QAM64",
    "6.900MS - QAM256",
    "6.900MS - QAM128",
    "6.875MS - QAM64",
    "6.875MS - QAM256",
    "6.875MS - QAM128",
    "6.111MS - QAM64",
    "6.111MS - QAM256",
    "6.111MS - QAM128",
};

static UINT16 germany_servers_str[] =
{
   RS_GERMANY_KABEL_DEUTSCHLAND,
   RS_GERMANY_KABEL_BADEN,
   RS_GERMANY_UNITYMEDIA,
   RS_GERMANY_TELE_CONLUMBER,
};
*/

static UINT16 network_str[] =
{
   RS_GERMANY_KABEL_DEUTSCHLAND,
   RS_GERMANY_KABEL_BADEN,
   RS_GERMANY_UNITYMEDIA,
   RS_GERMANY_TELE_CONLUMBER,
    RS_6900MS_QAM16,
    RS_6900MS_QAM32,
    RS_6900MS_QAM64,                // network_str+4
    RS_6900MS_QAM256,
    RS_6900MS_QAM128,
    RS_6875MS_QAM16,
    RS_6875MS_QAM32,
    RS_6875MS_QAM64,
    RS_6875MS_QAM256,
    RS_6875MS_QAM128,
    RS_6111MS_QAM16,
    RS_6111MS_QAM32,
    RS_6111MS_QAM64,
    RS_6111MS_QAM256,
    RS_6111MS_QAM128,
};
/*
static char *germany_servers_str[] =
{
    "Kabel Deutschland",
    "Kabel Baden W¨¹rttemberg",
    "Unitymedia",
    "Tele Conlumber,Primacom",
};
*/

#define COUNTRY_NCNT    ARRAY_SIZE(country_str)
#define NETWORK_NCNT    15 //ARRAY_SIZE(network_str)
#define GERM_SER_CNT  4+15

#define LDEF_CON(root, var_con,nxt_obj,\
    ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,\
    CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    item_con_keymap,item_con_callback,  \
    conobj, focus_id,1)

#define LDEF_LABEL(root,var_txt,nxt_obj,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,NULL)

/*
#define LDEF_SET(root,varTxt,nxtObj,\
    ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)       \
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTS_SH_IDX,\
    TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,resID,str)
*/

#define LDEF_MSEL(root, var_num, nxt_obj, ID, l, t, w, h,style,cur,cnt,ptabl) \
    DEF_MULTISEL(var_num,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    item_sel_keymap,item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)


#define LDEF_BUTTON(root,var_txt,nxt_obj,ID,l,t,w,h,res_id)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,0,0,0,0, l,t,w,h, BTN_SH_IDX,BTN_HL_IDX,BTN_SL_IDX,BTN_GRY_IDX,   \
    item_sel_keymap,item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,res_id,NULL)

/*
#define LDEF_MM_ITEM(root,varCon,nxtObj,\
    varTxtName,varTxtset,ID,IDu,IDd,l,t,w,h,resID,setstr)   \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxtName,1)    \
    LDEF_LABEL(&varCon,varTxtName,&varTxtset,\
    0,0,0,0,0,l + TXTN_L_OF, \
    t + TXTN_T_OF,TXTN_W,TXTN_H,resID,NULL)   \
    LDEF_SET(&varCon,varTxtset,  NULL ,\
    1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,\
    TXTS_W,TXTS_H,0,setstr)\
*/

#define LDEF_MM_ITEM(root,var_con,nxt_obj,var_txt,var_txtset,ID,idu,idd,l,t,w,h,\
                        res_id,style,cur,cnt,ptbl) \
    LDEF_CON(&root,var_con,nxt_obj,ID,ID,ID,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_LABEL(&var_con,var_txt,&var_txtset,\
    l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W,TXTN_H,res_id)    \
    LDEF_MSEL(&var_con,var_txtset,NULL,ID, \
    l + TXTS_L_OF ,t + TXTS_T_OF,\
    TXTS_W,TXTS_H,style,cur,cnt,ptbl)\

#define LDEF_MM_ITEM_BUTTON(root,var_con,nxt_obj,\
    var_txt,ID,idl,idr,idu,idd,l,t,w,h,res_id) \
    LDEF_CON(&root,var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,&var_txt,ID)   \
    LDEF_BUTTON(&var_con,var_txt,NULL,ID,l,t,w,h,res_id)


#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)     \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    con_keymap,con_callback,  \
    nxt_obj, focus_id,0)


LDEF_MM_ITEM(win_country_net_con,item_country_con, \
                &item_network_con,item_country_label,item_country_sel,1,4,2, \
                CON_L, CON_T + (CON_H + CON_GAP)*0,\
        CON_W,CON_H, RS_SYSTEM_COUNTRY, \
                STRING_ID,0,COUNTRY_NCNT,country_str)

LDEF_MM_ITEM(win_country_net_con,item_network_con, \
                &item_button_prev_con,item_network_label,\
        item_network_sel,2,1,4,    \
                CON_L, CON_T + (CON_H + CON_GAP)*1,\
        CON_W,CON_H, RS_SYSTEM_NETWORK, \
                STRING_ID,0,GERM_SER_CNT,network_str)

LDEF_MM_ITEM_BUTTON(win_country_net_con, item_button_prev_con,\
               &item_button_next_con, item_button_prev_label,\
            3,4,4,2,4, \
                CON_L, CON_T + (CON_H + CON_GAP)*7, \
        BTN_W, BTN_H, RS_DISPLAY_BACK)


LDEF_MM_ITEM_BUTTON(win_country_net_con, item_button_next_con, \
                NULL, item_button_next_label, 4,3,3,3,1, \
                CON_W-50, CON_T + (CON_H + CON_GAP)*7, \
        BTN_W, BTN_H, RS_DISPLAY_NEXT)

/*
DEF_CONTAINER(win_country_net_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H,\
    WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    con_keymap,con_callback,  \
    (POBJECT_HEAD)&item_country_con, 1,0)
*/

LDEF_WIN(win_country_net_con,(POBJECT_HEAD)&item_country_con,W_L,W_T,W_W,W_H,1)

enum
{
    COUNTRY_ID = 1,
    NETWORK_ID,
    BUTTON_PREV_ID,
    BUTTON_NEXT_ID,
    MAX_ID
};

static CONTAINER *con_items[] =
{
    &item_country_con,
    &item_network_con,
    &item_button_prev_con,
    &item_button_next_con,
};

static MULTISEL *multisel_items[] =
{
    &item_country_sel,
    &item_network_sel,
};




static union as_frontend_param all_network[]=
{
    {{QAM16,6900}},
    {{QAM32,6900}},
    {{QAM64,6900}},
    {{QAM256,6900}},
    {{QAM128,6900}},
    {{QAM16,6875}},
    {{QAM32,6875}},
    {{QAM64,6875}},
    {{QAM256,6875}},
    {{QAM128,6875}},
    {{QAM16,6111}},
    {{QAM32,6111}},
    {{QAM64,6111}},
    {{QAM256,6111}},
    {{QAM128,6111}}
};


#if 1//def DVBC_MODE_CHANGE
static char *all_network_qamb_str[]=
{
    "5.057MS - QAM64",
    "5.361MS - QAM256",
};

static union as_frontend_param all_network_qamb[]=
{
    {{QAM64,5057}},
    {{QAM256,5361}},
};

#define MAX_QAMB_NETWORK_NUM ARRAY_SIZE(all_network_qamb_str)
#endif

/*
static SCAN_NET all_network[]=
{
    {6900,QAM64},
    {6900,QAM256},
    {6900,QAM128},
    {6875,QAM64},
    {6875,QAM256},
    {6875,QAM128},
    {6111,QAM64},
    {6111,QAM256},
    {6111,QAM128}
};
*/


static UINT8 old_country_sel = 0;
static UINT8 old_network_sel = 0;
static UINT8 old_germ_serv_sel = 0;

BOOL from_auto_scan = FALSE;

void win_contry_net_set_entry(BOOL auto_scan)
{
    from_auto_scan = auto_scan;
  //  from_auto_scan = FALSE;
}

static UINT32 network_setting_to_osd(union as_frontend_param *network)
{
    UINT32 i=0;

    for(i=0;i<NETWORK_NCNT;i++)
    {
        if((network->c_param.sym == all_network[i].c_param.sym )\
       && (network->c_param.constellation == all_network[i].c_param.constellation))
    {
            return i;
    }
    }
    return 0;
}


static UINT32 qamb_network_setting_to_osd(union as_frontend_param *network)
{
#ifdef DVBC_MODE_CHANGE
    UINT32 i=0;

    for(i=0;i<NETWORK_NCNT;i++)
    {
        if((network->c_param.sym == all_network_qamb[i].c_param.sym )\
       && (network->c_param.constellation == all_network_qamb[i].c_param.constellation))
        {
            return i;
        }
    }
#endif    
    return 0;
}


static UINT32 osd_to_network_setting(UINT32 sel, \
              union as_frontend_param *network)
{
    if(sel >= NETWORK_NCNT)
    {
        sel = 0;
    }
    network->c_param.constellation = all_network[sel].c_param.constellation;
    network->c_param.sym = all_network[sel].c_param.sym;
    return sel;
}

static UINT32 osd_to_germ_serv_setting(UINT32 sel)
{
    SYSTEM_DATA *sys_data=NULL;

    sys_data = sys_data_get();

    if(sel >= GERM_SER_CNT)
    {
        sel = 0;
    }
    if(GERM_TYPE_COUNT<=sel )
    {
        sys_data->current_ft[0].c_param.sym = all_network[sel-4].c_param.sym;
        sys_data->current_ft[0].c_param.constellation = \
    all_network[sel-4].c_param.constellation;
        sys_data->current_ft_count = 1;
        sys_data->germ_servs_type = sel;
        return sel;
    }
    switch(sel)
    {
        case 0:   //Kabel Deutschland: (6.900MS QAM 256; 6.900MS QAM64)
        case 1:   //Kabel Baden W¨¹rttemberg:( 6.900MS QAM 256; 6.900MS QAM64)
        case 2:   //Unitymedia:(6.900MS QAM 256; 6.900MS QAM64)
        case 3:   //Unitymedia:(6.900MS QAM 256; 6.900MS QAM64)
            sys_data->current_ft[0].c_param.sym = 6900;
            sys_data->current_ft[1].c_param.sym = 6900;
            sys_data->current_ft[0].c_param.constellation = QAM256;
            sys_data->current_ft[1].c_param.constellation = QAM64;
            sys_data->current_ft_count = 2;
            break;
    //    case 3:  //Tele Conlumber,Primacom:6.900MS QAM64
    //        sys_data->current_ft[0].c_param.sym = 6900;
    //       sys_data->current_ft[0].c_param.constellation = QAM64;
    //        sys_data->current_ft_count = 1;

        default:
            break;
    }
    sys_data->germ_servs_type = sel;
    return sel;
}


static void load_settings(void)
{
    MULTISEL *msel=NULL;
    SYSTEM_DATA *sys_data=NULL;
    UINT32 sel=0;

    sys_data = sys_data_get();

    //country
    msel = &item_country_sel;
    osd_set_multisel_count(msel, COUNTRY_NCNT);
    sel = sys_data->country;
    osd_set_multisel_sel(msel, sel);
    old_country_sel = sel;

    //network
    msel = &item_network_sel;

    if(GERMANY == sys_data->country)
    {
        osd_set_multisel_count(msel, GERM_SER_CNT);
        msel->b_sel_type = STRING_ID;
        msel->p_sel_table = network_str;
        sel =  sys_data->germ_servs_type;
        //germ_network_setting_to_osd(&(sys_data->current_ft),
        // sys_data->current_ft_count);
        old_germ_serv_sel = sel;
        old_network_sel = 0;
    }
    else
    {
        osd_set_multisel_count(msel, NETWORK_NCNT);
        msel->b_sel_type = STRING_ID; //STRING_ANSI;
        msel->p_sel_table = network_str+4;
        sel = network_setting_to_osd(&(sys_data->current_ft[0]));
        old_germ_serv_sel = 0;
        old_network_sel = sel;
    }
    osd_set_multisel_sel(msel, sel);

#ifdef DVBC_MODE_CHANGE
    if(NUM_ONE==sys_data->dvbc_mode)
    {
        msel = &item_network_sel;
        msel->p_sel_table = (void *)all_network_qamb_str;
        msel->b_sel_type = STRING_ANSI; //STRING_ANSI;
        msel->n_sel = 0;
        msel->n_count = MAX_QAMB_NETWORK_NUM;
        osd_draw_object((POBJECT_HEAD)msel, C_UPDATE_ALL);

        old_country_sel = 0;

        sys_data->current_ft[0].c_param.sym = all_network_qamb[0].c_param.sym;
        sys_data->current_ft[0].c_param.constellation = \
    all_network_qamb[0].c_param.constellation;
        sys_data->current_ft_count = 1;
        old_network_sel = 0;
    }
#endif

    //if (from_auto_scan)
        //OSD_SetContainerFocus( (CONTAINER*)&win_country_net_con,NETWORK_ID);
    //else
        osd_set_container_focus( (CONTAINER*)&win_country_net_con,COUNTRY_ID);

}


static UINT8 get_current_dvbc_mode(void)
{
    SYSTEM_DATA * sys_data = sys_data_get();
    UINT8 dvbc_mode = 0;//J8AC
    if(sys_data)
    {
        #ifdef DVBC_MODE_CHANGE
        dvbc_mode = sys_data->dvbc_mode;
        #endif
    }

    return dvbc_mode;
}


UINT8 win_country_net_get_num(void)
{
    UINT8 id = osd_get_focus_id((POBJECT_HEAD)&win_country_net_con);
    //SYSTEM_DATA *p_sys_data=sys_data_get();
    UINT8 num=0;

    switch(id)
    {
    case COUNTRY_ID:
        num=COUNTRY_NCNT;
        break;
    case NETWORK_ID: 
        if(get_current_dvbc_mode())
        {
            //J83B
            num = MAX_QAMB_NETWORK_NUM;
        }
        else
        {
            //J83AC
            if(GERMANY == osd_get_multisel_sel(&item_country_sel))
            {
                num = GERM_SER_CNT;
            }
            else
            {
                num = NETWORK_NCNT;
            }
        }       
        break;
    default:
        break;
    }
    return num;
}


PRESULT comlist_country_net_callback(POBJECT_HEAD pobj,\
        VEVENT event, UINT32 param1, UINT32 param2)
{
    UINT16 i=0;
    UINT16 wtop=0;
    UINT8 id=0;
    //char str_len;
    OBJLIST *ol=NULL;
    //UINT8 temp[4];

    //SYSTEM_DATA *p_sys_data=sys_data_get();
    MULTISEL *msel=NULL;

    PRESULT cbret = PROC_PASS;

    ol = (OBJLIST*)pobj;

    id = osd_get_focus_id((POBJECT_HEAD)&win_country_net_con);

    if(EVN_PRE_DRAW == event)
    {
        switch(id)
        {
            case COUNTRY_ID:
                    wtop = osd_get_obj_list_top(ol);
                    for(i=0;i<ol->w_dep && (i+wtop)<ol->w_count;i++)
                    {
                    win_comlist_set_str(i + wtop,NULL,NULL,country_str[i+wtop]);
                    }
                break;
            case NETWORK_ID:
                wtop = osd_get_obj_list_top(ol);
                msel = &item_country_sel;
                if(GERMANY == osd_get_multisel_sel(msel))
                {
                    for(i=0;i<ol->w_dep && (i+wtop)<ol->w_count;i++)
                    {
                        win_comlist_set_str(i + wtop, \
                            NULL,NULL,network_str[i+wtop]);
                    }
                }
                else
                {
                    for(i=0;i<ol->w_dep && (i+wtop)<ol->w_count;i++)
                    {
                        win_comlist_set_str(i + wtop, \
                            NULL,NULL,network_str[i+wtop+4]);
                    }
                }

                if(get_current_dvbc_mode())
                {
                    for(i=0;i<ol->w_dep && (i+wtop)<ol->w_count;i++)
                    {
                        win_comlist_set_str(i + wtop,\
                            all_network_qamb_str[i],NULL,0);
                    }
                }
                
                break;
               default:
                    break;
        }
    }
    else if(EVN_POST_CHANGE == event)
    {
        cbret = PROC_LEAVE;
    }
    return cbret;
}


static void win_item_enter_key(UINT8 id)
{
    SYSTEM_DATA *p_sys_data=sys_data_get();
    //UINT16 result;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    UINT16 sel=0;
    POBJECT_HEAD pobj = NULL;
    POBJECT_HEAD pobjitem = NULL;
    MULTISEL *msel=NULL;

    MEMSET(&rect,0,sizeof(OSD_RECT));
    MEMSET(&param,0,sizeof(COM_POP_LIST_PARAM_T));
    MEMSET(&pobjitem,0,sizeof(POBJECT_HEAD));
    MEMSET(&param,0,sizeof(POBJECT_HEAD));
    msel = multisel_items[id - 1];
    pobj = (POBJECT_HEAD)msel;
    pobjitem = (POBJECT_HEAD)con_items[id - 1];

    rect.u_left=CON_L+TXTS_L_OF;
    #ifndef SD_UI
    rect.u_width=TXTS_W;
    #else
    rect.u_width=TXTS_W+20;
    #endif
    rect.u_top = pobj->frame.u_top;
    param.selecttype = POP_LIST_SINGLESELECT;


    if(COUNTRY_ID == id)
    {
        param.cur = p_sys_data->country;
    }
    else if(NETWORK_ID == id)
    {
        if(GERMANY == osd_get_multisel_sel(&item_country_sel))
        {
            param.cur = p_sys_data->germ_servs_type;
        }
        //germ_network_setting_to_osd(&(p_sys_data->current_ft),
        // p_sys_data->current_ft_count);
        else
        {
            param.cur = network_setting_to_osd(&(p_sys_data->current_ft[0]));
        }

        if(get_current_dvbc_mode())
        {
            param.cur = qamb_network_setting_to_osd(&(p_sys_data->current_ft[0]));
        }
    }

    rect.u_height=300;
    sel  = win_com_open_sub_list(POP_LIST_MENU_COUNTRY_NETWORK,&rect,&param);
    if (sel == param.cur)
    {
        return;
    }
    osd_set_multisel_sel(msel, sel);
    osd_notify_event(pobj,EVN_POST_CHANGE,(UINT32)sel, 0);
    osd_track_object(pobjitem,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

}




static VACTION item_sel_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    UINT8   bid=0;
    VACTION act = VACT_PASS;

    bid = osd_get_obj_id(pobj);
    switch(key)
    {
    case V_KEY_RIGHT:
        if(bid<BUTTON_PREV_ID)
    {
        act = VACT_INCREASE;
    }
        break;
    case V_KEY_LEFT:
        if(bid<BUTTON_PREV_ID)
    {
        act = VACT_DECREASE;
    }
        break;
    case V_KEY_ENTER:
        act = VACT_ENTER;
        break;
    default:
        break;
    }
    return act;
}


static PRESULT item_sel_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    SYSTEM_DATA *sys_data=NULL;
    union as_frontend_param network;
    UINT8 bid=0;
    UINT32 sel=0;

    POBJECT_HEAD submenu = NULL;
    MULTISEL *msel=NULL;
    UINT8 i=0;

    MEMSET(&network,0,sizeof(union as_frontend_param));
    bid = osd_get_obj_id(pobj);
    sys_data = sys_data_get();

    switch(event)
    {
        case EVN_PRE_CHANGE:
            sel = *((UINT32*)param1);
            break;
        case EVN_POST_CHANGE:
            sel = param1;
            sys_data->rev2 = 0;
            switch(bid)
            {
                case COUNTRY_ID:
#ifdef DVBC_MODE_CHANGE
                    if(NUM_ONE==sys_data->dvbc_mode )
                    {
                        if(old_network_sel >= MAX_QAMB_NETWORK_NUM)
                        {
                            old_network_sel = MAX_QAMB_NETWORK_NUM-1;
                        }
                        
                        msel = &item_network_sel;
                        msel->p_sel_table = (void *)all_network_qamb_str;
                        msel->b_sel_type = STRING_ANSI;
                        msel->n_sel = old_network_sel;
                        msel->n_count = MAX_QAMB_NETWORK_NUM;
                        osd_draw_object((POBJECT_HEAD)msel, C_UPDATE_ALL);
                        
                        sys_data->country = (COUTRY_TYPE)sel;
                        old_country_sel = sel;
                        break;
                    }
 #endif
                    sys_data->country = (COUTRY_TYPE)sel;
                    if(GERMANY == (COUTRY_TYPE)sel&& GERMANY != old_country_sel)
                    {
                        msel = &item_network_sel;
                        msel->p_sel_table = (void *)network_str;
                        msel->b_sel_type = STRING_ID;
                        msel->n_sel = old_germ_serv_sel;
                        msel->n_count = GERM_SER_CNT;
                        osd_draw_object((POBJECT_HEAD)msel, C_UPDATE_ALL);
                    }
                    else if(GERMANY == old_country_sel&& GERMANY != (COUTRY_TYPE)sel)
                    {
                        msel = &item_network_sel;
                        msel->p_sel_table = (void *)(network_str+4);
                        msel->b_sel_type = STRING_ID; //STRING_ANSI;
                        msel->n_sel = old_network_sel;
                        msel->n_count = NETWORK_NCNT;
                        osd_draw_object((POBJECT_HEAD)msel, C_UPDATE_ALL);
                    }
                    old_country_sel = sel;
                    
                    break;
                case NETWORK_ID:

                    #ifdef DVBC_MODE_CHANGE
                    if(NUM_ONE==sys_data->dvbc_mode)
                    {
                        sys_data->current_ft[0].c_param.sym =all_network_qamb[sel].c_param.sym;
                        sys_data->current_ft[0].c_param.constellation =all_network_qamb[sel].c_param.constellation;
                        sys_data->current_ft_count = 1;
                        old_network_sel = sel;
                        break;
                    }
                    #endif

                      if(GERMANY == osd_get_multisel_sel(&item_country_sel))
                      {
                            osd_to_germ_serv_setting(sel);
                            old_germ_serv_sel = sel;
                      }
                      else
                      {
                        osd_to_network_setting(sel, &network);
                           sys_data->current_ft[0].c_param.sym =network.c_param.sym;
                           sys_data->current_ft[0].c_param.constellation = network.c_param.constellation;
                        sys_data->current_ft_count = 1;
                           old_network_sel = sel;
                      }

                //  sys_data = sys_data_get();

                    break;
                default:
                    break;
            }
            break;

        case EVN_FOCUS_PRE_GET:
            break;
        case EVN_FOCUS_PRE_LOSE:
            break;
        case EVN_UNKNOWN_ACTION:
            if(BUTTON_PREV_ID == bid)
            {
                MEMSET(&submenu,0,sizeof(POBJECT_HEAD));
//modify for the title of language submenu show welcome 2011 11 24
//welcome page has been solve by other method,
// not use the language submenu anymore
        //extern void set_win_lang_as_welcome(BOOL iswelcome);
        //set_win_lang_as_welcome(TRUE);
                submenu = (POBJECT_HEAD)&win_lan_con;
                if (osd_obj_open(submenu, 0xFFFFFFFF) != PROC_LEAVE)
                {
                    menu_stack_pop();
                    menu_stack_push(submenu);
                    ret = PROC_LOOP;
                }

            }
            else if(BUTTON_NEXT_ID == bid)
            {
                  for(i=sys_data->current_ft_count; i<MAX_FRONTEND_PARAM; i++)
                  {
                       sys_data->current_ft[i].c_param.sym = 0xff;
                       sys_data->current_ft[i].c_param.constellation= 0xff;
                  }

                  if(!from_auto_scan)
          {
              sys_data_save(1);
          }
          from_auto_scan = FALSE;

                  get_default_bandparam(sys_data->country, \
          (band_param *)&u_country_param);


                gaui_set_search_param_full();

                MEMSET(&submenu,0,sizeof(POBJECT_HEAD));
                submenu = (POBJECT_HEAD)&g_win2_search;
                if (osd_obj_open(submenu, 0xFFFFFFFF) != PROC_LEAVE)
                {
                    menu_stack_pop();
                    menu_stack_push(submenu);
                    ret = PROC_LOOP;
                }
            }
            else
        {
            win_item_enter_key(bid);
        }
                break;
        default:
                break;
    }

    return ret;
}

static VACTION item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    return VACT_PASS;
}

static PRESULT item_con_callback(POBJECT_HEAD pobj, \
               VEVENT event, UINT32 param1, UINT32 param2)
{
    return PROC_PASS;
}
static VACTION con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act=0;

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
        if(from_auto_scan)
        {
            act = VACT_CLOSE;

        }
        break;
    case V_KEY_RIGHT:
            act=VACT_CURSOR_RIGHT;
        break;
    case V_KEY_LEFT:
            act=VACT_CURSOR_LEFT;
        break;

    default:
        act = VACT_PASS;
        break;
    }
    return act;
}

static PRESULT con_callback(POBJECT_HEAD pobj, \
       VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;


    switch(event)
    {
        case EVN_PRE_OPEN:
            if(from_auto_scan)
//don't need for checking "from_auto_scan" any more,because we don't use this file when show welcome page
            {
                wincom_open_title((POBJECT_HEAD)&win_country_net_con,RS_INSTALLATION_AUTO_SEARCH, 0);
                osd_set_attr(&item_country_con, C_ATTR_ACTIVE/*C_ATTR_HIDDEN*/);
                //modify bomur 2012 0213
                osd_set_attr(&item_button_prev_con, C_ATTR_HIDDEN);
                osd_set_rect(&(item_button_next_con.head.frame),\
                CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H);
                osd_set_rect(&(item_button_next_label.head.frame),\
                 CON_L, CON_T + (CON_H + CON_GAP)*2,CON_W,CON_H);

                ((POBJECT_HEAD)(&item_button_next_con))->b_left_id = MAX_ID;
                ((POBJECT_HEAD)(&item_button_next_con))->b_up_id = NETWORK_ID;
                ((POBJECT_HEAD)(&item_button_next_con))->b_down_id \
                  = COUNTRY_ID;
                 //NETWORK_ID;modify bomur 2012 0213
                ((PTEXT_FIELD)(&item_button_next_label))->b_align \
                 = C_ALIGN_LEFT | C_ALIGN_VCENTER;
                ((POBJECT_HEAD)(&item_button_next_label))->style.b_show_idx = TXT_SH_IDX;

                /*OSD_SetRect(&(item_network_con.head.frame),\
                CON_L, CON_T,CON_W,CON_H);
                OSD_SetRect(&(item_network_label.head.frame),\
                CON_L, CON_T,TXTN_W,CON_H);
                OSD_SetRect(&(item_network_sel.head.frame),\
                CON_L+TXTS_L_OF, CON_T,TXTS_W,CON_H);

                ((POBJECT_HEAD)(&item_network_con))->bUpID = BUTTON_NEXT_ID;
                ((POBJECT_HEAD)(&item_network_con))->bDownID\
                 = BUTTON_NEXT_ID;*/
                 //modify bomur 2012 0213
                osd_set_rect(&(item_network_con.head.frame),\
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H);
                osd_set_rect(&(item_network_label.head.frame),\
                CON_L, CON_T + (CON_H + CON_GAP)*1,TXTN_W,CON_H);
                osd_set_rect(&(item_network_sel.head.frame),\
                CON_L+TXTS_L_OF, CON_T + (CON_H + CON_GAP)*1,\
                TXTS_W,CON_H);

                osd_set_rect(&(item_country_con.head.frame),\
                CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H);
                osd_set_rect(&(item_country_label.head.frame),\
                CON_L, CON_T + (CON_H + CON_GAP)*0,TXTN_W,CON_H);
                osd_set_rect(&(item_country_sel.head.frame),\
                CON_L+TXTS_L_OF, CON_T + (CON_H + CON_GAP)*0,TXTS_W,CON_H);

                ((POBJECT_HEAD)(&item_network_con))->b_up_id = COUNTRY_ID;
                ((POBJECT_HEAD)(&item_network_con))->b_down_id = BUTTON_NEXT_ID;

                osd_set_text_field_content(&item_button_next_label,STRING_ID,RS_COMMON_SEARCH);
                win_country_net_con.head.style.b_show_idx = WIN_SH_IDX;
                win_country_net_con.head.style.b_hlidx = WIN_HL_IDX;
                win_country_net_con.head.style.b_sel_idx = WIN_SL_IDX;
                win_country_net_con.head.style.b_gray_idx = WIN_GRY_IDX;

                osd_set_rect(&(win_country_net_con.head.frame), W_L,W_T,W_W,W_H);

            }
            else
            {
                wincom_open_title((POBJECT_HEAD)&win_country_net_con,RS_SYSTEM_WELCOME, 0);
                osd_set_attr(&item_country_con, C_ATTR_ACTIVE);
                osd_set_attr(&item_button_prev_con, C_ATTR_ACTIVE);
                osd_set_rect(&(item_button_next_con.head.frame),\
                CON_W-50, CON_T + (CON_H + CON_GAP)*7, BTN_W, BTN_H);
                osd_set_rect(&(item_button_next_label.head.frame),\
                CON_W-50, CON_T + (CON_H + CON_GAP)*7, BTN_W, BTN_H);

                ((POBJECT_HEAD)(&item_button_next_con))->b_left_id = BUTTON_PREV_ID;
                ((POBJECT_HEAD)(&item_button_next_con))->b_up_id = NETWORK_ID;
                ((POBJECT_HEAD)(&item_button_next_con))->b_down_id = COUNTRY_ID;
                ((PTEXT_FIELD)(&item_button_next_label))->b_align\
                  = C_ALIGN_CENTER | C_ALIGN_VCENTER;
                                ((POBJECT_HEAD)(&item_button_next_label))->style.b_show_idx \
                 = BTN_SH_IDX;
                #ifndef SD_UI
                osd_set_rect(&(item_network_con.head.frame),CON_L,\
                 CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H);
                osd_set_rect(&(item_network_label.head.frame),\
                CON_L, CON_T + (CON_H + CON_GAP)*1,\
                TXTN_W,CON_H);
                osd_set_rect(&(item_network_sel.head.frame),\
                CON_L+TXTS_L_OF, CON_T + (CON_H + CON_GAP)*1,\
                TXTS_W,CON_H);
                #else
                osd_set_rect(&(item_network_con.head.frame),\
                CON_L, CON_T + (CON_H + CON_GAP)*1,CON_W,CON_H);
                osd_set_rect(&(item_network_label.head.frame),\
                CON_L+TXTN_L_OF, CON_T + (CON_H + CON_GAP)*1,\
                TXTN_W,CON_H);
                osd_set_rect(&(item_network_sel.head.frame),\
                CON_L+TXTS_L_OF, CON_T + (CON_H + CON_GAP)*1,\
                TXTS_W,CON_H);
                #endif
                ((POBJECT_HEAD)(&item_network_con))->b_up_id = COUNTRY_ID;
                ((POBJECT_HEAD)(&item_network_con))->b_down_id = BUTTON_NEXT_ID;

                win_country_net_con.head.style.b_show_idx = WSTL_WIN_BODYLEFT_01_HD;
                win_country_net_con.head.style.b_hlidx = WSTL_WIN_BODYLEFT_01_HD;
                win_country_net_con.head.style.b_sel_idx = WSTL_WIN_BODYLEFT_01_HD;
                win_country_net_con.head.style.b_gray_idx = WSTL_WIN_BODYLEFT_01_HD;
                osd_set_rect(&(win_country_net_con.head.frame),\
                 MAIN_W_L, W_T, MAIN_W_W, W_H);

            }
            load_settings();
            break;
        case EVN_POST_OPEN:
            // win_help_guide_open_ex(pObj,com_guide,2);
             break;
        case EVN_PRE_CLOSE:
            sys_data_save(1);
            /* Make OSD not flickering */
            *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
            from_auto_scan = FALSE;
            break;
        case EVN_POST_CLOSE:
            break;
        case EVN_MSG_GOT:
            break;
        default:
                break;
    }
    return ret;
}

//#endif
