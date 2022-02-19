/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_com_frontend.c
*
*    Description: To realize the common function of FE
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>
#include <hld/nim/nim_dev.h>
#include <api/libdiseqc/lib_diseqc.h>
#include "string.id"
#include "win_com.h"
#include "win_com_frontend.h"

#define UNICABLE_IF_CHANNEL_MAX_NUM   8   //8 channels
#define UNICABLE_IF_FREQS_MAX_NUM    (sizeof(unicable_freqs_list)/sizeof(unicable_freqs_list[0]) )   //12 freqs
#define LNB_STANDARD_TYPE_CNT       (sizeof(lnb_standard_freqs)/sizeof(lnb_standard_freqs[0]) )
#define LNB_USER_TYPE_CNT           (sizeof(lnb_user_freqs)/2)
#define LNB_UNIVERSAL_TYPE_CNT  (sizeof(lnb_universal_freqs)/2)
#define LNB_UNICABLE_TYPE_CNT   (sizeof(lnb_unicable_freqs)/2)

#define TUNERSAT_API_DEBUG  PRINTF//soc_printf

//////////////////////////////////////////////////////////////////////
// Some common functions : for tuner , sat, tp , LNB & DisEqC1.0,unicable
////////////////////////////////////////////////////////////////////

typedef enum
{
    LNB_FRE_5150 = 0,
    LNB_FRE_5750,
    LNB_FRE_5950,
    LNB_FRE_9750,
    LNB_FRE_10000,
    LNB_FRE_10050,
    LNB_FRE_10450,
    LNB_FRE_10550,
    LNB_FRE_10600,
    LNB_FRE_10700,
    LNB_FRE_10750,
    LNB_FRE_11250,
    LNB_FRE_11300,
    LNB_FRE_10200,         //Unicable LNB
}lnb_fre_t;

static const UINT16 unicable_freqs_list[] = //Unicable
{
    1210,   /*  0  */
    1284,   /*  1  */
    1400,   /*  2  */
    1420,   /*  3  */
    1516,   /*  4  */
    1632,   /*  5  */
    1680,   /*  6  */
    1748,   /*  7  */
    1864,   /*  8  */
    1980,   /*  9  */
    2040,   /*  10  */
    2096,   /*  11  */
};

static const UINT16 lnb_freqs_list[] =
{
    5150,   /*  0  */
    5750,   /*  1  */
    5950,   /*  2  */
    9750,   /*  3  */
    10000,  /*  4  */
    10050,  /*  5  */
    10450,  /*  6  */
    10550,  /*  7  */
    10600,  /*  8  */
    10700,  /*  9  */
    10750,  /*  10  */
    11250,  /*  11  */
    11300,  /*  12  */
    10200   /*  13 */
};

static const UINT8  lnb_standard_freqs[] =
{
    LNB_FRE_5150,
    LNB_FRE_5750,
    LNB_FRE_5950,
    LNB_FRE_9750,
    LNB_FRE_10000,
    LNB_FRE_10050,
    LNB_FRE_10450,
    LNB_FRE_10600,
    LNB_FRE_10700,
    LNB_FRE_10750,
    LNB_FRE_11250,
    LNB_FRE_11300
};

static const UINT8 lnb_user_freqs[][2] =
{
    {LNB_FRE_5150,   LNB_FRE_5750},
    {LNB_FRE_5750,   LNB_FRE_5150},
};

static const UINT8 lnb_universal_freqs[][2] =
{
    {LNB_FRE_9750,   LNB_FRE_10550},
    {LNB_FRE_9750,   LNB_FRE_10600},
    {LNB_FRE_9750,   LNB_FRE_10700},
    {LNB_FRE_9750,   LNB_FRE_10750},
};

static const UINT8 lnb_unicable_freqs[][2] =  //unicable
{
    {LNB_FRE_9750,   LNB_FRE_10600}, //9750,10600
    {LNB_FRE_10200,  LNB_FRE_10200}, //10200
};

UINT16 lnb_type_ids[4] =
{
    RS_INFO_LNB_STANDARD,
    RS_INFO_LNB_USER,
    RS_INFO_LNB_UNIVERSAL,
    RS_INFO_LNB_UNICABLE,
};

UINT16 diseqc_port_ids[] =
{
    RS_DISPLAY_DISABLE,
    RS_INFO_PORT1,
    RS_INFO_PORT2,
    RS_INFO_PORT3,
    RS_INFO_PORT4
};

UINT8   antset_cur_tuner = 0;

UINT8 get_universal_lnb_index(void)
{
    return (LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT);
}

UINT16 get_tuner_strid(UINT32 tuner_idx)
{
    UINT16 str_id = 0;
    SYSTEM_DATA *sys_data = NULL;

    sys_data = sys_data_get();

    if(LNB_FIXED == sys_data->tuner_lnb_type[tuner_idx])
    {
        str_id = (0 == tuner_idx)? RS_LNB1_FIXED : RS_LNB2_FIXED;
    }
    else
    {
        str_id = (0 == tuner_idx)? RS_LNB1_MOTOR : RS_LNB2_MOTOR;
    }

    return str_id;
}

#ifdef NEW_DEMO_FRAME

UINT32 get_tuner_cnt(void)
{
    UINT32 tuner_cnt  = 0;
    SYSTEM_DATA *sys_data = NULL;
    UINT32 tuner_num_2 = 2;

    if (tuner_num_2 == g_tuner_num)
    {
        sys_data = sys_data_get();

        if ((ANTENNA_CONNECT_DUAL_DIFF == sys_data->antenna_connect_type)
            && (2 == dev_get_device_cnt(HLD_DEV_TYPE_NIM, FRONTEND_TYPE_S)))
        {
            tuner_cnt = 2;
        }
        else
        {
            tuner_cnt = 1;
        }       
    }
    else
    {
        tuner_cnt = 1;
    }

    return tuner_cnt;
}

UINT16 get_tuner_sat_cnt(UINT16 tuner_idx)
{
    UINT16 sat_cnt = 0;
    __MAYBE_UNUSED__ UINT16 sat_select_mode=0;
    __MAYBE_UNUSED__ UINT16 tuner_cnt = 0;
    __MAYBE_UNUSED__ UINT32 tuner_num_2 = 2;
#ifdef SUPPORT_COMBO_T2_S2
    //tuner1 DVBT2,tuner2 DVBS2
    sat_cnt = get_sat_num(SET_SELECTED | SET_TUNER2_VALID);
#else
    if (tuner_num_2 == g_tuner_num)
    {
        tuner_cnt = get_tuner_cnt();

        switch(tuner_idx)
        {
        case TUNER_FIRST:
            sat_select_mode = SET_TUNER1_VALID;
            break;
        case TUNER_SECOND:
            sat_select_mode = SET_TUNER2_VALID;
            if (1 == tuner_cnt)
            {
                sat_select_mode = SET_TUNER1_VALID;
                TUNERSAT_API_DEBUG("Only one tuner : TUNER_SECOND invalid!!!");
            }
            break;
        case TUNER_EITHER:
            sat_select_mode = SET_EITHER_TUNER_VALID;
            if(1 == tuner_cnt)
            {
                sat_select_mode = SET_TUNER1_VALID;
            }
            break;
        case TUNER_EITHER_SELECT:
            sat_select_mode =  SET_SELECTED | SET_TUNER1_VALID | SET_TUNER2_VALID;
            if(1 == tuner_cnt)
            {
                sat_select_mode = SET_SELECTED | SET_TUNER1_VALID;
            }
            break;
        default:
            break;
        }

        /* TODO: Get the select satellite for tuner : tuner_idx*/

        sat_cnt = get_sat_num(sat_select_mode);
    }
    else
    {
        sat_cnt = get_sat_num(SET_SELECTED | SET_TUNER1_VALID);
    }
#endif
    return sat_cnt;
}

void get_tuner_sat(UINT32 tuner_idx, UINT32 sat_idx, S_NODE *snode)
{
    __MAYBE_UNUSED__ UINT16 sat_select_mode=0;
    __MAYBE_UNUSED__ UINT32 tuner_cnt = 0;
    __MAYBE_UNUSED__ UINT32 tuner_num_2 = 2;
 #ifdef SUPPORT_COMBO_T2_S2
    //tuner1 DVBT2,tuner2 DVBS2
    get_sat_at(sat_idx, SET_SELECTED | SET_TUNER2_VALID,snode);
#else
    if (tuner_num_2 == g_tuner_num)
    {
        tuner_cnt = get_tuner_cnt();
        switch(tuner_idx)
        {
        case TUNER_FIRST:
            sat_select_mode = SET_TUNER1_VALID;
            break;
        case TUNER_SECOND:
            sat_select_mode = SET_TUNER2_VALID;
            if(1 == tuner_cnt)
            {
                sat_select_mode = SET_TUNER1_VALID;
                TUNERSAT_API_DEBUG("Only one tuner : TUNER_SECOND invalid!!!");
            }
            break;
        case TUNER_EITHER:
            sat_select_mode = SET_EITHER_TUNER_VALID;
            if (1 == tuner_cnt)
            {
                sat_select_mode = SET_TUNER1_VALID;
            }
            break;
        case TUNER_EITHER_SELECT:
            sat_select_mode =  SET_SELECTED | SET_TUNER1_VALID | SET_TUNER2_VALID;
            if(1 == tuner_cnt)
            {
                sat_select_mode = SET_SELECTED | SET_TUNER1_VALID;
            }
            break;
        default:
            break;
        }
        get_sat_at(sat_idx,sat_select_mode, snode);
    }
    else
    {
        get_sat_at(sat_idx, SET_SELECTED | SET_TUNER1_VALID,snode);
    }
#endif    
}

void get_tuner_sat_name(UINT32 tuner_idx, UINT32 sat_idx, UINT16 *unistr)
{
    S_NODE s_node;
    UINT32 sat_cnt = 0;
    UINT16 sat_select_mode=0;
    char str[30] = {0};
    UINT32 str_len = 0;
    __MAYBE_UNUSED__ UINT32 tuner_cnt = 0;
    __MAYBE_UNUSED__ UINT32 tuner_num_2 = 2;
#ifdef SUPPORT_COMBO_T2_S2
    //tuner1 DVBT2,tuner2 DVBS2
    sat_select_mode = SET_SELECTED | SET_TUNER2_VALID;
#else
    MEMSET(&s_node, 0x0, sizeof(S_NODE));
    if (tuner_num_2 == g_tuner_num)
    {
        tuner_cnt = get_tuner_cnt();

        switch(tuner_idx)
        {
        case TUNER_FIRST:
            sat_select_mode = SET_TUNER1_VALID;
            break;
        case TUNER_SECOND:
            sat_select_mode = SET_TUNER2_VALID;
            if(1 == tuner_cnt)
            {
                sat_select_mode = SET_TUNER1_VALID;
                TUNERSAT_API_DEBUG("Only one tuner : TUNER_SECOND invalid!!!");
            }
            break;
        case TUNER_EITHER:
            sat_select_mode = SET_EITHER_TUNER_VALID;
            if(1 == tuner_cnt)
            {
                sat_select_mode = SET_TUNER1_VALID;
            }
            break;
        case TUNER_EITHER_SELECT:
            sat_select_mode =  SET_SELECTED | SET_TUNER1_VALID | SET_TUNER2_VALID;
            if(1 == tuner_cnt)
            {
                sat_select_mode = SET_SELECTED | SET_TUNER1_VALID;
            }
            break;
        default:
            break;
        }
    }
    else
    {
        sat_select_mode = SET_SELECTED | SET_TUNER1_VALID;
    }
#endif
    sat_cnt = get_sat_num(sat_select_mode);
    if(sat_idx < sat_cnt)
    {
        get_sat_at(sat_idx,sat_select_mode,&s_node);
        snprintf(str,30, "(%lu/%lu) ",sat_idx + 1, sat_cnt);
        com_asc_str2uni((UINT8 *)str, unistr);
        str_len = com_uni_str_len(unistr);
        com_uni_str_copy_char((UINT8*)(&unistr[str_len]), s_node.sat_name);
    }
    else
    {
        com_asc_str2uni((UINT8 *)"No Sat", unistr);
    }
}

#else

UINT32 get_tuner_cnt(void)
{
    UINT32 tuner_cnt;
    SYSTEM_DATA *sys_data;

#ifdef SUPPORT_TWO_TUNER
    sys_data = sys_data_get();

    if(sys_data->antenna_connect_type == ANTENNA_CONNECT_DUAL_DIFF)
    {
        tuner_cnt = 2;
    }
    else
    {
        tuner_cnt = 1;
    }
#else
    tuner_cnt = 1;
#endif

    return tuner_cnt;
}

UINT16 get_tuner_sat_cnt(UINT16 tuner_idx)
{
    UINT16 sat_cnt;
#ifdef SUPPORT_TWO_TUNER

    UINT16 sat_select_mode=0;
    UINT16 tuner_cnt;

    tuner_cnt = get_tuner_cnt();

    switch(tuner_idx)
    {
    case TUNER_FIRST:
        sat_select_mode = SET_TUNER1_VALID;
        break;
    case TUNER_SECOND:
        sat_select_mode = SET_TUNER2_VALID;
        if(1 == tuner_cnt)
        {
            sat_select_mode = SET_TUNER1_VALID;
            TUNERSAT_API_DEBUG("Only one tuner : TUNER_SECOND invalid!!!");
        }
        break;
    case TUNER_EITHER:
        sat_select_mode = SET_EITHER_TUNER_VALID;
        if(1 == tuner_cnt)
        {
            sat_select_mode = SET_TUNER1_VALID;
        }
        break;
    case TUNER_EITHER_SELECT:
        sat_select_mode =  SET_SELECTED | SET_TUNER1_VALID | SET_TUNER2_VALID;
        if(1 == tuner_cnt)
        {
            sat_select_mode = SET_SELECTED | SET_TUNER1_VALID;
        }
        break;
    }

    /* TODO: Get the select satellite for tuner : tuner_idx*/

    sat_cnt = get_sat_num(sat_select_mode);
#else
    #ifdef SELECT_SAT_ONLY
        sat_cnt = get_sat_num(SET_SELECTED);
    #else
        sat_cnt = get_sat_num(VIEW_ALL);
    #endif
#endif
    return sat_cnt;
}

void get_tuner_sat(UINT32 tuner_idx, UINT32 sat_idx, S_NODE *snode)
{
#ifdef SUPPORT_TWO_TUNER
    UINT16 sat_select_mode=0;
    UINT32 tuner_cnt;

    tuner_cnt = get_tuner_cnt();
    switch(tuner_idx)
    {
    case TUNER_FIRST:
        sat_select_mode = SET_TUNER1_VALID;
        break;
    case TUNER_SECOND:
        sat_select_mode = SET_TUNER2_VALID;
        if(1 == tuner_cnt)
        {
            sat_select_mode = SET_TUNER1_VALID;
            TUNERSAT_API_DEBUG("Only one tuner : TUNER_SECOND invalid!!!");
        }
        break;
    case TUNER_EITHER:
        sat_select_mode = SET_EITHER_TUNER_VALID;
        if(1 == tuner_cnt)
        {
            sat_select_mode = SET_TUNER1_VALID;
        }
        break;
    case TUNER_EITHER_SELECT:
        sat_select_mode =  SET_SELECTED | SET_TUNER1_VALID | SET_TUNER2_VALID;
        if(1 == tuner_cnt)
        {
            sat_select_mode = SET_SELECTED | SET_TUNER1_VALID;
        }
        break;
    }

    get_sat_at(sat_idx,sat_select_mode, snode);
#else
    #ifdef SELECT_SAT_ONLY
        get_sat_at(sat_idx, SET_SELECTED,snode);
    #else
        get_sat_at(sat_idx,VIEW_ALL,snode);
    #endif
#endif
}

void get_tuner_sat_name(UINT32 tuner_idx, UINT32 sat_idx, UINT16 *unistr)
{
    S_NODE s_node;
    UINT32 sat_cnt;
    UINT16 sat_select_mode=0;
    char str[30];
    UINT32 str_len;
#ifdef SUPPORT_TWO_TUNER
    UINT32 tuner_cnt;

    tuner_cnt = get_tuner_cnt();
    switch(tuner_idx)
    {
    case TUNER_FIRST:
        sat_select_mode = SET_TUNER1_VALID;
        break;
    case TUNER_SECOND:
        sat_select_mode = SET_TUNER2_VALID;
        if(1 == tuner_cnt)
        {
            sat_select_mode = SET_TUNER1_VALID;
            TUNERSAT_API_DEBUG("Only one tuner : TUNER_SECOND invalid!!!");
        }
        break;
    case TUNER_EITHER:
        sat_select_mode = SET_EITHER_TUNER_VALID;
        if(1 == tuner_cnt)
        {
            sat_select_mode = SET_TUNER1_VALID;
        }
        break;
    case TUNER_EITHER_SELECT:
        sat_select_mode =  SET_SELECTED | SET_TUNER1_VALID | SET_TUNER2_VALID;
        if(1 == tuner_cnt)
        {
            sat_select_mode = SET_SELECTED | SET_TUNER1_VALID;
        }
        break;
    }
#else
    #ifdef SELECT_SAT_ONLY
        sat_select_mode = SET_SELECTED;
    #else
        sat_select_mode = VIEW_ALL;
    #endif
#endif
    sat_cnt = get_sat_num(sat_select_mode);
    if(sat_idx < sat_cnt)
    {
        get_sat_at(sat_idx,sat_select_mode,&s_node);
        snprintf(str,30,"(%d/%d) ",sat_idx + 1, sat_cnt);
        com_asc_str2uni(str, unistr);
        str_len = com_uni_str_len(unistr);
        com_uni_str_copy_char((UINT8*)(&unistr[str_len]), s_node.sat_name);
    }
    else
    {
        com_asc_str2uni("No Sat", unistr);
    }
}
#endif

#ifndef _BUILD_OTA_E_

BOOL wincom_reset_nim(void)
{
#ifdef DVBS_SUPPORT
    int tuner_idx = cur_tuner_idx;
#endif
    P_NODE p_node;
    S_NODE s_node;
    T_NODE t_node;

    MEMSET(&p_node, 0x0, sizeof (p_node));
    MEMSET(&s_node, 0x0, sizeof (s_node));
    MEMSET(&t_node, 0x0, sizeof (t_node));

#ifndef NEW_DEMO_FRAME
    struct cc_antenna_info antenna;
    struct cc_xpond_info    xpond_info;
#else
#ifdef DVBS_SUPPORT
    struct ft_antenna antenna;
    union ft_xpond xpond_info;
    struct nim_device *nim = NULL;
#endif
#endif

#ifdef NEW_DEMO_FRAME
    INT32 ts_route_id = 0;
    struct ts_route_info ts_route;

    MEMSET(&ts_route, 0x0, sizeof(struct ts_route_info));
    if(ts_route_get_by_type(TS_ROUTE_MAIN_PLAY, (UINT32 *)&ts_route_id, &ts_route) == RET_FAILURE)
    {
        //libc_printf("cann't find route\n");
        return FALSE;
    }

    if(!(ts_route.state&TS_ROUTE_STATUS_PLAY))
    {
        libc_printf("TS Route Main Play stopped\n");
        return FALSE;
    }
#endif

    if(SUCCESS != get_prog_at(sys_data_get_cur_group_cur_mode_channel(), &p_node))
    {
        return FALSE;
    }

    if(SUCCESS!=get_tp_by_id(p_node.tp_id, &t_node))
    {
        return FALSE;
    }
    get_sat_by_id(t_node.sat_id, &s_node);

#ifndef DVBS_SUPPORT
    return FALSE;
#else

#if defined(SUPPORT_TWO_TUNER) && !defined(BC_PVR_SUPPORT)
    tuner_idx = s_node.tuner1_valid? 0 : (s_node.tuner2_valid? 1 : 0);
#endif

    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, tuner_idx);
    if(FRONTEND_TYPE_S != dev_get_device_sub_type(nim, HLD_DEV_TYPE_NIM))
    {
        return FALSE ;
    }

#ifndef NEW_DEMO_FRAME
    struct cc_antenna_info antenna;
    struct cc_xpond_info    xpond_info;

    sat2antenna_ext(&s_node, &antenna,tuner_idx);
    xpond_info.frq = t_node.frq;
    xpond_info.sym = t_node.sym;
    xpond_info.pol = t_node.pol;

    set_antenna(&antenna);
    set_xpond(&xpond_info);
#else
    MEMSET(&antenna, 0, sizeof(struct ft_antenna));
    MEMSET(&xpond_info, 0, sizeof(union ft_xpond));
  #ifdef DVBS_SUPPORT
    sat2antenna(&s_node, &antenna);
    xpond_info.s_info.type = FRONTEND_TYPE_S;
    xpond_info.s_info.frq = t_node.frq;
    xpond_info.s_info.sym = t_node.sym;
    xpond_info.s_info.pol = t_node.pol;
    xpond_info.s_info.tp_id = t_node.tp_id;
#ifdef PLSN_SUPPORT
    xpond_info.s_info.super_scan = 0;
    xpond_info.s_info.pls_num = t_node.pls_num;
#endif
    
    nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, tuner_idx);

    frontend_tuning(nim, &antenna, &xpond_info, 1);
  #endif
#endif
    return TRUE;
#endif//DVBS_SUPPORT
}
#endif

void get_tp_name(UINT32 sat_id, UINT32 tp_pos,UINT16 *unistr)
{
    T_NODE t_node;
    UINT32 tp_cnt = 0;
    char str[30] = {0};

    MEMSET(&t_node, 0x0, sizeof(T_NODE));
    tp_cnt = get_tp_num_sat(sat_id);
    if(tp_pos<tp_cnt)
    {
        get_tp_at(sat_id,tp_pos,&t_node);
        snprintf(str,30,"(%lu/%lu) %lu %c %lu",tp_pos+1,tp_cnt,t_node.frq,(0 == t_node.pol)? 'H' : 'V',t_node.sym);
    }
    else
    {
        strncpy(str, "No TP", 29);
    }
    com_asc_str2uni((UINT8 *)str, unistr);
}

UINT32 get_lnb_type_cnt(void)
{
#ifdef SUPPORT_UNICABLE
       return LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT + LNB_UNIVERSAL_TYPE_CNT + LNB_UNICABLE_TYPE_CNT;
#endif
    return LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT + LNB_UNIVERSAL_TYPE_CNT ;
}

UINT32 get_lnb_type_index(UINT32 lnb_type, UINT32 lnb_freq1, UINT32 lnb_freq2)
{
    UINT32 i = 0;
    UINT32 freq1 = 0;
    UINT32 freq2 = 0;

    switch(lnb_type)
    {
    case LNB_CTRL_STD:
        for(i=0;i<LNB_STANDARD_TYPE_CNT;i++)
        {
            freq1 = lnb_freqs_list[lnb_standard_freqs[i]];
            if(freq1 == lnb_freq1)
            {
                return i;
            }
        }
        break;
    case LNB_CTRL_POL:
        for(i=0;i<LNB_USER_TYPE_CNT;i++)
        {
            freq1 = lnb_freqs_list[lnb_user_freqs[i][0]];
            freq2 = lnb_freqs_list[lnb_user_freqs[i][1]];
            if((freq1  == lnb_freq1) && (freq2 == lnb_freq2))
            {
                return LNB_STANDARD_TYPE_CNT + i;
            }
        }
        break;
    case LNB_CTRL_22K:
        for(i=0;i<LNB_UNIVERSAL_TYPE_CNT;i++)
        {
            freq1 = lnb_freqs_list[lnb_universal_freqs[i][0]];
            freq2 = lnb_freqs_list[lnb_universal_freqs[i][1]];

            if((freq1  == lnb_freq1) && (freq2 == lnb_freq2))
            {
                return LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT + i;
            }
        }
        break;
    case LNB_CTRL_UNICABLE:
        for(i=0;i<LNB_UNICABLE_TYPE_CNT;i++)
        {
            freq1 = lnb_freqs_list[lnb_unicable_freqs[i][0]];
            freq2 = lnb_freqs_list[lnb_unicable_freqs[i][1]];
            if((freq1  == lnb_freq1) && (freq2 == lnb_freq2))
            {
                return LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT + LNB_UNIVERSAL_TYPE_CNT + i;
            }
        }
        break;
    default:
        break;
    }

    return 0xFFFF;
}

void get_lnb_type_setting(UINT32 lnb_idx,UINT32 *lnb_type, UINT32 *lnb_freq1, UINT32 *lnb_freq2)
{
    if(lnb_idx >= get_lnb_type_cnt())
    {
        *lnb_type = LNB_CTRL_STD;
        *lnb_freq1 = lnb_freqs_list[lnb_standard_freqs[0]];//lnb_freqs_list[0];
        *lnb_freq2 = *lnb_freq1;
    }
    else if(lnb_idx < LNB_STANDARD_TYPE_CNT)
    {
        *lnb_type = LNB_CTRL_STD;
        *lnb_freq1 = lnb_freqs_list[lnb_standard_freqs[lnb_idx]];
        *lnb_freq2 = *lnb_freq1;
    }
    else if(lnb_idx < LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT)
    {
        lnb_idx -= LNB_STANDARD_TYPE_CNT;
        *lnb_type = LNB_CTRL_POL;
        *lnb_freq1 = lnb_freqs_list[lnb_user_freqs[lnb_idx][0]];
        *lnb_freq2 = lnb_freqs_list[lnb_user_freqs[lnb_idx][1]];
    }
    else if(lnb_idx < LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT + LNB_UNIVERSAL_TYPE_CNT)
    {
        lnb_idx -= LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT;
        *lnb_type = LNB_CTRL_22K;
        *lnb_freq1 = lnb_freqs_list[lnb_universal_freqs[lnb_idx][0]];
        *lnb_freq2 = lnb_freqs_list[lnb_universal_freqs[lnb_idx][1]];
    }
    else
    {
        lnb_idx -= LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT + LNB_UNIVERSAL_TYPE_CNT;
        *lnb_type = LNB_CTRL_UNICABLE;
        *lnb_freq1 = lnb_freqs_list[lnb_unicable_freqs[lnb_idx][0]];
        *lnb_freq2 = lnb_freqs_list[lnb_unicable_freqs[lnb_idx][1]];
    }
}

void get_lnb_name(UINT32 lnb_idx,UINT16 *unistr)
{
    UINT32 str_len = 0;
    char    str[30] = {0};
    UINT16 lnb_freq1 = 0;
    UINT16 lnb_freq2 = 0;

    if(lnb_idx >= get_lnb_type_cnt())
    {
        com_asc_str2uni((UINT8 *)"Invaldi LNB type", unistr);
        return;
    }
    if(lnb_idx < LNB_STANDARD_TYPE_CNT)
    {
        lnb_freq1 = lnb_freqs_list[lnb_standard_freqs[lnb_idx]];
        snprintf(str, 30, "%d",lnb_freq1);
    }
    else if(lnb_idx < LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT)
    {
        lnb_idx -= LNB_STANDARD_TYPE_CNT;
        lnb_freq1 = lnb_freqs_list[lnb_user_freqs[lnb_idx][0]];
        lnb_freq2 = lnb_freqs_list[lnb_user_freqs[lnb_idx][1]];
        snprintf(str, 30, "%d-%d",lnb_freq1,lnb_freq2);
    }
    else if(lnb_idx < LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT + LNB_UNIVERSAL_TYPE_CNT)
    {
        lnb_idx -= LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT;
        com_uni_str_copy_char((UINT8*)unistr,osd_get_unicode_string(lnb_type_ids[2]));
        str_len = com_uni_str_len(unistr);
        lnb_freq1 = lnb_freqs_list[lnb_universal_freqs[lnb_idx][0]];
        lnb_freq2 = lnb_freqs_list[lnb_universal_freqs[lnb_idx][1]];
        snprintf(str, 30, "(%d-%d)",lnb_freq1,lnb_freq2);
    }
    else
    {
        lnb_idx -= LNB_STANDARD_TYPE_CNT + LNB_USER_TYPE_CNT + LNB_UNIVERSAL_TYPE_CNT;
        com_uni_str_copy_char((UINT8*)unistr,osd_get_unicode_string(lnb_type_ids[3]));
        str_len = com_uni_str_len(unistr);
        lnb_freq1 = lnb_freqs_list[lnb_unicable_freqs[lnb_idx][0]];
        lnb_freq2 = lnb_freqs_list[lnb_unicable_freqs[lnb_idx][1]];
        if (lnb_freq1 == lnb_freq2)
        {
            snprintf(str, 30, "(%d)",lnb_freq1);
        }
        else
        {
            snprintf(str, 30, "(%d-%d)",lnb_freq1,lnb_freq2);
        }
    }

    com_asc_str2uni((UINT8 *)str, &unistr[str_len]);
}

UINT32 get_diseqc10_cnt(void)
{
    return 5;
}

void get_diseqc10_name(UINT32 index, char *str __MAYBE_UNUSED__, 
    UINT16 *unistr __MAYBE_UNUSED__,UINT16 *str_id)
{
    *str_id = diseqc_port_ids[index];
}

UINT32 get_diseqc11_cnt(void)
{
    return 29;
}

void get_diseqc11_name(UINT32 val, UINT16 *unistr)
{
    char str[30] = {0};
    UINT8 *pstr = NULL;
    UINT32 max_val_4 = 4;
    UINT32 max_val_8 = 8;
    UINT32 max_val_12 = 12;

    if(0 == val)
    {
        pstr = osd_get_unicode_string(RS_DISPLAY_DISABLE);
        com_uni_str_copy_char( (UINT8*)unistr,pstr);
    }
    else
    {
        max_val_4 = 4;
        max_val_8 = 8;
        max_val_12 = 12;
        if(max_val_4 >= val)
        {
            snprintf(str,30,"1Cascade M1: Port%lu",(val - 0));
        }
        else if(max_val_8 >= val)
        {
            snprintf(str,30,"1Cascade M2: Port%lu",(val - 4));
        }
        else if(max_val_12 >= val)
        {
            snprintf(str,30,"1Cascade M3: Port%lu",(val - 8));
        }
        else
        {
            snprintf(str,30,"2Cascades: Port%lu",(val - 12));
        }
        com_asc_str2uni((UINT8 *)str, unistr);
    }
}

UINT16 get_unicable_freq(UINT8 index)
{
    if(index < sizeof(unicable_freqs_list)/sizeof(unicable_freqs_list[0]))
    {
        return unicable_freqs_list[index];
    }
    else
    {
        return unicable_freqs_list[0];
    }
}

UINT32 get_unicable_freq_index(UINT16 freq)
{
    UINT32 i = 0;

    for(i = 0;i<UNICABLE_IF_FREQS_MAX_NUM;i++)
    {
        if(freq == unicable_freqs_list[i])
        {
            return i;
        }
    }
    return 0;
}

UINT16 get_if_channel_cnt(void)
{
    return UNICABLE_IF_CHANNEL_MAX_NUM;
}

void get_if_channel_name(UINT32 channel_idx,UINT16 *unistr)
{
    char str[2] = {0};

    if (channel_idx >= UNICABLE_IF_CHANNEL_MAX_NUM)
    {
        channel_idx = 0;
    }
    snprintf(str,2, "%lu",channel_idx+1);
    com_asc_str2uni((UINT8 *)str,unistr);
}

UINT16 get_centre_freqs_cnt(void)
{
    return UNICABLE_IF_FREQS_MAX_NUM;
}

void get_centre_freqs_name(UINT32 freq_idx,UINT16 *unistr)
{
    char    str[10] = {0};
    UINT16 freq = 0;
    
    if (freq_idx >= UNICABLE_IF_FREQS_MAX_NUM)
    {
        freq_idx = 0;
    }

    freq = unicable_freqs_list[freq_idx];
    snprintf(str,10,"%d",freq);
    com_asc_str2uni((UINT8 *)str, unistr);
}

UINT32 diseqc11_db_to_ui(UINT32 diseqc11_type, UINT32 diseqc11_port)
{
    UINT32 diseqc11_mode = 0;
    UINT32 val = 0;

    switch(diseqc11_type)
    {
    case DISEQC11_TYPE_DISABLE:
    case DISEQC11_TYPE_RESERVED:
        break;
    case DISEQC11_TYPE_1CASCADE:
        diseqc11_mode = diseqc11_port & 0x0C;
        if(DISEQC11_1CASCADE_RESERVED == diseqc11_mode)
        {
            val = 1 + (DISEQC11_1CASCADE_MODE1 | (diseqc11_port & 3));
        }
        else
        {
            val = 1 + diseqc11_port;
        }
        break;
    case DISEQC11_TYPE_2CASCADE:
        val = 1 + 12 + diseqc11_port;
        break;
    default:
        break;
    }

    return val;
}

void diseqc11_ui_to_db(UINT32 val, UINT8 *type, UINT8 *port)
{
    const UINT8 diseqc11_mode1 = 4;
    const UINT8 diseqc11_mode2 = 8;
    const UINT8 diseqc11_mode3 = 12;

    if(0 == val)
    {
        *type = DISEQC11_TYPE_DISABLE;
        *port = 0;
    }
    else if(val <= diseqc11_mode1)
    {
        *type = DISEQC11_TYPE_1CASCADE;
        *port = DISEQC11_1CASCADE_MODE1 | (val - 1);
    }
    else if(val <= diseqc11_mode2)
    {
        *type = DISEQC11_TYPE_1CASCADE;
        *port = DISEQC11_1CASCADE_MODE2 | (val - 5);
    }
    else if(val <= diseqc11_mode3)
    {
        *type = DISEQC11_TYPE_1CASCADE;
        *port = DISEQC11_1CASCADE_MODE3 | (val - 9);
    }
    else
    {
        *type = DISEQC11_TYPE_2CASCADE;
        *port = val - 13;
    }
}

UINT16 win_load_default_satidx(void)
{
    UINT16 i = 0;
    UINT16 sat_cnt = 0;
    UINT16 sat_pos = 0;
    S_NODE s_node;
    P_NODE playing_pnode;
    UINT8  tuner_idx = 0;
    sat_pos = 0;
    UINT32 tuner_num_2 = 2;

    MEMSET(&s_node, 0x0, sizeof (s_node));
    MEMSET(&playing_pnode,0,sizeof(P_NODE));

    ap_get_playing_pnode(&playing_pnode);

    if(tuner_num_2 == get_tuner_cnt())
    {
       tuner_idx = TUNER_EITHER;
    }
    else
    {
       tuner_idx = antset_cur_tuner;
    }

    sat_cnt = get_tuner_sat_cnt(tuner_idx);
    for(i = 0;i < sat_cnt;i++)
    {
        get_tuner_sat(tuner_idx,i,&s_node);
        if(s_node.sat_id == playing_pnode.sat_id)
        {
            sat_pos = i;
            break;
        }
    }
    return sat_pos;
}

