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
#include <api/libdiseqc/lib_diseqc.h>
#include <hld/decv/decv.h>

#include <api/libosd/osd_lib.h>
#include <api/libtsi/psi_db.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "ctrl_util.h"
#include "control.h"
#include "key.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_signal.h"
#include "win_diag_energy.h"
#include "win_search.h"
#ifdef MULTIFEED_SUPPORT
#include <api/libsi/lib_multifeed.h>
#endif
#ifdef SUPPORT_CAS_A
#include "win_com_menu_define.h"
#endif

#define DEB_PRINT    soc_printf
/*******************************************************************************
*    Objects definition
*******************************************************************************/

extern CONTAINER g_win_tplist;

extern TEXT_FIELD    tplst_sat;

extern TEXT_FIELD     tplst_split1;
extern TEXT_FIELD     tplst_split2;

extern OBJLIST     tplst_olist;

extern CONTAINER tplst_item0;
extern CONTAINER tplst_item1;
extern CONTAINER tplst_item2;
extern CONTAINER tplst_item3;
extern CONTAINER tplst_item4;
extern CONTAINER tplst_item5;
extern CONTAINER tplst_item6;
extern CONTAINER tplst_item7;
extern CONTAINER tplst_item8;
extern CONTAINER tplst_item9;
extern CONTAINER tplst_item10;


extern TEXT_FIELD tplst_idx0;
extern TEXT_FIELD tplst_idx1;
extern TEXT_FIELD tplst_idx2;
extern TEXT_FIELD tplst_idx3;
extern TEXT_FIELD tplst_idx4;
extern TEXT_FIELD tplst_idx5;
extern TEXT_FIELD tplst_idx6;
extern TEXT_FIELD tplst_idx7;
extern TEXT_FIELD tplst_idx8;
extern TEXT_FIELD tplst_idx9;

extern TEXT_FIELD tplst_fre0;
extern TEXT_FIELD tplst_fre1;
extern TEXT_FIELD tplst_fre2;
extern TEXT_FIELD tplst_fre3;
extern TEXT_FIELD tplst_fre4;
extern TEXT_FIELD tplst_fre5;
extern TEXT_FIELD tplst_fre6;
extern TEXT_FIELD tplst_fre7;
extern TEXT_FIELD tplst_fre8;
extern TEXT_FIELD tplst_fre9;

extern TEXT_FIELD tplst_sym0;
extern TEXT_FIELD tplst_sym1;
extern TEXT_FIELD tplst_sym2;
extern TEXT_FIELD tplst_sym3;
extern TEXT_FIELD tplst_sym4;
extern TEXT_FIELD tplst_sym5;
extern TEXT_FIELD tplst_sym6;
extern TEXT_FIELD tplst_sym7;
extern TEXT_FIELD tplst_sym8;
extern TEXT_FIELD tplst_sym9;

extern TEXT_FIELD tplst_pol0;
extern TEXT_FIELD tplst_pol1;
extern TEXT_FIELD tplst_pol2;
extern TEXT_FIELD tplst_pol3;
extern TEXT_FIELD tplst_pol4;
extern TEXT_FIELD tplst_pol5;
extern TEXT_FIELD tplst_pol6;
extern TEXT_FIELD tplst_pol7;
extern TEXT_FIELD tplst_pol8;
extern TEXT_FIELD tplst_pol9;

extern TEXT_FIELD tplst_isid0;
extern TEXT_FIELD tplst_isid1;
extern TEXT_FIELD tplst_isid2;
extern TEXT_FIELD tplst_isid3;
extern TEXT_FIELD tplst_isid4;
extern TEXT_FIELD tplst_isid5;
extern TEXT_FIELD tplst_isid6;
extern TEXT_FIELD tplst_isid7;
extern TEXT_FIELD tplst_isid8;
extern TEXT_FIELD tplst_isid9;

extern TEXT_FIELD tplst_line0;
extern TEXT_FIELD tplst_line1;
extern TEXT_FIELD tplst_line2;
extern TEXT_FIELD tplst_line3;
extern TEXT_FIELD tplst_line4;
extern TEXT_FIELD tplst_line5;
extern TEXT_FIELD tplst_line6;
extern TEXT_FIELD tplst_line7;
extern TEXT_FIELD tplst_line_sat;

#ifdef PLSN_SUPPORT
extern TEXT_FIELD tplst_pls0;
extern TEXT_FIELD tplst_pls1;
extern TEXT_FIELD tplst_pls2;
extern TEXT_FIELD tplst_pls3;
extern TEXT_FIELD tplst_pls4;
extern TEXT_FIELD tplst_pls5;
extern TEXT_FIELD tplst_pls6;
extern TEXT_FIELD tplst_pls7;
extern TEXT_FIELD tplst_pls8;
extern TEXT_FIELD tplst_pls9;
#endif

extern SCROLL_BAR    tplst_scb;

static PRESULT tplst_list_sat_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tplst_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tplst_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tplst_list_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tplst_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION tplst_keymap(POBJECT_HEAD p_obj, UINT32 key);
static PRESULT tplst_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2);

BOOL win_tplist_set_search_pid_callback(P_NODE* node);

#define WIN_SH_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_HL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_SL_IDX      WSTL_WIN_BODYRIGHT_01_HD
#define WIN_GRY_IDX  WSTL_WIN_BODYRIGHT_01_HD

#define LST_SAT_SH_IDX     WSTL_TEXT_11_HD

#define SPLIT_LINE_SH    WSTL_LINE_MENU_HD

#define LST_SH_IDX        WSTL_NOSHOW_IDX
#define LST_HL_IDX        WSTL_NOSHOW_IDX
#define LST_SL_IDX        WSTL_NOSHOW_IDX
#define LST_GRY_IDX         WSTL_NOSHOW_IDX

#define CON_SH_IDX   WSTL_BUTTON_01_HD
#define CON_HL_IDX   WSTL_BUTTON_05_HD
#define CON_SL_IDX   WSTL_BUTTON_01_HD
#define CON_GRY_IDX  WSTL_BUTTON_01_HD

#define TXTI_SH_IDX   WSTL_TEXT_09_HD
#define TXTI_HL_IDX   WSTL_BUTTON_02_FG_HD//WSTL_MIXBACK_BLACK_IDX_HD
#define TXTI_SL_IDX   WSTL_TEXT_09_HD
#define TXTI_GRY_IDX  WSTL_TEXT_09_HD

#define TXTC_SH_IDX   WSTL_TEXT_09_HD
#define TXTC_HL_IDX   WSTL_BUTTON_02_FG_HD
#define TXTC_SL_IDX   WSTL_TEXT_09_HD
#define TXTC_GRY_IDX  WSTL_TEXT_09_HD


#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT//sharon WSTL_SCROLLBAR_01_HD

#define LIST_BAR_MID_RECT_IDX      WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT//sharon WSTL_SCROLLBAR_02_HD

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define    W_L         248
#define    W_T         98
#define    W_W         692
#define    W_H         488
#endif

#define LST_SAT_L    (W_L + 20)
#define LST_SAT_T    (W_T + 4)
#define LST_SAT_W    (W_W - 70)
#define LST_SAT_H      36//40

#define SPLIT_LINE1_L    (W_L + 20)
#define SPLIT_LINE1_T      (LST_SAT_T + LST_SAT_H + 14)
#define SPLIT_LINE1_W    LST_SAT_W
#define SPLIT_LINE1_H    2

#define SCB_L         (W_L + 20)
#define SCB_T         (W_T + 70)//110//150
#define SCB_W         12//18
#define SCB_H         282//330

#define LST_L        (SCB_L + SCB_W + 10)
#define LST_T        (LST_SAT_T + LST_SAT_H + 30)
#define LST_W        (W_W - SCB_W - 60)
#define LST_H        340
#define LST_GAP     8

#define ITEM_L        (LST_L + 4)
#define ITEM_T        (LST_T + 2)
#define ITEM_W        (LST_W - 4)
#define ITEM_H        40
#define ITEM_GAP    8

#ifdef PLSN_SUPPORT
#ifdef MULTISTREAM_SUPPORT
#define ITEM_IDX_L  20
#define ITEM_IDX_W  46
#define ITEM_FRE_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_FRE_W  130
#define ITEM_SYM_L  (ITEM_FRE_L + ITEM_FRE_W)
#define ITEM_SYM_W  160
#define ITEM_POL_L  (ITEM_SYM_L + ITEM_SYM_W)
#define ITEM_POL_W  40
#define ITEM_ISID_L  (ITEM_POL_L + ITEM_POL_W)
#define ITEM_ISID_W  80

#define ITEM_PLS_L	(ITEM_ISID_L + ITEM_ISID_W)
#define ITEM_PLS_W	160
#else
#define ITEM_IDX_L  20
#define ITEM_IDX_W  54
#define ITEM_FRE_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_FRE_W  150
#define ITEM_SYM_L  (ITEM_FRE_L + ITEM_FRE_W)
#define ITEM_SYM_W  170
#define ITEM_POL_L  (ITEM_SYM_L + ITEM_SYM_W)
#define ITEM_POL_W  70

#define ITEM_PLS_L	(ITEM_POL_L + ITEM_POL_W)
#define ITEM_PLS_W	150

#endif
#else
#define ITEM_IDX_L  20
#define ITEM_IDX_W  60
#define ITEM_FRE_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_FRE_W  160
#define ITEM_SYM_L  (ITEM_FRE_L + ITEM_FRE_W)
#define ITEM_SYM_W  180
#define ITEM_POL_L  (ITEM_SYM_L + ITEM_SYM_W)
#define ITEM_POL_W  80
#define ITEM_ISID_L  (ITEM_POL_L + ITEM_POL_W)
#define ITEM_ISID_W  80
#endif
#define LIST_ITEMCNT 6

#define LINE_L_OF    0
#define LINE_T_OF      (ITEM_H+2)
#define LINE_W        ITEM_W
#define LINE_H         4

#define SPLIT_LINE2_L    SPLIT_LINE1_L
#define SPLIT_LINE2_T      (ITEM_T + LIST_ITEMCNT * (ITEM_H + ITEM_GAP) + 14)
#define SPLIT_LINE2_W    SPLIT_LINE1_W
#define SPLIT_LINE2_H    2
#else
#define    W_L         105//384
#define    W_T         57//138
#define    W_W         482
#define    W_H         370

#define LST_SAT_L    (W_L + 20)
#define LST_SAT_T    (W_T + 8)
#define LST_SAT_W    (W_W - 70)
#define LST_SAT_H      24//40

#define SPLIT_LINE1_L    (W_L + 20)
#define SPLIT_LINE1_T      (LST_SAT_T + LST_SAT_H + 14)
#define SPLIT_LINE1_W    LST_SAT_W
#define SPLIT_LINE1_H    2

#define SCB_L         (W_L + 20)
#define SCB_T         (W_T + 65)//110//150
#define SCB_W         12//18
#define SCB_H         170//330

#define LST_L        (SCB_L + SCB_W + 10)
#define LST_T        (LST_SAT_T + LST_SAT_H + 25)
#define LST_W        (W_W - SCB_W - 80)
#define LST_H        250
#define LST_GAP     8

#define ITEM_L        (LST_L + 4)
#define ITEM_T        (LST_T + 2)
#define ITEM_W        (LST_W - 4)
#define ITEM_H        24
#define ITEM_GAP    6

#ifdef PLSN_SUPPORT
#ifdef MULTISTREAM_SUPPORT
#define ITEM_IDX_L  20
#define ITEM_IDX_W  40
#define ITEM_FRE_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_FRE_W  90
#define ITEM_SYM_L  (ITEM_FRE_L + ITEM_FRE_W)
#define ITEM_SYM_W  90
#define ITEM_POL_L  (ITEM_SYM_L + ITEM_SYM_W)
#define ITEM_POL_W  36

#define ITEM_ISID_L  (ITEM_POL_L + ITEM_POL_W)
#define ITEM_ISID_W  40

#define ITEM_PLS_L	(ITEM_ISID_L + ITEM_ISID_W)
#define ITEM_PLS_W	80
#else
#define ITEM_IDX_L  20
#define ITEM_IDX_W  50
#define ITEM_FRE_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_FRE_W  100
#define ITEM_SYM_L  (ITEM_FRE_L + ITEM_FRE_W)
#define ITEM_SYM_W  100
#define ITEM_POL_L  (ITEM_SYM_L + ITEM_SYM_W)
#define ITEM_POL_W  36

#define ITEM_PLS_L	(ITEM_POL_L + ITEM_POL_W)
#define ITEM_PLS_W	80
#endif
#else
#define ITEM_IDX_L  20
#define ITEM_IDX_W  60
#define ITEM_FRE_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_FRE_W  100
#define ITEM_SYM_L  (ITEM_FRE_L + ITEM_FRE_W)
#define ITEM_SYM_W  100
#define ITEM_POL_L  (ITEM_SYM_L + ITEM_SYM_W)
#define ITEM_POL_W  40
#define ITEM_ISID_L  (ITEM_POL_L + ITEM_POL_W)
#define ITEM_ISID_W  40
#endif

#define LIST_ITEMCNT 6

#define LINE_L_OF    0
#define LINE_T_OF      (ITEM_H+2)
#define LINE_W        ITEM_W
#define LINE_H         4

#define SPLIT_LINE2_L    SPLIT_LINE1_L
#define SPLIT_LINE2_T      (ITEM_T + LIST_ITEMCNT * (ITEM_H + ITEM_GAP) + 14)
#define SPLIT_LINE2_W    SPLIT_LINE1_W
#define SPLIT_LINE2_H    2

#endif

#define LDEF_TXTSAT(root,var_txt,nxt_obj,l,t,w,h,str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, LST_SAT_SH_IDX,LST_SAT_SH_IDX,LST_SAT_SH_IDX,LST_SAT_SH_IDX,   \
    NULL,tplst_list_sat_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)\

#define LDEF_TPLST_LINE(root, var_txt, nxt_obj, l, t, w, h, str)        \
    DEF_TEXTFIELD(var_txt,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, l, t, w, h, SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,SPLIT_LINE_SH,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    tplst_list_item_con_keymap,tplst_list_item_con_callback,  \
    conobj, 1,1)

#define LDEF_TXTIDX(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_TXTOTHER(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, TXTC_SH_IDX,TXTC_HL_IDX,TXTC_SL_IDX,TXTC_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 10,0,res_id,str)

#ifdef PLSN_SUPPORT
#ifdef MULTISTREAM_SUPPORT
#define LDEF_LIST_ITEM(root,varCon,varIdx,varFre,varSym,varPol,varIsid,varPls,varLine,ID,l,t,w,h,idxstr,freqstr,symbstr,isidstr,plsstr)	\
	LDEF_CON(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varIdx,1)	\
	LDEF_TXTIDX(&varCon,varIdx,&varFre ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)	\
	LDEF_TXTOTHER(&varCon,varFre,&varSym,1,1,1,1,1,l + ITEM_FRE_L, t,ITEM_FRE_W,h,0,freqstr)	\
	LDEF_TXTOTHER(&varCon,varSym, &varPol,0,0,0,0,0,l + ITEM_SYM_L, t,ITEM_SYM_W,h,0,symbstr) \
	LDEF_TXTOTHER(&varCon,varPol, &varIsid,0,0,0,0,0,l + ITEM_POL_L, t,ITEM_POL_W,h,0,0) \
	LDEF_TXTOTHER(&varCon,varIsid, &varPls/*&varLine*/,0,0,0,0,0,l + ITEM_ISID_L, t,ITEM_ISID_W,h,0,isidstr) \
	LDEF_TXTOTHER(&varCon,varPls, NULL,0,0,0,0,0,l + ITEM_PLS_L, t,ITEM_PLS_W,h,0,plsstr) \
	LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,ITEM_L + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)
#else
#define LDEF_LIST_ITEM(root,varCon,varIdx,varFre,varSym,varPol,varPls,varLine,ID,l,t,w,h,idxstr,freqstr,symbstr,plsstr)	\
	LDEF_CON(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varIdx,1)	\
	LDEF_TXTIDX(&varCon,varIdx,&varFre ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)	\
	LDEF_TXTOTHER(&varCon,varFre,&varSym,1,1,1,1,1,l + ITEM_FRE_L, t,ITEM_FRE_W,h,0,freqstr)	\
	LDEF_TXTOTHER(&varCon,varSym, &varPol,0,0,0,0,0,l + ITEM_SYM_L, t,ITEM_SYM_W,h,0,symbstr) \
	LDEF_TXTOTHER(&varCon,varPol, &varPls/*&varLine*/,0,0,0,0,0,l + ITEM_POL_L, t,ITEM_POL_W,h,0,0)\
	LDEF_TXTOTHER(&varCon,varPls, NULL,0,0,0,0,0,l + ITEM_PLS_L, t,ITEM_PLS_W,h,0,plsstr) \
	LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,ITEM_L + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)
#endif
#else
#ifdef MULTISTREAM_SUPPORT
#define LDEF_LIST_ITEM(root,varCon,varIdx,varFre,varSym,varPol,varIsid,varLine,ID,l,t,w,h,idxstr,freqstr,symbstr,isidstr)	\
	LDEF_CON(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varIdx,1)	\
	LDEF_TXTIDX(&varCon,varIdx,&varFre ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)	\
	LDEF_TXTOTHER(&varCon,varFre,&varSym,1,1,1,1,1,l + ITEM_FRE_L, t,ITEM_FRE_W,h,0,freqstr)	\
	LDEF_TXTOTHER(&varCon,varSym, &varPol,0,0,0,0,0,l + ITEM_SYM_L, t,ITEM_SYM_W,h,0,symbstr) \
	LDEF_TXTOTHER(&varCon,varPol, &varIsid,0,0,0,0,0,l + ITEM_POL_L, t,ITEM_POL_W,h,0,0)\
	LDEF_TXTOTHER(&varCon,varIsid, NULL/*&varLine*/,0,0,0,0,0,l + ITEM_ISID_L, t,ITEM_ISID_W,h,0,isidstr)\
	LDEF_LINE(&varCon,varLine,NULL,1,1,1,1,1,ITEM_L + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)

#else
#define LDEF_LIST_ITEM(root,var_con,var_idx,var_fre,var_sym,var_pol,var_line,ID,l,t,w,h,idxstr,freqstr,symbstr)    \
    LDEF_CON(&root,var_con,NULL,ID,ID,ID,ID,ID,l,t,w,h,&var_idx,1)    \
    LDEF_TXTIDX(&var_con,var_idx,&var_fre ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)    \
    LDEF_TXTOTHER(&var_con,var_fre,&var_sym,1,1,1,1,1,l + ITEM_FRE_L, t,ITEM_FRE_W,h,0,freqstr)    \
    LDEF_TXTOTHER(&var_con,var_sym, &var_pol,0,0,0,0,0,l + ITEM_SYM_L, t,ITEM_SYM_W,h,0,symbstr) \
    LDEF_TXTOTHER(&var_con,var_pol, NULL/*&varLine*/,0,0,0,0,0,l + ITEM_POL_L, t,ITEM_POL_W,h,0,0)\
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,ITEM_L + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)
#endif
#endif

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)    \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 20, w, h - 40, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)    \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    tplst_list_keymap,tplst_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(var_con,nxt_obj,l,t,w,h,focus_id)        \
    DEF_CONTAINER(var_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    tplst_keymap,tplst_callback,  \
    nxt_obj, focus_id,0)


LDEF_TXTSAT(g_win_tplist,tplst_sat,&tplst_split1,\
    LST_SAT_L, LST_SAT_T, LST_SAT_W, LST_SAT_H,len_display_str)

LDEF_TPLST_LINE(g_win_tplist, tplst_split1, &tplst_split2, SPLIT_LINE1_L, SPLIT_LINE1_T, SPLIT_LINE1_W, SPLIT_LINE1_H, NULL)
LDEF_TPLST_LINE(g_win_tplist, tplst_split2, &tplst_olist, SPLIT_LINE2_L, SPLIT_LINE2_T, SPLIT_LINE2_W, SPLIT_LINE2_H, NULL)

#ifdef PLSN_SUPPORT
#ifdef MULTISTREAM_SUPPORT
LDEF_LIST_ITEM(tplst_olist,tplst_item0,tplst_idx0,tplst_fre0,tplst_sym0, tplst_pol0,tplst_isid0, tplst_pls0, tplst_line0, 1,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*0,ITEM_W, ITEM_H, display_strs[0], display_strs[10],display_strs[20],display_strs[30],display_strs[40])

LDEF_LIST_ITEM(tplst_olist,tplst_item1,tplst_idx1,tplst_fre1,tplst_sym1, tplst_pol1,tplst_isid1, tplst_pls1, tplst_line1, 2,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[1], display_strs[11],display_strs[21],display_strs[31],display_strs[41])

LDEF_LIST_ITEM(tplst_olist,tplst_item2,tplst_idx2,tplst_fre2,tplst_sym2, tplst_pol2,tplst_isid2,tplst_pls2, tplst_line2, 3,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[2], display_strs[12],display_strs[22],display_strs[32],display_strs[42])	

LDEF_LIST_ITEM(tplst_olist,tplst_item3,tplst_idx3,tplst_fre3,tplst_sym3, tplst_pol3,tplst_isid3,tplst_pls3, tplst_line3, 4,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[3], display_strs[13],display_strs[23],display_strs[33],display_strs[43])

LDEF_LIST_ITEM(tplst_olist,tplst_item4,tplst_idx4,tplst_fre4,tplst_sym4, tplst_pol4,tplst_isid4,tplst_pls4, tplst_line4, 5,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[4], display_strs[14],display_strs[24],display_strs[34],display_strs[44])

LDEF_LIST_ITEM(tplst_olist,tplst_item5,tplst_idx5,tplst_fre5,tplst_sym5, tplst_pol5,tplst_isid5,tplst_pls5, tplst_line5, 6,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[5], display_strs[15],display_strs[25],display_strs[35],display_strs[45])
#else
LDEF_LIST_ITEM(tplst_olist,tplst_item0,tplst_idx0,tplst_fre0,tplst_sym0, tplst_pol0, tplst_pls0, tplst_line0, 1,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*0,ITEM_W, ITEM_H, display_strs[0], display_strs[10],display_strs[20],display_strs[30])

LDEF_LIST_ITEM(tplst_olist,tplst_item1,tplst_idx1,tplst_fre1,tplst_sym1, tplst_pol1, tplst_pls1, tplst_line1, 2,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[1], display_strs[11],display_strs[21],display_strs[31])

LDEF_LIST_ITEM(tplst_olist,tplst_item2,tplst_idx2,tplst_fre2,tplst_sym2, tplst_pol2, tplst_pls2, tplst_line2, 3,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[2], display_strs[12],display_strs[22],display_strs[32])	

LDEF_LIST_ITEM(tplst_olist,tplst_item3,tplst_idx3,tplst_fre3,tplst_sym3, tplst_pol3, tplst_pls3, tplst_line3, 4,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[3], display_strs[13],display_strs[23],display_strs[33])

LDEF_LIST_ITEM(tplst_olist,tplst_item4,tplst_idx4,tplst_fre4,tplst_sym4, tplst_pol4, tplst_pls4, tplst_line4, 5,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[4], display_strs[14],display_strs[24],display_strs[34])

LDEF_LIST_ITEM(tplst_olist,tplst_item5,tplst_idx5,tplst_fre5,tplst_sym5, tplst_pol5, tplst_pls5, tplst_line5, 6,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[5], display_strs[15],display_strs[25],display_strs[35])
#endif

#else

#ifdef MULTISTREAM_SUPPORT
LDEF_LIST_ITEM(tplst_olist,tplst_item0,tplst_idx0,tplst_fre0,tplst_sym0, tplst_pol0,tplst_isid0,tplst_line0, 1,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*0,ITEM_W, ITEM_H, display_strs[0], display_strs[10],display_strs[20],display_strs[30])

LDEF_LIST_ITEM(tplst_olist,tplst_item1,tplst_idx1,tplst_fre1,tplst_sym1, tplst_pol1,tplst_isid1,tplst_line1, 2,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[1], display_strs[11],display_strs[21],display_strs[31])

LDEF_LIST_ITEM(tplst_olist,tplst_item2,tplst_idx2,tplst_fre2,tplst_sym2, tplst_pol2,tplst_isid2,tplst_line2, 3,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[2], display_strs[12],display_strs[22],display_strs[32])	

LDEF_LIST_ITEM(tplst_olist,tplst_item3,tplst_idx3,tplst_fre3,tplst_sym3, tplst_pol3,tplst_isid3,tplst_line3, 4,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[3], display_strs[13],display_strs[23],display_strs[33])

LDEF_LIST_ITEM(tplst_olist,tplst_item4,tplst_idx4,tplst_fre4,tplst_sym4, tplst_pol4,tplst_isid4,tplst_line4, 5,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[4], display_strs[14],display_strs[24],display_strs[34])

LDEF_LIST_ITEM(tplst_olist,tplst_item5,tplst_idx5,tplst_fre5,tplst_sym5, tplst_pol5,tplst_isid5,tplst_line5, 6,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[5], display_strs[15],display_strs[25],display_strs[35])


#else
LDEF_LIST_ITEM(tplst_olist,tplst_item0,tplst_idx0,tplst_fre0,tplst_sym0, tplst_pol0,tplst_line0, 1,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*0,ITEM_W, ITEM_H, display_strs[0], display_strs[10],display_strs[20])

LDEF_LIST_ITEM(tplst_olist,tplst_item1,tplst_idx1,tplst_fre1,tplst_sym1, tplst_pol1,tplst_line1, 2,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[1], display_strs[11],display_strs[21])

LDEF_LIST_ITEM(tplst_olist,tplst_item2,tplst_idx2,tplst_fre2,tplst_sym2, tplst_pol2,tplst_line2, 3,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[2], display_strs[12],display_strs[22])

LDEF_LIST_ITEM(tplst_olist,tplst_item3,tplst_idx3,tplst_fre3,tplst_sym3, tplst_pol3,tplst_line3, 4,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[3], display_strs[13],display_strs[23])

LDEF_LIST_ITEM(tplst_olist,tplst_item4,tplst_idx4,tplst_fre4,tplst_sym4, tplst_pol4,tplst_line4, 5,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[4], display_strs[14],display_strs[24])

LDEF_LIST_ITEM(tplst_olist,tplst_item5,tplst_idx5,tplst_fre5,tplst_sym5, tplst_pol5,tplst_line5, 6,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[5], display_strs[15],display_strs[25])
#endif

#endif

#if 0
LDEF_LIST_ITEM(tplst_olist,tplst_item6,tplst_idx6,tplst_fre6,tplst_sym6, tplst_pol6,tplst_line6, 7,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*6,ITEM_W, ITEM_H, display_strs[6], display_strs[16],display_strs[26])

LDEF_LIST_ITEM(tplst_olist,tplst_item7,tplst_idx7,tplst_fre7,tplst_sym7, tplst_pol7, 8,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*7,ITEM_W, ITEM_H, display_strs[7], display_strs[17],display_strs[27])

LDEF_LIST_ITEM(tplst_olist,tplst_item8,tplst_idx8,tplst_fre8,tplst_sym8, tplst_pol8, 9,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*8,ITEM_W, ITEM_H, display_strs[8], display_strs[18],display_strs[28])

LDEF_LIST_ITEM(tplst_olist,tplst_item9,tplst_idx9,tplst_fre9,tplst_sym9, tplst_pol9, 10,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*9,ITEM_W, ITEM_H, display_strs[9], display_strs[19],display_strs[29])
#endif

LDEF_LISTBAR(tplst_olist,tplst_scb,LIST_ITEMCNT,SCB_L,SCB_T, SCB_W, SCB_H)


POBJECT_HEAD tplist_items[] =
{
    (POBJECT_HEAD)&tplst_item0,
    (POBJECT_HEAD)&tplst_item1,
    (POBJECT_HEAD)&tplst_item2,
    (POBJECT_HEAD)&tplst_item3,
    (POBJECT_HEAD)&tplst_item4,
    (POBJECT_HEAD)&tplst_item5,
#if 0
    (POBJECT_HEAD)&tplst_item6,
    (POBJECT_HEAD)&tplst_item7,
    (POBJECT_HEAD)&tplst_item8,
    (POBJECT_HEAD)&tplst_item9,
#endif    
};

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_tplist,tplst_olist,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, LIST_ITEMCNT, 0,tplist_items,&tplst_scb,NULL,NULL)


LDEF_WIN(g_win_tplist,&tplst_sat, W_L,W_T,W_W, W_H, 1)


/*******************************************************************************
*    Local vriable & function declare
*******************************************************************************/

#define TPTURN_TIMER_NAME "tptn"
#define TPTURN_TIMER_TIME    600

static ID tpturn_timer_id = OSAL_INVALID_ID;


static UINT8 wait_signalmsg_time = 0;
static UINT16    cur_tp_sat_idx = 0;
static UINT16   pre_tp_sat_idx =0;
static UINT16    tp_list_pos = 0xffff;
static UINT16   tp_list_top = 0;
static UINT16   cur_tp_idx = 0;

UINT32 tp_vpid=0,tp_apid=0,tp_ppid=0;
struct help_item_resource  tplist_helpinfo[] =
{
    {IM_EPG_COLORBUTTON_RED,       RS_HELP_EDIT},
    {IM_EPG_COLORBUTTON_GREEN,     RS_HELP_ADD},
    {IM_EPG_COLORBUTTON_YELLOW,    RS_HELP_DELETE},
    {IM_EPG_COLORBUTTON_BLUE,      RS_HELP_SEARCH},
};

#define HELP_CNT    (sizeof(tplist_helpinfo)/sizeof(struct help_item_resource))

#define VACT_SAT_PRE        (VACT_PASS + 1)
#define VACT_SAT_NEXT        (VACT_PASS + 2)
#define VACT_SAT_SELECT    (VACT_PASS + 3)
#define VACT_TP_EDIT        (VACT_PASS + 4)
#define VACT_TP_ADD        (VACT_PASS + 5)
#define VACT_TP_DEL        (VACT_PASS + 6)
#define VACT_TP_SRCH        (VACT_PASS + 7)
#define VACT_TP_CAPTURE (VACT_PASS+8)    //press mute two times, start capture
//extern UINT8 delay_signal_detect;
static BOOL tpscan_return  = FALSE;
BOOL set_movedish_flag(BOOL flag);
static PRESULT win_tplist_list_unkown_act_proc(VACTION act);
static PRESULT win_tplist_message_proc(UINT32 msg_type, UINT32 msg_code);

static void win_tplist_set_sat_display(void);
static void win_tplist_set_display(void);
static void win_tplist_load_sat(BOOL updatesat);
static void win_tplist_tpturn_handler(UINT32 nouse);
static void win_tplist_tuner_frontend(void);
static UINT32 win_tplist_check_change(void);
static void win_tpsrch_set_search_param(UINT32 fta_only,UINT32 srch_chan,UINT32 nit_flag);
extern UINT32 win_tpe_open(UINT32 mode,UINT32 sat_id,UINT32 tp_id);
#ifdef PLSN_SUPPORT
extern void win_tpe_get_setting(UINT32* freq, UINT32* symb, UINT32* pol, UINT32* pls);
#else
extern void win_tpe_get_setting(UINT32* freq, UINT32* symb, UINT32* pol);
#endif
extern UINT32 win_tpsrch_open(UINT32* ftaonly_vpid, UINT32* srch_chan_apid,UINT32* nit_ppid);
#ifdef DVBS_SUPPORT
extern void sat2antenna(S_NODE *sat, struct ft_antenna *antenna);
#endif
/*******************************************************************************
*    key mapping and event callback definition
*******************************************************************************/

static PRESULT tplst_list_sat_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;


    switch(event)
    {
    case EVN_PRE_DRAW:
        win_tplist_set_sat_display();
        break;
    default:
        break;
    }

    return ret;
}

static VACTION tplst_list_item_con_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act = VACT_PASS;

    return act;
}

static PRESULT tplst_list_item_con_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;

    return ret;
}


static VACTION tplst_list_keymap(POBJECT_HEAD p_obj, UINT32 key)
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
    case V_KEY_P_UP:
        act = VACT_CURSOR_PGUP;
        break;
    case V_KEY_P_DOWN:
        act = VACT_CURSOR_PGDN;
        break;

    case V_KEY_LEFT:
        act = VACT_SAT_PRE;
        break;
    case V_KEY_RIGHT:
        act = VACT_SAT_NEXT;
        break;
    #ifndef C3041
    case V_KEY_SAT:
        act = VACT_SAT_SELECT;
    #endif
        break;
    case V_KEY_RED:
        act = VACT_TP_EDIT;
        break;
    case V_KEY_GREEN:
        act = VACT_TP_ADD;
        break;
    case V_KEY_YELLOW:
        act = VACT_TP_DEL;
        break;
    case V_KEY_BLUE:
    case V_KEY_ENTER:
        act = VACT_TP_SRCH;
        break;
    case V_KEY_MUTE:
        act = VACT_TP_CAPTURE;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT tplst_list_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    VACTION unact = VACT_PASS;


    switch(event)
    {
    case EVN_PRE_DRAW:
        win_tplist_set_display();
        break;
    case EVN_POST_DRAW:
        break;
    case EVN_ITEM_PRE_CHANGE:
        api_stop_timer(&tpturn_timer_id);
        break;
    case EVN_ITEM_POST_CHANGE:
        tpturn_timer_id = api_start_timer(TPTURN_TIMER_NAME, TPTURN_TIMER_TIME, win_tplist_tpturn_handler);
        break;
    case EVN_PRE_CHANGE:
        break;
    case EVN_POST_CHANGE:
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_tplist_list_unkown_act_proc(unact);
        break;

    default:
        break;
    }


    return ret;
}



static VACTION tplst_keymap(POBJECT_HEAD p_obj, UINT32 key)
{
    VACTION act =VACT_PASS;

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

static PRESULT tplst_callback(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8 back_saved = 0;
    S_NODE s_node;
	static UINT32 tp_count=0;
	OBJLIST* ol=NULL;
#ifdef PLSN_SUPPORT
    SYSTEM_DATA *sys_data = NULL;
#endif

    MEMSET(&s_node, 0 ,sizeof(s_node));
    switch(event)
    {
    case EVN_PRE_OPEN:
        //if(sys_data_get_cur_satidx() == (UINT16)(~0))
            cur_tp_sat_idx = win_load_default_satidx();
        //else
        //    cur_tp_sat_idx = sys_data_get_cur_satidx();

        tp_list_pos = 0xffff;
        tp_list_top = 0;

        win_tplist_load_sat(FALSE);
           pre_tp_sat_idx= cur_tp_sat_idx;
        wincom_open_title(p_obj,RS_TP_LIST,0);
        break;
    case EVN_POST_OPEN:
        //win_signal_open(pObj);
        //win_signal_open_tplist(pObj);
        ol = &tplst_olist;
		tp_count = osd_get_obj_list_count(ol);
        win_signal_open_ex(p_obj,p_obj->frame.u_left,p_obj->frame.u_top + LIST_ITEMCNT*(ITEM_H + ITEM_GAP) + LST_SAT_H +50);

        wincom_open_help(p_obj,tplist_helpinfo , HELP_CNT);
#ifdef PLSN_SUPPORT
        sys_data = sys_data_get();
        if (1 == sys_data->super_scan)
        {
            api_stop_timer(&tpturn_timer_id);
            tpturn_timer_id = api_start_timer(TPTURN_TIMER_NAME, 4000, win_tplist_tpturn_handler);
        }
        else
        {
            win_tplist_tuner_frontend();
        }
#else
        win_tplist_tuner_frontend();
#endif
		if(0!=tp_count)
		{
        	win_signal_refresh();
		}
        //win_signal_update();
        get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx, &s_node);
        if(!tpscan_return)
            wincom_dish_move_popup_open(0xFFFF,s_node.sat_id,cur_tuner_idx,&back_saved);
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
#ifdef _INVW_JUICE
        if(win_tplist_check_change()){
            update_data();
            osal_delay_ms(500);
            inview_update_database();  //v0.1.4
        }

#else
        if(win_tplist_check_change())
            update_data();
#endif

        api_stop_timer(&tpturn_timer_id);
        win_signal_close();
        wincom_close_help();
        sys_data_check_channel_groups();
        sys_data_set_cur_satidx((UINT16)cur_tp_sat_idx);
        #ifdef MULTIFEED_SUPPORT
        multifeed_del_invalid_info();
        #endif
        break;
    case EVN_MSG_GOT:
		win_tplist_message_proc(param1,param2);
        break;
    default:
        break;
    }


    return ret;
}

static void win_tplist_set_capture(S_NODE *s_node, T_NODE *t_node)
{
    INT32 tuning_frq = 0;
    UINT8 pol = 0;
    struct nim_device *nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, (s_node->tuner1_valid ? 0 : (s_node->tuner2_valid ? 1 : 0)));
    SYSTEM_DATA* sys_data = sys_data_get();

    switch(s_node->lnb_type)
    {
        case LNB_CTRL_22K:
            if(t_node->frq > KU_LNB_22K_EDGE_FREQUENCY)
            {
                api_diseqc_set_22k(nim, 1);
                tuning_frq = t_node->frq - ((s_node->lnb_high > s_node->lnb_low) ? s_node->lnb_high : s_node->lnb_low);
            }
            else
            {
                api_diseqc_set_22k(nim, 0);
                tuning_frq = t_node->frq - ((s_node->lnb_high > s_node->lnb_low) ? s_node->lnb_low : s_node->lnb_high);
            }
        break;
        case LNB_CTRL_POL:
            pol = ((s_node->pol == 0) || (s_node->pol == 3)) ? (s_node->pol + 1) : (s_node->pol);
            if(pol == 1)
            {
                tuning_frq = s_node->lnb_low - t_node->frq;
            }
            else
            {
                tuning_frq = s_node->lnb_high - t_node->frq;
            }
            break;
        case LNB_CTRL_UNICABLE:
//            frontend_set_unicable(nim,antenna,xpond);
            tuning_frq = sys_data->tuner_lnb_antenna[cur_tuner_idx].unicable_freq;;
            break;
        case LNB_CTRL_STD:
        default:
            tuning_frq = (((s_node->lnb_low != 0) ? s_node->lnb_low : s_node->lnb_high)) - t_node->frq;
            break;
    }

    if(tuning_frq < 0)
    {
        tuning_frq = -tuning_frq;
    }

    UINT32  __MAYBE_UNUSED__ cap_param[3];
    cap_param[0] = tuning_frq;
    cap_param[1] = (UINT32)NULL;
    cap_param[2] = t_node->sym;
    libc_printf("win start capture freq = %d\n", tuning_frq);
    //nim_ioctl_ext(nim, NIM_DRIVER_START_CAPTURE, &cap_param);
    set_capture_freq(nim, tuning_frq);
}

static PRESULT win_tplist_list_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    UINT8    pre_sat_idx = 0;
    UINT16 sel = 0,sat_cnt =0;
    OSD_RECT rect;
    COM_POP_LIST_PARAM_T param;
    OBJLIST* ol = NULL;
    S_NODE s_node;
    T_NODE t_node;
    UINT32 freq = 0,symb,pol =0;
    UINT8 back_saved = 0;
    UINT32 ftaonly_vpid = 0,srch_chan_apid =0,nit_ppid =0;
    UINT32 choice =0;
    POBJECT_HEAD sub_menu =NULL;
    INT32 iret = -1;
    UINT16  __MAYBE_UNUSED__ pre_sat_id = 0,cur_sat_id =0;
    struct nim_device *nim=NULL;
#ifdef PLSN_SUPPORT    
    UINT32 plsn = 0;
#endif

    MEMSET(&rect, 0, sizeof(rect));
    MEMSET(&param, 0, sizeof(param));
    MEMSET(&s_node, 0, sizeof(s_node));
    MEMSET(&t_node, 0, sizeof(t_node));
    sat_cnt = get_tuner_sat_cnt(TUNER_EITHER);

    get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx, &s_node);
    ol = &tplst_olist;
    sel = osd_get_obj_list_cur_point(ol);

    pre_sat_idx = cur_tp_sat_idx;

    switch(act)
    {
    case VACT_TP_CAPTURE:
        {
            /* If not TP, can't capture */
            if(osd_get_obj_list_count(ol) == 0)
                break;
            get_tp_at(s_node.sat_id,sel, &t_node);
            win_tplist_set_capture(&s_node, &t_node);
            sub_menu = (POBJECT_HEAD)&g_con_diag_energy;
            wincom_close_help();
            if(osd_obj_open(sub_menu, 0xFFFFFFFF) != PROC_LEAVE)
            {
                menu_stack_push(sub_menu);
            }
        }
        break;
    case VACT_SAT_PRE:
        if(cur_tp_sat_idx > 0)
            cur_tp_sat_idx --;
        else
            cur_tp_sat_idx = sat_cnt - 1;
        goto SAT_CHANGE;

    case VACT_SAT_NEXT:
        if(cur_tp_sat_idx < (sat_cnt - 1))
            cur_tp_sat_idx ++;
        else
            cur_tp_sat_idx = 0;
        goto SAT_CHANGE;

    case VACT_SAT_SELECT:
        param.id = TUNER_EITHER;
        param.cur = cur_tp_sat_idx;
        param.selecttype = POP_LIST_SINGLESELECT;
        osd_set_rect(&rect, LST_SAT_L - 20 , LST_SAT_T, LST_SAT_W + 40, 360);
        rect.u_top += LST_SAT_H;
        sel = win_com_open_sub_list(POP_LIST_TYPE_SAT,&rect,&param);
        if(sel < sat_cnt)
            cur_tp_sat_idx = sel;
        goto SAT_CHANGE;
SAT_CHANGE:
        tp_list_pos = 0;
        tp_list_top = 0;
        if(cur_tp_sat_idx != pre_sat_idx)
        {
            pre_tp_sat_idx = pre_sat_idx;
            get_tuner_sat(TUNER_EITHER, pre_sat_idx, &s_node);
            pre_sat_id = s_node.sat_id;

            get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx, &s_node);
            cur_sat_id = s_node.sat_id;

            #ifdef SUPPORT_TWO_TUNER
                cur_tuner_idx = s_node.tuner1_valid? 0 : 1;
            #endif

#ifdef _INVW_JUICE
            if(win_tplist_check_change()){
                update_data();
                osal_delay_ms(500);
                inview_update_database(); //v0.1.4
            }

#else
            if(win_tplist_check_change())
                update_data();
#endif
            win_tplist_load_sat(TRUE);
            //OSD_TrackObject( (POBJECT_HEAD)&tplst_olist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            osd_track_object( (POBJECT_HEAD)&g_win_tplist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            api_stop_timer(&tpturn_timer_id);
            tpturn_timer_id = api_start_timer(TPTURN_TIMER_NAME, TPTURN_TIMER_TIME, win_tplist_tpturn_handler);
        }
        break;
    case VACT_TP_EDIT:
        /* If not TP, can't EDIT */
        if(osd_get_obj_list_count(ol) == 0)
            break;

        get_tp_at(s_node.sat_id,sel, &t_node);
        choice = win_tpe_open(0, s_node.sat_id, t_node.tp_id);
        if( choice)
         {
        #ifdef PLSN_SUPPORT
            win_tpe_get_setting(&freq, &symb, &pol, &plsn);
        #else
            win_tpe_get_setting(&freq, &symb, &pol);
        #endif

        #ifdef PLSN_SUPPORT
            if((freq != t_node.frq) || (symb != t_node.sym) || (pol != t_node.pol) || (plsn != t_node.pls_num))
        #else
            if((freq != t_node.frq) || (symb != t_node.sym) || (pol != t_node.pol))
        #endif
            {
                t_node.frq = freq;
                t_node.sym = symb;
                t_node.pol = pol;
				#ifdef MULTISTREAM_SUPPORT
				t_node.t2_profile = 0;
				#endif
            #ifdef PLSN_SUPPORT
                t_node.pls_num = plsn;
            #endif
                modify_tp(t_node.tp_id,&t_node);
                api_stop_timer(&tpturn_timer_id);
                tpturn_timer_id = api_start_timer(TPTURN_TIMER_NAME, TPTURN_TIMER_TIME, win_tplist_tpturn_handler);

            }
        }

        //OSD_TrackObject( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        osd_track_object( (POBJECT_HEAD)&g_win_tplist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

        break;
    case VACT_TP_ADD:
        if(osd_get_obj_list_count(ol) == MAX_TP_NUM)
            break;
        extern UINT32 edit_tp_exist;
        edit_tp_exist = 0;

        choice = win_tpe_open(1, s_node.sat_id, 0);
        if(choice)
        {
        #ifdef PLSN_SUPPORT
            win_tpe_get_setting(&freq, &symb, &pol, &plsn);
        #else
            win_tpe_get_setting(&freq, &symb, &pol);
        #endif
            MEMSET(&t_node,0,sizeof(t_node) );
            t_node.frq = freq;
            t_node.sym = symb;
            t_node.pol = pol;
            t_node.sat_id = s_node.sat_id;
        #ifdef MULTISTREAM_SUPPORT
            t_node.t2_profile = 0;  // not a multistream tp
            t_node.plp_id = 0xff;   // default invalid
        #endif
        #ifdef PLSN_SUPPORT
            t_node.pls_num = plsn;
        #endif
            iret = add_node(TYPE_TP_NODE,s_node.sat_id,(void*)&t_node);
            if((iret == DBERR_MAX_LIMIT) || (iret == DBERR_FLASH_FULL))
            {
                osd_track_object( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                win_compopup_init(WIN_POPUP_TYPE_OK);
                win_compopup_set_msg(NULL,NULL, RS_DISPLAY_TOO_MANY_TP);
                win_compopup_open_ext(&back_saved);
                break;
            }
            osd_set_obj_list_count(ol, osd_get_obj_list_count(ol) + 1);
            //OSD_TrackObject( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            osd_track_object( (POBJECT_HEAD)&g_win_tplist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            osd_change_focus((POBJECT_HEAD)ol,osd_get_obj_list_count(ol) - 1, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);

            api_stop_timer(&tpturn_timer_id);
            tpturn_timer_id = api_start_timer(TPTURN_TIMER_NAME, TPTURN_TIMER_TIME, win_tplist_tpturn_handler);

        }
        else
        {
            osd_track_object( (POBJECT_HEAD)&g_win_tplist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            //OSD_TrackObject( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
             if( edit_tp_exist > 0)
             {
                 osd_change_focus((POBJECT_HEAD)ol,edit_tp_exist - 1, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
                 edit_tp_exist = 0;
             }
             //OSD_TrackObject( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
        break;
    case VACT_TP_DEL:
        /* If not TP, can't DEL */
        if(osd_get_obj_list_count(ol) == 0)
            break;

        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg(NULL, NULL,RS_DISPLAY_SURE_TO_DELETE);
        if(win_compopup_open_ext(&back_saved) != WIN_POP_CHOICE_YES)
            break;

        get_tp_at(s_node.sat_id,sel, &t_node);
        del_tp_by_id(t_node.tp_id);
        osd_set_obj_list_count(ol, osd_get_obj_list_count(ol) -1);
        osd_track_object( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

        if( osd_get_obj_list_count(ol) > 0)
        {
            if(sel ==  osd_get_obj_list_count(ol))    /* Delete last TP */
                osd_change_focus((POBJECT_HEAD)ol,osd_get_obj_list_count(ol) - 1, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
            else
                tpturn_timer_id = api_start_timer(TPTURN_TIMER_NAME, TPTURN_TIMER_TIME, win_tplist_tpturn_handler);
        }
		else
		{
			tpturn_timer_id = api_start_timer(TPTURN_TIMER_NAME, TPTURN_TIMER_TIME, win_tplist_tpturn_handler);
		}
        break;
    case VACT_TP_SRCH:
        /* If not TP, can't SEARCH */
        if(osd_get_obj_list_count(ol) == 0)
            break;

        sys_data_set_cur_satidx(cur_tp_sat_idx);
        tp_list_pos = osd_get_obj_list_cur_point(ol);
        tp_list_top = osd_get_obj_list_top(ol);

        choice = win_tpsrch_open(&ftaonly_vpid,&srch_chan_apid,&nit_ppid);
        if((choice ==1) || (choice ==2))
            {
              if(s_node.tuner1_valid == 1)
                  {
                   nim=    (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
                  }
             else if(s_node.tuner2_valid == 1)
                 {
                 nim=    (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 1);
                 }
             nim_io_control(nim, NIM_DRIVER_SET_BLSCAN_MODE, 0);
            }
        if(choice==1)
        {
            win_tpsrch_set_search_param(ftaonly_vpid,srch_chan_apid,nit_ppid);
            sub_menu = (POBJECT_HEAD)&g_win_search;
            wincom_close_help();

            if(osd_obj_open(sub_menu, 0xFFFFFFFF) != PROC_LEAVE)
            {
                menu_stack_push(sub_menu);
                set_movedish_flag(TRUE);/*only tp search,need not moving dish when return*/
            }
        }
        else if(choice==2) //pid search
        {
            tp_vpid = ftaonly_vpid;
            tp_apid = srch_chan_apid;
            tp_ppid = nit_ppid;

            prog_callback_register((prog_node_return)win_tplist_set_search_pid_callback);

            get_tp_at(s_node.sat_id,sel,&t_node);
            win_search_set_pid_param(t_node.tp_id, tp_vpid, tp_apid, tp_ppid);

            win_tpsrch_set_search_param(P_SEARCH_FTA|P_SEARCH_SCRAMBLED,P_SEARCH_TV|P_SEARCH_RADIO,0);

            sub_menu = (POBJECT_HEAD)&g_win_search;
            wincom_close_help();
            if(osd_obj_open(sub_menu, 0xFFFFFFFF) != PROC_LEAVE)
                menu_stack_push(sub_menu);

        }
        else
        {
            //OSD_TrackObject( (POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            osd_track_object( (POBJECT_HEAD)&g_win_tplist, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
        break;
    default:
        break;
    }


    return ret;
}

#ifdef PLSN_SUPPORT
static void update_tp_info()
{
	PCONTAINER con = NULL;
	PTEXT_FIELD	txt = NULL;		
	S_NODE s_node;
	T_NODE t_node;
	char str[20];
	UINT16 cur_point = 0;
	UINT32 pls_num = 0;
	UINT16 item_count = 0;

	item_count = osd_get_obj_list_count(&tplst_olist);	
	if (item_count == 0)
	{
		return;
	}
	
	get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx, &s_node);
	cur_point = osd_get_obj_list_cur_point(&tplst_olist);	
	if (cur_point != cur_tp_idx)
	{
		return;
	}
	get_tp_at(s_node.sat_id, cur_point, &t_node);
	if(s_node.tuner1_valid)
	{
	   nim_io_control(g_nim_dev, NIM_DRIVER_GET_PLSN, &pls_num);
	}
	if(s_node.tuner2_valid) 
	{
	   nim_io_control(g_nim_dev2, NIM_DRIVER_GET_PLSN, &pls_num);
	}
	if (pls_num == t_node.pls_num)
	{
		return;
	}	
	t_node.pls_num = pls_num;
	modify_tp(t_node.tp_id, &t_node);
	update_data();
	
	con = (PCONTAINER)osd_get_focus_object((POBJECT_HEAD)(&tplst_olist));
	txt = (PTEXT_FIELD)osd_get_container_next_obj(con);
	while (osd_get_objp_next(txt))
	{
		txt = (PTEXT_FIELD)osd_get_objp_next(txt);
	}	
	sprintf(str, "%d", pls_num);
	osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
	osd_track_object((POBJECT_HEAD)con, C_UPDATE_ALL|C_DRAW_TYPE_HIGHLIGHT);
}
#endif


static PRESULT win_tplist_message_proc(UINT32 msg_type, UINT32 msg_code)
{
    PRESULT ret = PROC_LOOP;
	static UINT32 tp_count=0;
	OBJLIST* ol=&tplst_olist;
	tp_count = osd_get_obj_list_count(ol);
#ifdef PLSN_SUPPORT
    UINT8 lock_stat = 0;
    SYSTEM_DATA *sys_data = NULL;
    static UINT8 previous_stat = 0;
#endif
    
    switch(msg_type)
    {
    case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
		if(0==tp_count)
		{
			break;
		}
        wait_signalmsg_time ++;
        if(wait_signalmsg_time >= 2)
            win_signal_refresh();
    #ifdef PLSN_SUPPORT
        lock_stat = msg_code & 0xff;
        sys_data = sys_data_get();
        if ((sys_data->super_scan) && (lock_stat != previous_stat))
        {
            update_tp_info();
            previous_stat = lock_stat;
        }
    #endif        
        break;
    case CTRL_MSG_SUBTYPE_CMD_TP_TURN:
        wait_signalmsg_time = 0;
        win_tplist_tuner_frontend();
        cur_tp_idx = osd_get_obj_list_cur_point(&tplst_olist);
        break;
    default:
        break;
    }

    return ret;

}

static void win_tplist_set_sat_display(void)
{
    TEXT_FIELD* txt = NULL;
    S_NODE s_node;
    SYSTEM_DATA* sys_data = NULL;
    sys_data = sys_data_get();

    MEMSET(&s_node, 0 ,sizeof(s_node));
    UINT16 name[3*MAX_SERVICE_NAME_LENGTH]={0};
    name[0] = 0;
    txt = &tplst_sat;
    get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx, &s_node);

	//if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_DIFF && dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S) <= 2)
	if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_DIFF )
    {
        if(s_node.tuner1_valid)
            com_asc_str2uni((UINT8 *)"[LNB1] ", name);
        else
            com_asc_str2uni((UINT8 *)"[LNB2] ", name);
        com_uni_str_cat(name, (UINT16 *)s_node.sat_name);
    }
    else
    {
        com_asc_str2uni((UINT8 *)"[LNB1] ", name);
        com_uni_str_cat(name, (UINT16 *)s_node.sat_name);
    }
    osd_set_text_field_content(txt, STRING_UNICODE, (UINT32)name);
}

static void win_tplist_set_display(void)
{
    char str[20]={0};
    UINT16 pol_str_id =0;
    UINT16  __MAYBE_UNUSED__ top =0,cnt =0,page =0,index =0,curitem =0;
    UINT32 i=0;//,n=0;
    UINT32 valid_idx =0;
    S_NODE s_node;
    T_NODE t_node;
    OBJLIST* ol = NULL;
    CONTAINER* item = NULL;
    TEXT_FIELD* txt = NULL;
    INT32 ret = 0;

    MEMSET(&s_node, 0, sizeof(s_node));
    MEMSET(&t_node, 0, sizeof(t_node));
    ol = &tplst_olist;

    cnt = osd_get_obj_list_count(ol);
    page = osd_get_obj_list_page(ol);
    top = osd_get_obj_list_top(ol);
    curitem = osd_get_obj_list_new_point(ol);

    get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx, &s_node);

    for(i=0;i<page;i++)
    {
        item = (CONTAINER*)tplist_items[i];
        index = top + i;

        if(index< cnt)
            valid_idx = 1;
        else
            valid_idx = 0;

        if(valid_idx)
            get_tp_at(s_node.sat_id,index,&t_node);

        /* TP IDX */
        txt = (PTEXT_FIELD)osd_get_container_next_obj(item);
        if(valid_idx)
        {
            ret = snprintf(str,20,"%d",(INT16)(index + 1));
            if(0 == ret)
            {
                ali_trace(&ret);
            }
        }
        else
            strncpy(str,"", (20-1));
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);

        /* Freq*/
        txt = (PTEXT_FIELD)osd_get_objp_next(txt);
        if(valid_idx)
        {
            ret = snprintf(str,20,"%5ldMHz",(INT32)(t_node.frq));
            if(0 == ret)
            {
                ali_trace(&ret);
            }
        }
        else
            strncpy(str,"", (20-1));
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);

        /* Symb */
        txt = (PTEXT_FIELD)osd_get_objp_next(txt);
        if(valid_idx)
        {
            ret = snprintf(str,20,"%5ldKs/s",(INT32)t_node.sym);
            if(0 == ret)
            {
                ali_trace(&ret);
            }
        }
        else
            strncpy(str,"", (20-1));
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);

        /* pol */
        txt = (PTEXT_FIELD)osd_get_objp_next(txt);
        if(!valid_idx)
            pol_str_id = 0;
        else if(t_node.pol == 0)
            pol_str_id = RS_INFO_POLARITY_H;
        else
            pol_str_id = RS_INFO_POLARITY_V;
        osd_set_text_field_content(txt, STRING_ID, (UINT32)pol_str_id);

    
	#ifdef MULTISTREAM_SUPPORT
		/* isid */
		txt = (PTEXT_FIELD)osd_get_objp_next(txt);
		if(!valid_idx)
			strncpy(str,"", 20-1);
		else if(t_node.t2_profile == 1)
			snprintf(str, 20, "M%2d",t_node.plp_id);
		else
			strncpy(str,"S", 20-1); // single stream
		osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
	#endif	

    #ifdef PLSN_SUPPORT
        /* pls */
        txt = (PTEXT_FIELD)osd_get_objp_next(txt);
        if (valid_idx)
        {
            snprintf(str, 20, "%d", t_node.pls_num);
        }
        else
        {
            strncpy(str, "", 20-1);
        }
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)str);
    #endif
    }

}


static void win_tplist_load_sat(BOOL updatesat)
{
    UINT16 i=0,tp_num =0,focus_pos =0,max_top =0;
    OBJLIST* ol = NULL;
    //CONTAINER* item = NULL;
    TEXT_FIELD* txt = NULL;
    SCROLL_BAR* sb = NULL;
    S_NODE s_node;
    T_NODE prog_tp_node;
    P_NODE playing_pnode;

    MEMSET(&s_node, 0, sizeof(s_node));
    MEMSET(&prog_tp_node, 0, sizeof(prog_tp_node));
    MEMSET(&playing_pnode, 0, sizeof(playing_pnode));
    if(cur_tp_sat_idx >= get_tuner_sat_cnt(TUNER_EITHER) )
        cur_tp_sat_idx= 0;

    MEMSET(&playing_pnode,0,sizeof(P_NODE));
    ap_get_playing_pnode(&playing_pnode);
    get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx, &s_node);

    ol = &tplst_olist;
    txt = &tplst_sat;
    sb = &tplst_scb;

    recreate_tp_view(VIEW_SINGLE_SAT, s_node.sat_id);
    tp_num = get_tp_num_sat(s_node.sat_id);
    osd_set_obj_list_count(ol, tp_num);    /* Set count */
    focus_pos = tp_list_pos;//OSD_GetObjListCurPoint(ol);

    if (tp_num > osd_get_obj_list_page(ol))
        max_top = tp_num - osd_get_obj_list_page(ol);
    else
        max_top = 0;

    if (focus_pos == 0xffff)
    {
        if(playing_pnode.sat_id == s_node.sat_id)
        {
            for(i = 0;i < tp_num;i++)
            {
                get_tp_at(playing_pnode.sat_id, i, &prog_tp_node);
                if(prog_tp_node.tp_id == playing_pnode.tp_id)
                {
                    focus_pos = i;
                    if (focus_pos > max_top)
                        tp_list_top = max_top;
                    else
                        tp_list_top = focus_pos;
                    break;
                }
            }
        }
    }
    if(focus_pos >= tp_num)
    {
        focus_pos = 0;
        tp_list_top = 0;
    }

    osd_set_obj_list_cur_point(ol, focus_pos);
    osd_set_obj_list_new_point(ol, focus_pos);
    osd_set_obj_list_top(ol, tp_list_top);
    osd_set_scroll_bar_max(sb,tp_num);
    osd_set_scroll_bar_pos(sb, focus_pos);

    win_tplist_set_sat_display();

    if(updatesat)
        osd_draw_object( (POBJECT_HEAD)txt, C_UPDATE_ALL);

}


static void win_tplist_tpturn_handler(UINT32 nouse)
{
    if(tpturn_timer_id != OSAL_INVALID_ID)
    {
        tpturn_timer_id = OSAL_INVALID_ID;
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_TURN,(UINT32)&g_win_tplist,FALSE);
    }

}

UINT32 win_tplist_key_notify_proc(UINT32 msg_code)
{
    BOOL valid_key = ap_key_map_use_hkey(msg_code, tplst_list_keymap);

    if (!valid_key)
    {
        valid_key = ap_key_map_use_hkey(msg_code, tplst_keymap);
    }
    if (valid_key)
    {
        nim_io_control(g_nim_dev, NIM_DRIVER_STOP_CHANSCAN, 1);
    }
    return 1;
}

static void win_tplist_tuner_frontend(void)
{
    S_NODE s_node;
    T_NODE t_node;
#ifdef NEW_DEMO_FRAME
    struct ft_antenna antenna;
    union ft_xpond    xpond_info;
    struct nim_device *nim = NULL;
#else
    struct cc_antenna_info antenna;
    struct cc_xpond_info    xpond_info;
#endif
#if defined(MULTISTREAM_SUPPORT) || defined(PLSN_SUPPORT)
    SYSTEM_DATA *sys_data = NULL;
#endif

    OBJLIST* ol = NULL;
    UINT32 tp_idx =0;
    UINT16  __MAYBE_UNUSED__ pre_sat_id =0,cur_sat_id=0;
    UINT8 back_saved =0;
    BOOL old_get_key = FALSE;
    MEMSET(&s_node, 0, sizeof(s_node));
    MEMSET(&t_node, 0, sizeof(t_node));
    MEMSET(&antenna, 0, sizeof(antenna));
    MEMSET(&xpond_info, 0, sizeof(xpond_info));

    nim_io_control(g_nim_dev, NIM_DRIVER_STOP_CHANSCAN, 0);
    ap_set_key_notify_proc(win_tplist_key_notify_proc);
    old_get_key = ap_enable_key_task_get_key(TRUE);

    win_signal_set_level_quality(0, 0, 0);
    win_signal_update();
    osd_update_vscr(osd_get_task_vscr(osal_task_get_current_id()));

    ol = &tplst_olist;

    tp_idx = osd_get_obj_list_cur_point(ol);

    get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx,&s_node);
    get_tp_at(s_node.sat_id,tp_idx,&t_node);

#ifdef SUPPORT_TWO_TUNER
    cur_tuner_idx = s_node.tuner1_valid? 0 : 1;
#endif

#ifndef NEW_DEMO_FRAME
    struct cc_antenna_info antenna;
    struct cc_xpond_info    xpond_info;

    sat2antenna_ext(&s_node, &antenna,cur_tuner_idx);
    xpond_info.frq = t_node.frq;
    xpond_info.sym = t_node.sym;
    xpond_info.pol = t_node.pol;

    set_antenna(&antenna);
    set_xpond(&xpond_info);
#else
    MEMSET(&antenna, 0, sizeof(struct ft_antenna));
    MEMSET(&xpond_info, 0, sizeof(union ft_xpond));
    sat2antenna(&s_node, &antenna);
    xpond_info.s_info.type = FRONTEND_TYPE_S;
    xpond_info.s_info.frq = t_node.frq;
    xpond_info.s_info.sym = t_node.sym;
    xpond_info.s_info.pol = t_node.pol;
    xpond_info.s_info.tp_id = t_node.tp_id;
#ifdef MULTISTREAM_SUPPORT
    sys_data = sys_data_get();
    if(sys_data->ms_enable && (1 == t_node.t2_profile))
	{
		xpond_info.s_info.isid = t_node.plp_id;
		xpond_info.s_info.change_type = 1;
	}
	else
	{
		xpond_info.s_info.change_type = 0;
	}
#endif
#ifdef PLSN_SUPPORT
    sys_data = sys_data_get();
    xpond_info.s_info.super_scan = sys_data->super_scan;
    xpond_info.s_info.pls_num = t_node.pls_num;
#endif
    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, (s_node.tuner1_valid ? 0 : (s_node.tuner2_valid ? 1 : 0)));

    frontend_tuning(nim, &antenna, &xpond_info, 1);

#endif

    if(pre_tp_sat_idx != cur_tp_sat_idx)
    {
        get_tuner_sat(TUNER_EITHER, pre_tp_sat_idx, &s_node);
        pre_sat_id = s_node.sat_id;

        get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx, &s_node);
        cur_sat_id = s_node.sat_id;

        wincom_dish_move_popup_open(/*pre_sat_id*/0xFFFF,cur_sat_id,cur_tuner_idx,&back_saved);
        pre_tp_sat_idx = cur_tp_sat_idx;
    }

    ap_enable_key_task_get_key(old_get_key);
    ap_set_key_notify_proc(NULL);
    nim_io_control(g_nim_dev, NIM_DRIVER_STOP_CHANSCAN, 0);
}

static UINT32 win_tplist_check_change(void)
{
    UINT8 back_saved =0;

    if(check_node_modified(TYPE_TP_NODE) )
    {
        win_compopup_init(WIN_POPUP_TYPE_OKNO);
        win_compopup_set_msg(NULL, NULL,RS_MSG_ARE_YOU_SURE_TO_SAVE);
        if(win_compopup_open_ext(&back_saved) != WIN_POP_CHOICE_YES)
            return 0;
        else
            return 1;
    }

    return 0;
}

static void win_tpsrch_set_search_param(UINT32 fta_only,UINT32 srch_chan,UINT32 nit_flag)
{
    S_NODE s_node;
    T_NODE t_node;
    struct as_service_param param;
    OBJLIST* ol = NULL;
    UINT16 tp_idx =0;
    //struct vdec_device *vdec;
    //struct vdec_io_get_frm_para vfrm_param;
    UINT32 addr =0, len =0;
#ifdef PLSN_SUPPORT
    SYSTEM_DATA *sys_data = NULL;
#endif
    
    MEMSET(&s_node, 0, sizeof(s_node));
    MEMSET(&t_node, 0, sizeof(t_node));

    ol = &tplst_olist;
    tp_idx = osd_get_obj_list_cur_point(ol);

    get_tuner_sat(TUNER_EITHER, cur_tp_sat_idx, &s_node);
    get_tp_at(s_node.sat_id,tp_idx,&t_node);
#ifdef NEW_DEMO_FRAME
    MEMSET(&param, 0, sizeof(struct as_service_param));
    param.as_frontend_type = FRONTEND_TYPE_S;
    param.as_prog_attr = fta_only |srch_chan;
    param.as_to = 0;
    param.as_sat_cnt = 1;
    param.sat_ids[0] = s_node.sat_id;
    param.as_from = t_node.tp_id;    
    #ifdef MULTISTREAM_SUPPORT
    param.ms_enable = 1;// Enable MultiStream search
    #endif
  #ifdef PLSN_SUPPORT
    param.pls_num = t_node.pls_num;
    sys_data = sys_data_get();
    param.super_scan = sys_data->super_scan;
  #endif
#else
    param.as_prog_attr = fta_only |srch_chan;
    param.as_to = 0;
    param.as_sat_id = s_node.sat_id;
    param.as_from = t_node.tp_id;
#endif
    param.as_method = nit_flag? AS_METHOD_NIT_TP : AS_METHOD_TP;
    param.as_p_add_cfg = PROG_ADD_REPLACE_OLD;

    // set param
    win_search_set_param(&param);

    update_data();
#if 0
    vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    vfrm_param.ufrm_mode = VDEC_UN_DISPLAY;
    vdec_io_control(vdec, VDEC_IO_GET_FRM, (UINT32)&vfrm_param);
    addr = vfrm_param.t_frm_info.u_c_addr;
    len = (vfrm_param.t_frm_info.u_height*vfrm_param.t_frm_info.u_width*3)/2;
#else
    addr = __MM_AUTOSCAN_DB_BUFFER_ADDR;
    addr &= 0x8fffffff;
    len = __MM_AUTOSCAN_DB_BUFFER_LEN;
#endif
    db_search_init((UINT8 *)addr, len);

}

BOOL win_tplist_set_search_pid_callback(P_NODE* node)
{
    UINT32 i =0;

    if(node->video_pid != tp_vpid)
        return FALSE;
    if(node->pcr_pid != tp_ppid)
        return FALSE;

    for(i=0;i<node->audio_count;i++)
    {
        if(node->audio_pid[i] == tp_apid)
            break;
    }
    if(i==node->audio_count)
        return FALSE;

    UINT8 str[60] = {0};
    com_uni_str_to_asc(node->service_name, (char *)str);
    //soc_printf("v/a/p pid:%d/%d/%d %s\n",node->video_pid,node->audio_pid[i],node->pcr_pid,str);

    return TRUE;
}

BOOL set_movedish_flag(BOOL flag)
{
    tpscan_return = flag;
    return flag;
}
