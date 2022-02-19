/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_search_ext_function.c
*
*    Description: The internal function of search
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include "win_search.h"
#include "win_search_ext_function.h"
#ifdef SHOW_WELCOME_FIRST
#include "win_welcom.h"
#endif

#define FREE_P_SAFELY(p) do{\
                        if(NULL!=p)\
                            FREE(p);\
                     } while(0)

typedef struct
{
    struct as_service_param search_param;
    //sat
    BOOL   moving_disk;
    UINT16 cur_searching_sat_no;
    UINT16 pre_sat_id;
    UINT16 cur_sat_id;

    //tp
    BOOL   cur_tp_srched_ok;
    UINT16 searched_tp_cnt;

    //prog
    UINT16 searched_channel_cnt[2];  //tv, radio
    P_NODE cur_searched_prog[2];

    //animation
    UINT32 animation_timer_loop;

    //pid search param
    BOOL pid_srch_flag;
    UINT32 pid_srch_v;
    UINT32 pid_srch_a;
    UINT32 pid_srch_p;
    UINT32 pid_srch_tp;

    BOOL search_stop_signal;
    BOOL pre_close_signal;
    BOOL power_off_signal;


}win_srch_priv;

#ifdef SUPPORT_FRANCE_HD
static void tnt_same_service_proc(UINT32 tp_id1, UINT32 tp_id2)
{
    P_NODE node;
    INT32 num1, num2, i, j;
    UINT16 *service_id = NULL, *lcn = NULL,*intensity = NULL;
    UINT8   channel_mode;
    UINT32 max_valid_lcn =0;
    UINT16 *service_type = NULL,*hd_lcn = NULL;
    UINT16 *hd_lcn_ture = NULL,*quality = NULL;    
    UINT16 *network_id = NULL;
    T_NODE tp;

    MEMSET(&node, 0, sizeof(P_NODE));
    recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE, tp_id1);
    num1 = get_prog_num(VIEW_ALL|PROG_TVRADIO_MODE, 0);

    service_type= (UINT16*)MALLOC(num1*2);  
    hd_lcn = (UINT16*)MALLOC(num1*2);
    hd_lcn_ture=(UINT16*)MALLOC(num1*2);
    service_id = (UINT16*)MALLOC(num1*2);
    lcn = (UINT16*)MALLOC(num1*2);
    network_id= (UINT16*)MALLOC(num1*2);
    intensity=(UINT16*)MALLOC(num1*2);
    quality=(UINT16*)MALLOC(num1*2);

    if((NULL == service_type) || (NULL == hd_lcn) || (NULL == hd_lcn_ture)
        ||(NULL == service_id) ||(NULL == lcn) ||(NULL == network_id)
        ||(NULL == intensity) || (NULL == quality))
    {
        FREE_P_SAFELY(service_type);
        FREE_P_SAFELY(hd_lcn);
        FREE_P_SAFELY(hd_lcn_ture);
        FREE_P_SAFELY(service_id);
        FREE_P_SAFELY(lcn);
        FREE_P_SAFELY(network_id);
        FREE_P_SAFELY(intensity);
        FREE_P_SAFELY(quality);
        return;
    }

    for(i=0; i<num1; i++)
    {
        get_prog_at(i, &node);
        get_tp_by_id(node.tp_id, &tp);
        network_id[i]=tp.network_id;
        intensity[i]=tp.intensity;
        quality[i]=tp.quality;
        service_type[i] = node.service_type;
        hd_lcn[i] = node.hd_lcn;
        hd_lcn_ture[i] = node.hd_lcn_ture;
        service_id[i] = node.prog_number;
        lcn[i] = node.LCN;
        DBG_PRINTF("TP %d's prog: i %d, service id %x service type = 0x%x tp.freq = %d, intensity = %d \n",\
               tp.frq, i, node.prog_number,node.service_type, tp.frq, tp.intensity);
    }

    recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE, tp_id2);
    num2 = get_prog_num(VIEW_ALL|PROG_TVRADIO_MODE, 0);

    for(i=0; i<num1; i++)
    {
        DBG_PRINTF("\nProg %x has hd lcn, check with other...\n", service_id[i]);
        DBG_PRINTF(" service_type[%d]=%x\n", i,service_type[i]);

        UINT32 serv_type_val = 0x19;

        if(serv_type_val == service_type[i]) //hd service
        {//compare program node
            if(hd_lcn_ture[i]==TRUE)//with similcast lcn
            {
                DBG_PRINTF(" has similcast lcn \n");
                for(j=num2-1; j>=0; j--)//last
                {
                    get_prog_at(j, &node);//tp2  node
                    get_tp_by_id(node.tp_id, &tp);
                    //if((tp.quality<=quality[i])||(tp.intensity+5<=intensity[i]))//last <= first
                    DBG_PRINTF("\n compare with prog %x\n", node.prog_number);
                    DBG_PRINTF("\n prog %x 's node.hd_lcn_ture = %d, tp:frq = %d, intensity=%d; prog %x 's tp.intensity=%d\n",\
                           node.prog_number,node.hd_lcn_ture, tp.frq,tp.intensity,service_id[i], intensity[i]);

                    if((node.hd_lcn_ture==TRUE)&&(tp.intensity<=(intensity[i]+10)))//last <= first
                    {
                        DBG_PRINTF(" Oh, prog %x has hd_simucast_lcn, and it's signal < prog %x\n", node.prog_number, service_id[i]);
                        DBG_PRINTF(" prog %x 's network_id %x, prog %x 's network_id %x \n",\
                               node.prog_number, tp.network_id, service_id[i], network_id[i]);

                        if((tp.network_id==france_hd_network_id)&&(network_id[i]==france_hd_network_id))//France network
                        {
                            DBG_PRINTF("\n prog %x 's LCN %d , prog %x 's hd_simucast_lcn %d\n",\
                                   node.prog_number,node.LCN,service_id[i],hd_lcn[i]);

                            if(hd_lcn[i] == node.LCN)//tp1 compare tp2
                            {
                                DBG_PRINTF(" Oh, them equal!So here swap\n", node.prog_number, service_id[i]);
                                DBG_PRINTF(" Set prog %x's current's LCN = it's simucast LCN\n", node.prog_number);
                                DBG_PRINTF(" SWAP ,LCN from %d to %d\n", node.LCN, node.hd_lcn);
                                node.LCN=node.hd_lcn; // SD
                                modify_prog(node.prog_id,  &node);//sd view
                                update_data();
                                recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE, tp_id1);
                                get_prog_at(i, &node);
                                DBG_PRINTF(" Set prog %x's current's LCN = it's simucast LCN\n", node.prog_number);
                                DBG_PRINTF(" SWAP ,LCN from %d to %d\n", node.LCN, node.hd_lcn);
                                node.LCN =hd_lcn[i] ;//HD use simukcast
                                modify_prog(node.prog_id,  &node);//hd view
                                update_data();
                                recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE, tp_id2);
                            }
                        }
                        else
                        {
                            if(tp.network_id!=france_hd_network_id)
                            {
                                if (node.LCN<max_node_lcn)
                                {
                                    node.LCN =  find_dtg_variant_region_lcn();
                                    DBG_PRINTF(" Foreign ! tp.network_id=%x lcn %d\n", tp.network_id,node.LCN);
                                    modify_prog(node.prog_id,  &node);
                                    update_data();
                                }
                            }
                            else if(network_id[i]!=france_hd_network_id)
                            {

                                recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE, tp_id1);
                                get_prog_at(i, &node);
                                if (node.LCN<max_node_lcn)
                                {
                                    node.LCN =  find_dtg_variant_region_lcn();
                                    DBG_PRINTF(" Foreign ! network_id[%d]=%x lcn %d\n",i,network_id[i],node.LCN);
                                    modify_prog(node.prog_id,  &node);
                                    update_data();
                                    recreate_prog_view(VIEW_SINGLE_TP|PROG_TVRADIO_MODE, tp_id2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    FREE(service_id);
    FREE(lcn);
    FREE(hd_lcn);
    FREE(hd_lcn_ture);
    FREE(service_type);
    FREE(network_id);
    FREE(intensity);
    FREE(quality);
}

void tnt_search_ts_proc(UINT16 sat_id)
{
    INT32 i, j, k, t_num;
    UINT16 network_id, ts_id,intensity=0;
    UINT32 tp_id;
    T_NODE t_node;
    UINT8   channel_mode, db_changed = 0,quality=0;

    //reserve status
        channel_mode = sys_data_get_cur_chan_mode();

    t_num = get_tp_num_sat(sat_id);

    DBG_PRINTF("Let's Check all program's LCN:\n");
    DBG_PRINTF("\tCurrent DB has %d TPs\n", t_num);
    //compare all tp node
    for(j=0; j<t_num; j++)
    {
        get_tp_at(sat_id, j, &t_node);

        network_id = t_node.network_id;
        ts_id = t_node.t_s_id;
        tp_id = t_node.tp_id;
        DBG_PRINTF("\nCurrently check the [%d] tp with others...tp id = %d, ts id = %d freq = %d\n",
               j, tp_id, ts_id, t_node.frq);
        for(k=t_num-1; k>=0; k--)
        {
            if (k==j)
                continue;
            get_tp_at(sat_id, k, &t_node);
            DBG_PRINTF("\nCompare with [%d] tp tp id = %d ts id = %d  ", k, t_node.tp_id, t_node.t_s_id);
            DBG_PRINTF("freq %d intensity %d, quality %d\n",t_node.frq,t_node.intensity,t_node.quality );
            tnt_same_service_proc(tp_id,t_node.tp_id);
            update_data();
        }

    }

}
#endif
#ifdef POLAND_SPEC_SUPPORT
UINT32 get_chan_pos_indb(P_NODE *srch_pnode,UINT8 chan_mode)
{
    UINT16 prog_num=0,n=0,position=0;
    P_NODE p_node;
    MEMSET(&p_node, 0, sizeof(P_NODE));
    recreate_prog_view(VIEW_ALL | chan_mode, 0);
    prog_num = get_prog_num(VIEW_ALL | PROG_TVRADIO_MODE, 0);
    n=prog_num-1;

    if(prog_num > 0)
    {
        while(n >= 0)
        {
            if(get_prog_at(n, &p_node) == DB_SUCCES)
            {
                if(p_node.tp_id == srch_pnode->tp_id)
                {
                    position=n;
                    break;

                }
            }
            --n;
        }

    }
    return position;


}

#endif

#ifdef SHOW_WELCOME_FIRST
static struct as_service_param win_search_param;

void handle_post_close_when_none_poweroff_signal(POBJECT_HEAD p_obj, VEVENT event, UINT32 param1, UINT32 param2)
{
    //PRESULT ret = PROC_PASS;
    //UINT8 av_flag = 0;
    UINT16 cur_chan = 0;
    //UINT32 position=0;
    //UINT8 back_saved = 0;
    POBJECT_HEAD mainmenu = NULL;//modify for adding welcome page when only open dvbt2011 10 17
    //UINT16 title_id = 0;//modify for adding welcome page when only open dvbt 2011 10 17
    UINT32 tv_num = 0;
    UINT32 radio_num=0;

    osd_clear_object( (POBJECT_HEAD)&srch_progress_bar, C_UPDATE_ALL);
    osd_clear_object( (POBJECT_HEAD)&srch_progress_txt, C_UPDATE_ALL);

    MEMSET(&win_search_param,0,sizeof(win_search_param));
    get_search_param(&win_search_param);
    
    if(win_search_param.as_method == AS_METHOD_NIT_TP ||
        win_search_param.as_method == AS_METHOD_NIT||
        win_search_param.as_method == AS_METHOD_MULTI_NIT)
    {
        osd_set_objp_next(&srch_line_v,&srch_progress_bar);
    }

    mainmenu = (POBJECT_HEAD)&g_win_mainmenu;
    osd_track_object(mainmenu,C_UPDATE_ALL);
    //modify end
    //modify for adding welcome page when only open dvbt 2011 19 18
    start_tdt();
    api_set_system_state(SYS_STATE_NORMAL);

    get_search_prog_cnt(&tv_num,&radio_num);
    if(1 == g_enter_welcom)
    {
        if(( tv_num > 0 ) || ( radio_num > 0 ))
        {
            osd_clear_object((POBJECT_HEAD)&g_win_submenu, 0);
            osd_clear_object((POBJECT_HEAD)&g_win_mainmenu, 0);
            wincom_close_title();
            menu_stack_pop_all();
            cur_chan = sys_data_get_cur_group_cur_mode_channel();

            api_play_channel(cur_chan, TRUE, TRUE,FALSE);
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win2_progname, FALSE);

        }
        else
        {
            osd_clear_object((POBJECT_HEAD)&g_win_submenu, 0);
            osd_clear_object((POBJECT_HEAD)&g_win_mainmenu, 0);
            wincom_close_title();
            menu_stack_pop_all();
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&g_win_mainmenu, FALSE);
        }
    }
#ifdef SUPPORT_FRANCE_HD
    else if(g_enter_welcom==(1 + 1))
    {
        osd_clear_screen();
        menu_stack_pop_all();
        cur_chan = sys_data_get_cur_group_cur_mode_channel();
        api_play_channel(cur_chan, TRUE, TRUE,FALSE);
        ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (POBJECT_HEAD)&g_win2_progname, FALSE);
    }
#endif

}
#endif//#ifdef SHOW_WELCOME_FIRST

