 /*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: sfu_test.c
*
*    Description:   This file contains all functions definition SFU test command functions
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <basic_types.h>
#include <api/libc/string.h>
#include <api/libnim/lib_nim.h>
#ifndef NEW_DEMO_FRAME
#include <api/libpub29/lib_pub27.h>
#else
#include <api/libpub/lib_pub.h>
#endif
#include <hld/dmx/dmx.h>
#include <bus/tsi/tsi.h>
#include "lib_ash.h"
#include "control.h"
#include <hld/sto/sto_dev.h>
#ifdef SFU_TEST_SUPPORT
#include "sfu_test.h"
#endif
#include "platform/board.h"
#include "power.h"
#ifdef _DEBUG_VERSION_
#define DEBUG
#define SH_PRINTF   libc_printf
#else
#define SH_PRINTF(...)
#endif

#ifdef SFU_TEST_SUPPORT

/*************************************************************************/

static UINT32 m_freq = 0;
static __MAYBE_UNUSED__ UINT32 m_bandwidth = 0;
static UINT32 m_sym = 0;
static UINT8 m_constellation = 0;
static UINT32 m_nim_id = 0;
static BOOL m_turn_on_sfutesting = FALSE;
static UINT32 m_auto_repeat_interval_ms = 0;
static ID sfu_auto_get_current_status_task_id = OSAL_INVALID_ID;
static BOOL m_pause_auto_get_current_status_task = FALSE;
static BOOL m_exit_task = FALSE;
static BOOL m_reset_dmx_per = 0;

static int set_nim(unsigned int argc, unsigned char * argv []);

void sci_echo(char *str)
{
    //UINT32 len = ComStrLen(str);
    UINT32 len = STRLEN(str);
    UINT32 i = 0;

    for (i=0; i<len; ++i)
    {
        LIB_ASH_OC(str[i]);
    }
}

/*************************************************************************/
#ifdef DVBS_SUPPORT

/*sfu for dvb-s*/
//static UINT32 nim_id = 0;
#define SSI_GAP_TIMEOUT 30000
static int set_tp_s(unsigned int argc, unsigned char *argv[]);
static int get_lock(unsigned int argc, unsigned char *argv[]);
static int get_ber(unsigned int argc, unsigned char *argv[]);
static int get_per(unsigned int argc, unsigned char *argv[]);
static int int_printf(unsigned int src);
static void reset_perflag();
static int set_ssiclk(unsigned int argc, unsigned char *argv[]);
static int set_ssigap(unsigned int argc, unsigned char *argv[]);
static int get_dmx_per_s(unsigned int argc, unsigned char * argv []);
static int set_spissiselect(unsigned int argc, unsigned char * argv []);
/* dvb-s Command <-> function map list */

struct ash_cmd sfutest_command_s[] =
{
    {"settuner",set_nim},
    {"settp", set_tp_s},
    {"getlock", get_lock},
    {"getber", get_ber},
    {"getper", get_per},
    {"set_ssi_gap",set_ssigap},
    {"set_ssi_clock",set_ssiclk},
    {"getper_dmx",get_dmx_per_s},
    {"set_ssi_spi_select",set_spissiselect},
    {NULL, NULL}
};

/*sfu test init*/
void sfutest_init_s()
{
    ash_cm_register_sfu_test(sfutest_command_s);
    //unsigned char *set_nim[]={"settuner","1"};
    //SetNim(2,set_nim);
    sci_mode_set(0,115200,1);
    m_turn_on_sfutesting = TRUE;
    key_set_upgrade_check_flag(0);
    ash_task_init();
    sci_echo("SFU(s) testing start.\r\n");
}

/*************************************************************************/

static int set_tp_s(unsigned int argc, unsigned char *argv[])
{
    UINT32 freq = 0;
    UINT32 sym  = 0;
    UINT16 channel = 0;

    T_NODE t_node;
    P_NODE p_node;

    freq = ATOI(argv[1]);
    sym = ATOI(argv[2]);

    reset_group();
    channel = sys_data_get_cur_group_cur_mode_channel();
    get_prog_at(channel,&p_node);
    get_tp_by_id(p_node.tp_id, &t_node);
    recreate_tp_view(VIEW_SINGLE_SAT, t_node.sat_id);
	if (3 != argc)
    {
		if(4 == argc)
		{
			if (0 == STRCMP((const char *)"vcm", (const char *)argv[3]) )
			{
				t_node.plp_id = 1;// ft->xpond.s_info.isid = t_node.plp_id; nim_isid.isid_write = xpond->s_info.isid;
				t_node.t2_profile = 1;
			}
			else
			{
				SH_PRINTF("Usage: SetPara <freq> <sym>\n");
        		return -1;
			}
		}
		else
		{
			if(5 == argc)
			{
				if (0 == STRCMP((const char *)"ccm", (const char *)argv[3]) )
				{
					if (0 == STRCMP((const char *)"wb", (const char *)argv[4]) )
					{
						t_node.tsn_id = 1;// ft->xpond.s_info.tsn_id = t_node.tsn_id; nim_tsn.tsn_write = xpond->s_info.tsn_id;
						t_node.tsn_profile = 1;
					}
					else
					{
						SH_PRINTF("Usage: SetPara <freq> <sym>\n");
						return -1;
					}
				}
				else
				{
			 		if (0 == STRCMP((const char *)"vcm", (const char *)argv[3]) ) 
			 		{
			 			t_node.plp_id = 1;// ft->xpond.s_info.isid = t_node.plp_id; nim_isid.isid_write = xpond->s_info.isid;
			 			t_node.t2_profile = 1;
			 		}
					else
					{
						SH_PRINTF("Usage: SetPara <freq> <sym>\n");
						return -1;
					}
					if (0 == STRCMP((const char *)"wb", (const char *)argv[4]) )
					{
						t_node.tsn_id = 1;// ft->xpond.s_info.tsn_id = t_node.tsn_id; nim_tsn.tsn_write = xpond->s_info.tsn_id;
						t_node.tsn_profile = 1;
					}
					else
					{
						SH_PRINTF("Usage: SetPara <freq> <sym>\n");
						return -1;
					}
				}
			}
			else
			{
				SH_PRINTF("Usage: SetPara <freq> <sym>\n");
				return -1;
			}
		}

    }
	
    t_node.frq = freq;
    t_node.sym = sym;

    modify_tp(t_node.tp_id,&t_node);
    api_play_channel(p_node.prog_id, TRUE, FALSE, TRUE);

    LIB_ASH_OC('S');//sci_write(SCI_FOR_RS232, a)
    LIB_ASH_OC('U');
    LIB_ASH_OC('C');
    LIB_ASH_OC('C');
    LIB_ASH_OC('E');
    LIB_ASH_OC('S');
    LIB_ASH_OC('S');

    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');
    reset_perflag();
    return 0;
}

static int get_lock(unsigned int argc, unsigned char *argv[])
{
    INT32 ret = -1;
    UINT8 lock = 0;

    if (1 != argc)
    {
        SH_PRINTF("Usage: GetLock\n");
        return -1;
    }

    struct nim_device *nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);

    if ((ret = nim_get_lock(nim_dev, &lock)) != SUCCESS)
    {
        SH_PRINTF("api_nim_get_lock: operation error! %d\n", ret);
        return -1;
    }

    if(!lock)
    {
         LIB_ASH_OC('U');
         LIB_ASH_OC('N');
    }
         LIB_ASH_OC('L');
         LIB_ASH_OC('O');
         LIB_ASH_OC('C');
         LIB_ASH_OC('K');

    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');
}

static int get_ber(unsigned int argc, unsigned char *argv[])
{
    INT32  ret = -1;
    UINT32 ber = 0;

    if (1 != argc)
    {
        SH_PRINTF("Usage: GetBER\n");
        return -1;
    }

    struct nim_device *nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);

    if(NULL == nim_dev->do_ioctl)
    {
        return -1;
    }

    ret = nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_BER, (UINT32)&ber);
    if (ret != SUCCESS)
    {
        SH_PRINTF("api_nim_get_BER: operation error! %d\n", ret);
        return -1;
    }

    int_printf(ber);

    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');
}
/*
static int GetPER(unsigned int argc, unsigned char *argv[])
{
    INT32 ret;
    UINT32 per;

    if (argc != 1)
    {
        SH_PRINTF("Usage: GetPER\n");
        return -1;
    }

    struct nim_device *nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);

    if ((ret = nim_io_control(nim_dev, NIM_DRIVER_READ_RSUB, &per)) != SUCCESS)
    {
        if(ret != ERR_TIME_OUT)
        {
            SH_PRINTF("api_nim_get_PER: operation error! %d\n", ret);
            return -1;
        }
        else
        {
            per = 0;
        }
    }


    IntPrintf(per);

    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');
}

static int GetPER(unsigned int argc, unsigned char *argv[])
{
    INT32 ret;
    UINT32 per; (argc != 1)
    UINT16 count = 100 ;
    if
    {
        SH_PRINTF("Usage: GetPER\n");
        return -1;
    }

    struct nim_device *nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);

do{

    if(count == 0)
    {
        SH_PRINTF("api_nim_get_PER: operation error! %d\n", ret);
        return -1;
    }
    else
    {
        osal_delay(150);
        count -- ;
        if((ret = nim_io_control(nim_dev, NIM_DRIVER_READ_RSUB, &per)) == SUCCESS)
                IntPrintf(per);
            break;
    }
}while((ret = nim_io_control(nim_dev, NIM_DRIVER_READ_RSUB, &per)) == ERR_PARA);

   LIB_ASH_OC('\r');
   LIB_ASH_OC('\n');
}
 */

 static int get_per(unsigned int argc, unsigned char *argv[])
{
    INT32  ret = -1;
    UINT32 per = 0;
    UINT8  i   = 0;
    struct nim_device *nim_dev = NULL;

    if (1 != argc)
    {
        SH_PRINTF("Usage: GetPER\n");
        return -1;
    }

    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);

    for(i = 0; i < 30; i++)
    {
        osal_delay(500);
        ret = nim_io_control(nim_dev, NIM_DRIVER_READ_RSUB, &per);
        if(ret == SUCCESS)
        {
            int_printf(per);
            break;
        }
        else if(ret == ERR_PARA)
        {
            if(29 == i)
            {
                per = 100100;
                int_printf(per);
                break;
            }
            else
            {
                continue;
            }
        }
        else
        {
            SH_PRINTF("api_nim_get_PER: operation error! %d\n", ret);
            return -1;
        }
    }

    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');
}

static int set_ssiclk(unsigned int argc, unsigned char *argv[])
{
    UINT32 clk = 0;
    struct nim_device *nim_dev = NULL;

    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);

    if (2 != argc)
    {
        SH_PRINTF("Usage: SetPara <gap%d>\n");
        return -1;
    }
    else if(NULL == nim_dev)
    {
        SH_PRINTF("Nim1 status error\n");
        return -2;
    }

    clk = ATOI(argv[1]);
    nim_io_control(nim_dev, NIM_DRIVER_SET_SSI_CLK, clk);

    LIB_ASH_OC('S');
    LIB_ASH_OC('e');
    LIB_ASH_OC('t');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('S');       
    LIB_ASH_OC('S');
    LIB_ASH_OC('I');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('C');
    LIB_ASH_OC('l');
    LIB_ASH_OC('o');
    LIB_ASH_OC('c');
    LIB_ASH_OC('k');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('t');
    LIB_ASH_OC('o');
    LIB_ASH_OC(' ');
    LIB_ASH_OC(((UINT8)(clk/10)+'0'));
    LIB_ASH_OC(((UINT8)(clk%10)+'0'));
    LIB_ASH_OC('M');

    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');

    reset_perflag();
    return 0;
}

static int set_ssigap(unsigned int argc, unsigned char *argv[])
{
    UINT32 gap = 0;
    struct nim_device *nim_dev = NULL;

    nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);

    if (2 != argc)
    {
        SH_PRINTF("Usage: SetPara <clk>\n");
        return -1;
    }
    else if(nim_dev == NULL)
    {
        SH_PRINTF("Nim1 status error\n");
        return -1;
    }

    if(strcasecmp("gap",argv[1]))
    {
        gap = ATOI(&(argv[1][3]));
        nim_io_control(nim_dev, NIM_DRIVER_CHANGE_TS_GAP, gap);
    }
    else
    {
        SH_PRINTF("parameter error\n");
        return -3;
    }

#if 1//path for lock delay
    UINT8   lock = 0;
    UINT32 pre_tick = osal_get_tick();

    while((osal_get_tick()-pre_tick)>=SSI_GAP_TIMEOUT)
    {
        nim_get_lock(nim_dev,&lock);
        if(lock)
        {
            break;
        }
        osal_delay(30);
    }
#endif

    LIB_ASH_OC('S');
    LIB_ASH_OC('e');
    LIB_ASH_OC('t');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('S');
    LIB_ASH_OC('S');
    LIB_ASH_OC('I');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('G');
    LIB_ASH_OC('a');
    LIB_ASH_OC('p');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('t');
    LIB_ASH_OC('o');
    LIB_ASH_OC(' ');
    LIB_ASH_OC('g');
    LIB_ASH_OC('a');
    LIB_ASH_OC('p');
    if(gap < 10)
    {
        LIB_ASH_OC((gap + '0'));
    }
    else
    {
        LIB_ASH_OC(((UINT8)gap/10 + '0'));
        LIB_ASH_OC(((UINT8)gap%10 + '0'));
    }

    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');

    reset_perflag();
    return 0;
}

static UINT32 bperflag = 0;
static void reset_perflag()
{
    bperflag = 0;
}

static int get_dmx_per_s(unsigned int argc, unsigned char * argv [ ])
{
    INT32 ret=0;
    UINT32 per = 0, packet_count_after = 0;
	static UINT32 packet_count_before = 0,packet_count_indi_err_before = 0;
	UINT32 demod_per = 0,packet_count_indi_err_after = 0;
#if 1
    if (1 != argc)
    {
        SH_PRINTF("Usage: GetDmxPER\n");

        return -1;
    }
    struct dmx_device *dmx_dev = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);//DMX0 is used to live play always
	struct nim_device *nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);


	if(NULL != nim_dev)
	{
		nim_io_control(nim_dev, NIM_DRIVER_READ_RSUB,&demod_per);
	
	}
	
    if(NULL != dmx_dev)
    {
        //struct dmx_private *prv = (struct dmx_private *)dmx_dev->priv;
        if(0 != bperflag)
        {
            dmx_io_control(dmx_dev,IO_GET_DISCONTINUE_COUNT,(UINT32)&per);//prv->disconti_counter;
            //In order to avoid this situation: 
            //Tuner had locked the signal, but no data package can passed to the DMX, because the quality is too bad.
            //'disconti_counter' of DMX driver has not to be counted at this time.
            //So 'getper_dmx' command will return result 'LOCK 0 0'.
            //It will make the SFU auto testing procedure confuse.
            if(0 == per)
            {          
            	packet_count_after = 0;
                dmx_io_control(dmx_dev, IO_DMX_GET_PACKET_NUM, (UINT32)&packet_count_after);                
				
                if(packet_count_before == packet_count_after)
                {
                    per = 2000000;
                }
				else
				{
					dmx_io_control(dmx_dev, DMX_GET_STREAM_ERR_INFO_INDI_ERR , (UINT32)&packet_count_indi_err_after);
					if(packet_count_indi_err_after != packet_count_indi_err_before)
					{
						 per = 2000000;
					}
					else
					{
						/*
						if(demod_per > 10)
						{
							per = demod_per;
						}
						*/
					}
				}
            }
        }
        else
        {
            per = 0;
            bperflag = 1;
        }
        //prv->disconti_counter = 0;
        dmx_io_control(dmx_dev,IO_CLEAR_DISCONTINUE_COUNT,0);

		packet_count_before = 0;
		packet_count_indi_err_before = 0;
		dmx_io_control(dmx_dev, IO_DMX_GET_PACKET_NUM, (UINT32)&packet_count_before);
		dmx_io_control(dmx_dev, DMX_GET_STREAM_ERR_INFO_INDI_ERR , (UINT32)&packet_count_indi_err_before);
    }
    else
    {
        SH_PRINTF("GetDmxPER: operation error! %d\n", ret);
        return -1;
    }

    int_printf(per);
    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');
    #endif
    return ret;
}

static int set_spissiselect(unsigned int argc, unsigned char * argv[])
{
    extern UINT32 nim_s3501_set_ts_output_mode(struct nim_device *dev,UINT32 ts_output_mode);

    struct dmx_device *dmx_dev = dev_get_by_id(HLD_DEV_TYPE_DMX, m_nim_id);

    if (2 != argc)
    {
        SH_PRINTF("Usage: SetPara <nim_id> \n");
        return -1;
    }
    if(strcasecmp("SSI",argv[1]))
    {
        //nim_id  = 0;
        //nim_s3501_set_ts_output_mode(dmx_dev,1);

        *((volatile UINT32 *)0xb8000088) = *((volatile UINT32 *)0xb8000088)& (~(0x40000000));
        osal_task_sleep(10);
        tsi_select(TSI_DMX_0, TSI_SSI_0);
        tsi_select(TSI_DMX_1, TSI_SSI_0);
    }
    else
    {
        //nim_id = 1;
        //nim_s3501_set_ts_output_mode(dmx_dev,0);

        *((volatile UINT32 *)0xb8000088) = *((volatile UINT32 *)0xb8000088)& (~(0x60000000));
        osal_task_sleep(10);
        tsi_select(TSI_DMX_0, TSI_SPI_1);
        tsi_select(TSI_DMX_1, TSI_SPI_1);
    }
    //SH_PRINTF("%s ",argv[1]);
    LIB_ASH_OC('O');
    LIB_ASH_OC('K');

    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');
}

#endif

/**************************************************************************/
#if (defined(DVBT_SUPPORT) || defined(ISDBT_SUPPORT))

/* sfu functions for dvb-t*/
BOOL bootup_checking_for_sfutesting();
BOOL sfutest_init_t();
void sfutesting_stop_t();
//static void SciEcho(char *str);
void show_sfutesting_help(char *str);

static int set_tp_t(unsigned int argc, unsigned char *argv[]);
static int do_set_tp_t(UINT32 central_freq, UINT32 bandwidth);
static int sfu_reboot_t(unsigned int argc, unsigned char *argv[]);
static int sfu_stop_t(unsigned int argc, unsigned char *argv[]);

static void sfu_auto_get_current_status_t();
static BOOL sfu_auto_get_current_status_task_start_t();
static void sfu_auto_get_current_status_task_delete_t();
static int sfu_get_current_status_t(unsigned int argc, unsigned char *argv[]);
static int get_dmx_per_t(unsigned int argc, unsigned char * argv []);
void set_sfutesting_parameter_t(UINT32 freq, UINT8 bandwidth,UINT32 nimid);
// dvb-t Command <-> function map list.
struct ash_cmd sfutest_command_t[] =
{
    {"settuner",set_nim},
    {"settp", set_tp_t},
    {"getcs", sfu_get_current_status_t},
    {"reboot", sfu_reboot_t},
    {"getper_dmx", get_dmx_per_t},
    {"stop", sfu_stop_t},
    {NULL, NULL}
};

static BOOL skip_all_pressed_key_proc(control_msg_t *p_msg)
{
    return TRUE;
}

void set_sfutesting_parameter_t(UINT32 freq, UINT8 bandwidth, UINT32 nim_id)//, UINT32 nim_id)
{
    m_freq = freq;
    m_bandwidth = bandwidth;
    m_nim_id=nim_id;
}

BOOL sfutest_init_t()
{
    P_NODE p_node;
    T_NODE t_node;
    SYSTEM_DATA *sys_data = NULL;
    sys_data = sys_data_get();
    sci_echo("Enter SFU auto test.\r\n");

    if(get_prog_num(VIEW_ALL|sys_data->cur_chan_mode[0], 0))
    {
        if (get_prog_at(sys_data_get_cur_group_cur_mode_channel(),&p_node) == SUCCESS)
        {
            get_tp_by_id(p_node.tp_id, &t_node);
            m_freq = t_node.frq;
            m_bandwidth = t_node.bandwidth;
        }
    }
    ash_cm_register_sfu_test(sfutest_command_t);
    //unsigned char *set_nim[]={"settuner","2"};
    //SetNim(2,set_nim);
    ash_task_init();
    m_turn_on_sfutesting = TRUE;
    sci_echo("SFU testing start.\r\n");

    return TRUE;
}

static BOOL sfuchannel_change_t(UINT32 sfreq, UINT16 bandwidth)
{
    T_NODE t_node;
    UINT32 center_freq=0;
    center_freq = sfreq + ((bandwidth*1000)>>1);
    union ft_xpond xponder;
    struct nim_device *nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, sys_data_get_sfu_nim_id());
    xponder.t_info.type = FRONTEND_TYPE_T;
    xponder.t_info.frq = center_freq;
    xponder.t_info.band_width = bandwidth*1000;
#if defined(DVBT2_SUPPORT)
    xponder.t_info.usage_type = (UINT8)USAGE_TYPE_AERIALTUNE;
#else
    xponder.t_info.usage_type = (UINT8)USAGE_TYPE_CHANSCAN;
#endif
    api_nim_reset_autoscan();       //50929 yuchun

    frontend_tuning(nim_dev, NULL, &xponder, 1);

    return TRUE;
}

void sfutesting_stop_t()
{
    P_NODE p_node;
    T_NODE t_node;
    SYSTEM_DATA *sys_data = NULL;
    sys_data = sys_data_get();

    sfu_ash_task_delete();
    sfu_auto_get_current_status_task_delete_t();
    m_freq = 0;
    m_bandwidth = 0;

    if(get_prog_num(VIEW_ALL|sys_data->cur_chan_mode[0], 0))
    {
        if (get_prog_at(sys_data_get_cur_group_cur_mode_channel(),&p_node) == SUCCESS)
        {
            get_tp_by_id(p_node.tp_id, &t_node);
            sfuchannel_change_t(t_node.frq - t_node.bandwidth*1000/2, t_node.bandwidth);
        }
    }

    m_turn_on_sfutesting = FALSE;
    sci_echo("SFU testing stop.\r\n");
}

void show_sfutesting_help(char *str)
{
    UINT32 i = 0;
    sci_echo("SFU testing commands:\r\n");
    for (i=0; sfutest_command_t[i].command != NULL; ++i)
    {
        sci_echo(sfutest_command_t[i].command);
        sci_echo("\r\n");
    }
}

static int set_tp_t(unsigned int argc, unsigned char *argv[])
{
    UINT32 freq = 0;    //in KHz.
    UINT32 bandwidth = 0;//must be 6/7/8 MHz.
    UINT16 channel = 0;
    T_NODE t_node;
    P_NODE p_node;

    MEMSET(&t_node, 0, sizeof(T_NODE));
    MEMSET(&p_node, 0, sizeof(P_NODE));

    if (3 != argc)
    {
        sci_echo("Usage: settp <Central_Frequence_KHz> <BandWidth_MHz>\r\n");
        sci_echo("Central_Frequence_KHz : 47000 ~ 890000, BandWidth_MHz : 6/7/8\r\n");
        return -1;
    }

    freq = ATOI(argv[1]);
    bandwidth = ATOI(argv[2]);

    do_set_tp_t(freq, bandwidth);
    return 0;
}

static int do_set_tp_t(UINT32 central_freq, UINT32 bandwidth)
{
    BOOL pause_status_bak;
    if ((central_freq < 47000) || (central_freq > 890000) || (bandwidth != 6 && bandwidth != 7 && bandwidth != 8) )
    {
        sci_echo("Usage: settp <Central_Frequence_KHz> <BandWidth_MHz>\r\n");
        sci_echo("Central_Frequence_KHz : 47000 ~ 890000, BandWidth_MHz : 6/7/8\r\n");
        return -1;
    }

    m_freq = central_freq;
    m_bandwidth = bandwidth;

    pause_status_bak = m_pause_auto_get_current_status_task;
    m_pause_auto_get_current_status_task = TRUE;
    //osal_delay(50); //wait for finish the output of the previous command.
    osal_delay(100); //wait for finish the output of the previous command.

    //make the frequence start point from the frequence central point.
    sfuchannel_change_t(central_freq - bandwidth*1000/2, bandwidth);
    osal_delay(1000);
    sci_echo("SUCCESS\r\n");

    m_pause_auto_get_current_status_task = pause_status_bak;
    m_reset_dmx_per = TRUE;
    return 0;
}

static int sfu_reboot_t(unsigned int argc, unsigned char *argv[])
{
    P_NODE p_node;
    T_NODE t_node;
    SYSTEM_DATA *sys_data = sys_data_get();

    if (1 != argc)
    {
        sci_echo("Usage: reboot\r\n");
        return -1;
    }

    //UIChChgStopProg(TRUE);
    api_stop_play(0);
    m_pause_auto_get_current_status_task = TRUE;

    //for change the frequency of current channel as the SFU testing frequency.
    if(get_prog_num(VIEW_ALL|sys_data->cur_chan_mode[0], 0))
    {
        if (get_prog_at(sys_data_get_cur_group_cur_mode_channel(),&p_node) == SUCCESS)
        {
            get_tp_by_id(p_node.tp_id, &t_node);

            t_node.frq = m_freq;
            t_node.bandwidth = m_bandwidth;
            recreate_tp_view(VIEW_SINGLE_SAT, 1);
            if (modify_tp(t_node.tp_id, &t_node) != SUCCESS )
            {
                sci_echo("Err: Fail to modify the tp of program.\r\n");
                return -1;
            }
            if (update_data() != SUCCESS)
            {
                 sci_echo("Err: Fail to modify the tp of program.\r\n");
                 return -1;
            }

#if 0
        if (lookup_node(TYPE_TP_NODE, &t_node,0)!=SUCCESS)
        {
            if ((UINT32)add_node(TYPE_TP_NODE, t_node.tp_id,&t_node)==(UINT32)STORE_SPACE_FULL)
                    {
                sci_echo("Err: Database is full.\r\n");
                        return -1;
            }
                  if (update_data() != SUCCESS)
                  {
                sci_echo("Err: Fail to add the tp.\r\n");
                        return -1;
            }
        }

            if (p_node.tp_id != t_node.tp_id)
            {
                p_node.tp_id = t_node.tp_id;
                if ( modify_prog(sys_data_get_cur_group_cur_mode_channel(), &p_node) != SUCCESS )
                {
                    sci_echo("Err: Fail to modify the tp of program.\r\n");
                    return -1;
                }
                if (update_data() != SUCCESS)
                {
                    sci_echo("Err: Fail to modify the tp of program.\r\n");
                    return -1;
                }
            }
#endif
        }
    }
    osal_interrupt_disable();

    sys_data_set_sfu_frequency_t(m_freq);
    sys_data_set_sfu_bandwidth_t(m_bandwidth);
    sys_data_set_sfu_nim_id_t(sys_data_get_sfu_nim_id());
    sys_data_save(0);
    osal_delay(100);

    nim_close(g_nim_dev);
    if(g_nim_dev2)
    {
        nim_close(g_nim_dev2);//add nim_close before watchdog_reboot to fix c3811-isdbt reboot can't succeed
    }

    sfu_ash_task_delete();
    sfu_auto_get_current_status_task_delete_t();

    board_power(FALSE);
    sys_watchdog_reboot();

    return 0;
}

static int sfu_stop_t(unsigned int argc, unsigned char *argv[])
{
    if (1 != argc)
    {
        sci_echo("Usage: stop\r\n");
        return -1;
    }
    sfutesting_stop_t();

    return 0;
}

//================================================
static void sfu_auto_get_current_status_t()
{
    unsigned int i = 0;
    nim_rec_performance_t status;

    struct nim_device *nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, sys_data_get_sfu_nim_id());
    while (!m_exit_task)
    {
        status.valid = FALSE;
        while (!status.valid && !m_exit_task)
        {
            if (m_pause_auto_get_current_status_task)
            {
                break;
            }
            if ( nim_io_control(nim_dev, NIM_DRIVER_GET_REC_PERFORMANCE_INFO, (UINT32)&status) != SUCCESS )
            {
                SH_PRINTF("%s: fail.\n", __FUNCTION__);
                break;
            }
            if ( (0 == status.lock) || (0xff == status.lock))
            {
                status.valid = FALSE;
                status.ber = 0;
                status.per = 0;
                break;
            }
            osal_task_sleep(1);
        }
        if(m_exit_task && !status.valid)
        {
            break;
        }
        if (m_pause_auto_get_current_status_task)
        {
            continue;
        }
        if( (0 == status.lock) || (0xff == status.lock))
        {
            sci_echo("UNLOCK ");
        }
        else
        {
            sci_echo("LOCK ");
        }
        int_printf(status.ber);
        sci_echo(" "); 
        int_printf(status.per);
        sci_echo("\r\n");

        if(m_exit_task)
        {
            break;
        }
        if (0 == m_auto_repeat_interval_ms)   //run 1 time.
        {
            break;
        }
        //m_AutoRepeatInterval_ms--;
        osal_task_sleep(m_auto_repeat_interval_ms);
    }
    sfu_auto_get_current_status_task_id = OSAL_INVALID_ID;
}

static BOOL sfu_auto_get_current_status_task_start_t()
{
    OSAL_T_CTSK     t_ctsk;
    m_exit_task     = FALSE;

    t_ctsk.stksz    = 0x1000;
    t_ctsk.quantum  = 10;
    t_ctsk.itskpri  = OSAL_PRI_NORMAL;
    t_ctsk.name[0]  = 'G';
    t_ctsk.name[1]  = 'C';
    t_ctsk.name[2]  = 'S';
    t_ctsk.task = (FP)sfu_auto_get_current_status_t;
    sfu_auto_get_current_status_task_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == sfu_auto_get_current_status_task_id)
    {
        PRINTF("Fail to create sfu_auto_get_current_status_task_id task\n");
        return FALSE;
    }

    return TRUE;
}

static void sfu_auto_get_current_status_task_delete_t()
{
    if(OSAL_INVALID_ID != sfu_auto_get_current_status_task_id)
    {
        m_pause_auto_get_current_status_task = TRUE;
        osal_delay(50); //wait for finish the output.

        osal_task_delete(sfu_auto_get_current_status_task_id);
        m_exit_task = TRUE;
        sfu_auto_get_current_status_task_id = OSAL_INVALID_ID;
    }
}

//================================================

static int sfu_get_current_status_t(unsigned int argc, unsigned char *argv[])
{
    INT32 ret;
    UINT32 auto_repeat_interval_ms = 0;

    if (2 < argc)
    {
        sci_echo("Usage: getcs [AutoRepeatInterval_ms]\r\n");
        return -1;
    }

    if (2 == argc)
    {
        auto_repeat_interval_ms = ATOI(argv[1]);
    }
    m_exit_task = TRUE;
    while(sfu_auto_get_current_status_task_id!=OSAL_INVALID_ID)
    osal_task_sleep(1);

    sfu_auto_get_current_status_task_delete_t();  //clean up the previous command first.

    m_auto_repeat_interval_ms = auto_repeat_interval_ms;
    m_pause_auto_get_current_status_task = FALSE;
    sfu_auto_get_current_status_task_start_t();
}

static int get_dmx_per_t(unsigned int argc, unsigned char * argv[])
{
    INT32  ret = 0;
    UINT32 per = 0, packet_count_before = 0, packet_count_after = 0;
    UINT8 lock = 0;
    UINT8 snr  = 0;
    UINT8 agc  = 0;
    UINT8 quality = 0;

    if (1 != argc)
    {
        SH_PRINTF("Usage: getper_dmx\n");
        return -1;
    }
    struct nim_device *nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, sys_data_get_sfu_nim_id());
    if(NULL == nim_dev)
    {
        SH_PRINTF("GetDmxPER: nim_dev null!\n");
        return -1;
    }
    nim_get_lock(nim_dev, &lock);

    if(lock)
    {
        //In order to avoid this situation:
        //Tuner had locked the signal, but no data package can passed to the DMX, because the quality is too bad.
        //'disconti_counter' of DMX driver has not to be counted at this time.
        //So 'getper_dmx' command will return result 'LOCK 0 0'.
        //It will make the SFU auto testing procedure confuse.
        nim_get_snr(nim_dev, &quality);
        if (0 == quality)
        {
            lock = 0;   //If quality is 0, then it shall be treat as Unlock.
        }
    }

    //In order to avoid this situation:
    //Tuner had locked the signal, but no data package can passed to the DMX, because the quality is too bad.
    //'disconti_counter' of DMX driver has not to be counted at this time.
    //So 'getper_dmx' command will return result 'LOCK 0 0'.
    //It will make the SFU auto testing procedure confuse.
    if (nim_get_snr(nim_dev, &snr) != SUCCESS)
    {
      if(NULL != nim_dev->do_ioctl)
      {
            nim_dev->do_ioctl(nim_dev, NIM_DRIVER_GET_AGC, (UINT32)&agc);
      }
    }
    if ((0 == snr)&&(0 == agc))
    {
        lock = 0;   //If snr and agc is 0, then it shall be treat as Unlock.
    }
    if(lock)
    {
        struct dmx_device *dmx_dev = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
        if(NULL == dmx_dev)
        {
            SH_PRINTF("GetDmxPER: dmx_dev null!\n");
            return -1;
        }

        //struct dmx_private *prv = (struct dmx_private *)dmx_dev->priv;
        if(!m_reset_dmx_per)
        {
            dmx_io_control(dmx_dev, IO_GET_DISCONTINUE_COUNT, (UINT32)&per);//prv->disconti_counter;
            //In order to avoid this situation: 
            //Tuner had locked the signal, but no data package can passed to the DMX, because the quality is too bad.
            //'disconti_counter' of DMX driver has not to be counted at this time.
            //So 'getper_dmx' command will return result 'LOCK 0 0'.
            //It will make the SFU auto testing procedure confuse.
            if(0 == per)
            {
                dmx_io_control(dmx_dev, IO_DMX_GET_PACKET_NUM, (UINT32)&packet_count_before);
                //IntPrintf(packet_count_before);
                osal_delay(100000);
                //SciEcho(" ");
                dmx_io_control(dmx_dev, IO_DMX_GET_PACKET_NUM, (UINT32)&packet_count_after);
                //IntPrintf(packet_count_after);                SciEcho(" ");
                if(packet_count_before == packet_count_after)
                {
                    per = 1000000;
                }
            }
        }
        else
        {
            per = 0;
            m_reset_dmx_per = FALSE;
        }
        //prv->disconti_counter = 0;
        dmx_io_control(dmx_dev, IO_CLEAR_DISCONTINUE_COUNT, 0);
    }

    if(!lock)
    {
        sci_echo("UNLOCK ");
    }
    else
    {
        sci_echo("LOCK ");
    }
    int_printf(0); // Don't need BER any more, always return 0
    sci_echo(" ");
    int_printf(per);
    sci_echo("\r\n");

    return ret;
}
#endif

#ifdef DVBC_SUPPORT

/*sfu for dvb-c*/

static UINT32 bperflag = 0;

void set_sfutesting_parameter_c(UINT32 freq, UINT8 constellation,UINT32 sym,UINT8 nimid);
BOOL sfutest_init_c();
void sfutesting_stop_c();
//void SciEcho(char *str);
void show_sfutesting_help(char *str);
static int int_printf(unsigned int src);
static void reset_perflag();
static int set_tp_c(unsigned int argc, unsigned char *argv[]);
static int do_set_tp_c(UINT32 central_freq, UINT8 constellation,UINT32 symbol);
static int sfu_reboot_c(unsigned int argc, unsigned char *argv[]);
static int sfu_stop_c(unsigned int argc, unsigned char *argv[]);

static void sfu_auto_get_current_status_c();
static BOOL sfu_auto_get_current_status_task_start_c();
static void sfu_auto_get_current_status_task_delete_c();
static int sfu_get_current_status_c(unsigned int argc, unsigned char *argv[]);
static int get_dmx_per_c(unsigned int argc, unsigned char * argv []);
// dvb-t Command <-> function map list.
struct ash_cmd sfutest_command_c[] =
{
    {"settuner",set_nim},
    {"settp", set_tp_c},
    {"getcs", sfu_get_current_status_c},
    {"reboot", sfu_reboot_c},
    {"getper_dmx", get_dmx_per_c},
    {"stop", sfu_stop_c},
    {NULL, NULL}
};

/*************************************************************************/
/* sfu functions for dvb-c*/

static void reset_perflag()
{
    bperflag = 0;
}

static BOOL skip_all_pressed_key_proc(control_msg_t *p_msg)
{
    return TRUE;
}

void set_sfutesting_parameter_c(UINT32 freq, UINT8 constellation,UINT32 sym,UINT8 nim_id)
{
    m_freq =freq;
    m_constellation = constellation;
    m_sym = sym;
    m_nim_id=nim_id;
}

BOOL sfutest_init_c()
{
    P_NODE p_node;
    T_NODE t_node;
    SYSTEM_DATA *sys_data;
    sys_data = sys_data_get();
    //SciEcho("Enter SFU auto test.\r\n");

    if(get_prog_num(VIEW_ALL|sys_data->cur_chan_mode[0], 0))
    {
        if (get_prog_at(sys_data_get_cur_group_cur_mode_channel(),&p_node) == SUCCESS)
        {
            get_tp_by_id(p_node.tp_id, &t_node);
            m_freq = t_node.frq;
            m_sym = t_node.sym;
            m_constellation=t_node.fec_inner;
        }
    }
    ash_cm_register_sfu_test(sfutest_command_c);
    ash_task_init();
    m_turn_on_sfutesting = TRUE;
    sci_echo("SFU testing start \r\n");

   //osal_task_sleep(2000);
   //testtask();
    return TRUE;
}

static BOOL sfuchannel_change_c(UINT32 sfreq,UINT8 constel,UINT32 sym)
{
    //T_NODE t_node;
    union ft_xpond xponder;
    struct nim_device *nim_dev = (struct nim_device *) dev_get_by_id(HLD_DEV_TYPE_NIM, sys_data_get_sfu_nim_id());

    //MEMSET(&xponder, 0, sizeof(xponder));
    xponder.c_info.type = FRONTEND_TYPE_C;
    xponder.c_info.frq = sfreq;
    xponder.c_info.sym = sym;
    xponder.c_info.modulation = constel;

    api_nim_reset_autoscan();       //50929 yuchun

    frontend_tuning(nim_dev, NULL, &xponder, 1);

    return TRUE;
}

static int do_set_tp_c(UINT32 central_freq, UINT8 constellation,UINT32 symbol_rate)
{
    BOOL pause_status_bak = FALSE;

    m_freq = central_freq;
    m_sym = symbol_rate;
    m_constellation=constellation;

    pause_status_bak = m_pause_auto_get_current_status_task;
    m_pause_auto_get_current_status_task = TRUE;
    osal_delay(100); //wait for finish the output of the previous command.


    //make the frequence start point from the frequence central point.
    sfuchannel_change_c(central_freq ,constellation,symbol_rate);
    osal_delay(1000);
    sci_echo("SUCCESS\r\n");

    m_pause_auto_get_current_status_task = pause_status_bak;
    m_reset_dmx_per = TRUE;
    return 0;
}

static int set_tp_c(unsigned int argc, unsigned char *argv[])
{
    UINT32 freq = 0;    //in KHz.
    UINT32 sym  = 0;
    UINT8 constellation = 0;
    UINT32 modulation = 0;

    //T_NODE t_node;
    //P_NODE p_node;

    if (argc != 4)
    {
        sci_echo("Usage: settp <Central_Frequence_KHz> <Constellation_qam> <Symbol_rate_KBps>\r\n");
        return -1;//freq:498000;constellation:6/64qam;8/256qam;sym:6875
    }

    freq = ATOI((char*)argv[1])/10;
    modulation = ATOI((char*)argv[2]);
    sym = ATOI((char*)argv[3]);
    while((modulation>>1)>0)
    {
        modulation=modulation>>1;
        constellation++;
    }
    do_set_tp_c(freq, constellation,sym );
    return 0;
}

void sfutesting_stop_c()
{
    P_NODE p_node;
    T_NODE t_node;
    SYSTEM_DATA *sys_data = NULL;
    sys_data = sys_data_get();


    sfu_ash_task_delete();
    sfu_auto_get_current_status_task_delete_c();
    m_freq = 0;
    m_sym = 0;
    m_constellation=0;

    if(get_prog_num(VIEW_ALL|sys_data->cur_chan_mode[0], 0))
    {
        if (get_prog_at(sys_data_get_cur_group_cur_mode_channel(),&p_node) == SUCCESS)
        {
            get_tp_by_id(p_node.tp_id, &t_node);
            sfuchannel_change_c(t_node.frq, t_node.fec_inner,t_node.sym);
        }
    }

    m_turn_on_sfutesting = FALSE;
    sci_echo("SFU testing stop.\r\n");
}

void show_sfutesting_help(char *str)
{
    UINT32 i = 0;

    sci_echo("SFU testing commands:\r\n");
    for (i=0; sfutest_command_c[i].command != NULL; ++i)
    {
        sci_echo((char*)sfutest_command_c[i].command);
        sci_echo("\r\n");
    }
}

static void sfu_standby_reboot()
{
    //unsigned long keycode;
    SYSTEM_DATA *sys_data = NULL;
    //UINT32 vkey = 0;
    //UINT32 times = 0;
    UINT32 timer = 0;
    //UINT32 display_type = 0;
    //UINT32 hh,mm,ss;
    UINT32 cur_time = 0;
    //UINT32 target_time = 0;
    date_time dt;
    //char time_str[10];
    struct sto_device *dev = NULL;

    dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);

    sys_data = sys_data_get();
    sys_data->bstandmode = 1;
    sys_data_save(1);
    api_set_system_state(SYS_STATE_POWER_OFF);
    board_usb_power(FALSE);
    power_off_process2();
    sys_data_save(1);

    key_pan_display("oFF ", 4);
    key_pan_display_standby(1);

    api_standby_led_onoff(TRUE);
    board_power(FALSE);

    if(1)   /* Real Standby*/
    {
        get_local_time(&dt);
        pan_close(g_pan_dev);
        // disable interrupt
        osal_interrupt_disable();

        cur_time = (dt.sec & 0x3F ) | ((dt.min & 0x3F )<<6)  | ((dt.hour & 0x1F )<<12) | ((dt.day & 0x1F)<<17)
            | ((dt.month & 0xF) << 22) | (((dt.year % 100) & 0x3F)<<26);

        timer = cur_time + 1;

#ifdef WATCH_DOG_SUPPORT
        dog_stop(0);
#endif
        sto_io_control(dev, STO_DRIVER_SUSPEND,0);
        sys_ic_enter_standby(timer, cur_time);
        // enable interrupt
        osal_interrupt_enable();
    }

}

static int sfu_reboot_c(unsigned int argc, unsigned char *argv[])
{
    P_NODE p_node;
    T_NODE t_node;
    SYSTEM_DATA *sys_data = sys_data_get();
    UINT32 __MAYBE_UNUSED__ tmp=0;

    if (argc != 1)
    {
        sci_echo("Usage: reboot\r\n");
        return -1;
    }

    //UIChChgStopProg(TRUE);
    api_stop_play(0);
    m_pause_auto_get_current_status_task = TRUE;

    //for change the frequency of current channel as the SFU testing frequency.
    if(get_prog_num(VIEW_ALL|sys_data->cur_chan_mode[0], 0))
    {
        if (get_prog_at(sys_data_get_cur_group_cur_mode_channel(),&p_node) == SUCCESS)
        {
            get_tp_by_id(p_node.tp_id, &t_node);

            t_node.frq = m_freq;
            t_node.fec_inner = m_constellation;
            t_node.sym = m_sym;

            recreate_tp_view(VIEW_SINGLE_SAT, 1);
            if (modify_tp(t_node.tp_id, &t_node) != SUCCESS )
            {
                sci_echo("Err: Fail to modify the tp of program.\r\n");
                return -1;
            }
            if (update_data() != SUCCESS)
            {
                 sci_echo("Err: Fail to modify the tp of program.\r\n");
                 return -1;
            }
#if 0
        if (lookup_node(TYPE_TP_NODE, &t_node,0)!=SUCCESS)
        {
            if ((UINT32)add_node(TYPE_TP_NODE, t_node.tp_id,&t_node)==(UINT32)STORE_SPACE_FULL)
                    {
                sci_echo("Err: Database is full.\r\n");
                        return -1;
            }
                  if (update_data() != SUCCESS)
                  {
                sci_echo("Err: Fail to add the tp.\r\n");
                        return -1;
            }
        }

            if (p_node.tp_id != t_node.tp_id)
            {
                p_node.tp_id = t_node.tp_id;
                if ( modify_prog(sys_data_get_cur_group_cur_mode_channel(), &p_node) != SUCCESS )
                {
                    sci_echo("Err: Fail to modify the tp of program.\r\n");
                    return -1;
                }
                if (update_data() != SUCCESS)
                {
                    sci_echo("Err: Fail to modify the tp of program.\r\n");
                    return -1;
                }
            }
#endif
        }
    }

    sys_data_set_sfu_frequency_c(m_freq);
    sys_data_set_sfu_constellation_c(m_constellation);
    sys_data_set_sfu_symbol_c(m_sym);
    sys_data_set_sfu_nim_id_c(sys_data_get_sfu_nim_id());

    sys_data_save(0);
    osal_delay(100);
    osal_task_sleep(100);

    sfu_ash_task_delete();
    sfu_auto_get_current_status_task_delete_c();
    //osal_interrupt_disable();
    //sys_watchdog_reboot();
    //power_off_process(0);
    //sys_data_set_cur_chan_mode(TV_CHAN);
    //power_on_process();
    if(sys_ic_get_chip_id() == ALI_S3281)
    {
        sfu_standby_reboot();//for 3281 and other.
    }
    else
    {
        /**
        *  before enter watchdog_reboot,you MUST NOT read/write flash
        *  as soon as possible.make sure the bus is clear.
        */
        osal_interrupt_disable();
        *((volatile UINT32 *)0xb802e098)=0xc2000d03;
        tmp = *((volatile UINT32 *)0xb802e098);
        hw_watchdog_reboot();
    }

    while(1) ;
    return 0;
}

static int sfu_stop_c(unsigned int argc, unsigned char *argv[])
{
    if (argc != 1)
    {
        sci_echo("Usage: stop\r\n");
        return -1;
    }
    sfutesting_stop_c();
    return 0;
}
/*
static void sfu_auto_get_current_status()
{
    unsigned int i;
    nim_rec_performance_t status;

    struct nim_device *nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);
    while (!m_exit_task)
    {
        status.valid = FALSE;
        while (!status.valid && !m_exit_task)
        {
            if (m_pause_auto_get_current_status_task)
                break;
            if ( nim_io_control(nim_dev, NIM_DRIVER_GET_REC_PERFORMANCE_INFO, (UINT32)&status) != SUCCESS )
            {
                SH_PRINTF("%s: fail.\n", __FUNCTION__);
                break;
            }
            if ( status.lock == 0 || status.lock == 0xff)
            {
                status.valid = FALSE;
                status.ber = 0;
                status.per = 0;
                break;
            }
            osal_task_sleep(1);
        }
        if(m_exit_task && !status.valid)
            break;
        if (m_pause_auto_get_current_status_task)
            continue;

        if( status.lock == 0 || status.lock == 0xff)
            SciEcho("UNLOCK ");
        else
            SciEcho("LOCK ");

        IntPrintf(status.ber);
        SciEcho(" ");
        IntPrintf(status.per);
        SciEcho("\r\n");

        if(m_exit_task)
            break;

        if (m_AutoRepeatInterval_ms == 0)   //run 1 time.
        {
            break;
        }
        //m_AutoRepeatInterval_ms--;
        osal_task_sleep(m_AutoRepeatInterval_ms);
    }
    sfu_auto_get_current_status_task_id = OSAL_INVALID_ID;
}
*/
//================================================

static void sfu_auto_get_current_status_c()
{
    UINT8 lock = 0;
    UINT32 ber = 0;
    UINT32 per = 0;
    INT32  ret_ber = -1;
    INT32  ret_per = -1;
    struct nim_device *nim_dev = NULL;

    nim_dev = (struct nim_device *) dev_get_by_id(HLD_DEV_TYPE_NIM, m_nim_id);

    if(NULL == nim_dev)
    {
        SH_PRINTF("GetCurrentStatus: nim_dev null!\n");
        return ;
    }
    nim_get_lock(nim_dev, &lock);

    while (!m_exit_task)
    {

        if (m_pause_auto_get_current_status_task)
        {
           break;
        }

        ret_ber = nim_io_control(nim_dev, NIM_DRIVER_READ_QPSK_BER, (UINT32)&ber);
        ret_per = nim_io_control(nim_dev, NIM_DRIVER_READ_RSUB, (UINT32)&per);
        if ( (ret_ber != SUCCESS) || (ret_per != SUCCESS) )
        {
            SH_PRINTF("%s: fail.\n", __FUNCTION__);
            break;
        }

        if (!lock)
        {
            ber = 0;
            per = 0;
            break;
        }
        osal_task_sleep(1);

        if(m_exit_task)
        {
            break;
        }
        if (m_pause_auto_get_current_status_task)
        {
            continue;
        }
        if(!lock)
        {
            sci_echo("UNLOCK ");
        }
        else
        {
            sci_echo("LOCK ");
        }
        int_printf(ber);
        sci_echo(" ");
        int_printf(per);
        sci_echo("\r\n");

        if(m_exit_task)
        {
            break;
        }
        if (m_auto_repeat_interval_ms == 0)   //run 1 time.
        {
            break;
        }
        m_auto_repeat_interval_ms--;
        osal_task_sleep(m_auto_repeat_interval_ms);
    }
    sfu_auto_get_current_status_task_id = OSAL_INVALID_ID;
}

static BOOL sfu_auto_get_current_status_task_start_c()
{
    OSAL_T_CTSK     t_ctsk;
    m_exit_task = FALSE;

    t_ctsk.stksz    = 0x1000;
    t_ctsk.quantum  = 10;
    t_ctsk.itskpri  = OSAL_PRI_NORMAL;
    t_ctsk.name[0]  = 'G';
    t_ctsk.name[1]  = 'C';
    t_ctsk.name[2]  = 'S';
    t_ctsk.task = (FP)sfu_auto_get_current_status_c;
    sfu_auto_get_current_status_task_id = osal_task_create(&t_ctsk);
    if(OSAL_INVALID_ID == sfu_auto_get_current_status_task_id)
    {
        PRINTF("Fail to create sfu_auto_get_current_status_task_id task\n");
        return FALSE;
    }

    return TRUE;
}

static void sfu_auto_get_current_status_task_delete_c()
{
    if(OSAL_INVALID_ID != sfu_auto_get_current_status_task_id)
    {
        m_pause_auto_get_current_status_task = TRUE;
        osal_delay(50); //wait for finish the output.

        osal_task_delete(sfu_auto_get_current_status_task_id);
        m_exit_task = TRUE;
        sfu_auto_get_current_status_task_id = OSAL_INVALID_ID;
    }
}

//================================================

static int sfu_get_current_status_c(unsigned int argc, unsigned char *argv[])
{
    //INT32 ret = -1;
    UINT32 sfu_start_tick = 0;
    UINT32 auto_repeat_interval_ms = 0;

    if (2 < argc)
    {
        sci_echo("Usage: getcs [AutoRepeatInterval_ms]\r\n");
        return -1;
    }

    if (2 == argc)
    {
        auto_repeat_interval_ms = ATOI((char*)argv[1]);
    }

    m_exit_task = TRUE;
    sfu_start_tick = osal_get_tick();
    while(sfu_auto_get_current_status_task_id!=OSAL_INVALID_ID)
    {
        osal_task_sleep(1);
        if((osal_get_tick() - sfu_start_tick) > 500)
        {
            sfu_auto_get_current_status_task_delete_c();
        }

        //if(sfu_auto_get_current_status_task_id!=OSAL_INVALID_ID )
        //{
            //SciEcho(">>>>>>>>>>>error<<<<<<<<<<<");
        //}
    }

    sfu_auto_get_current_status_task_delete_c();  //clean up the previous command first.

    m_auto_repeat_interval_ms = auto_repeat_interval_ms;
    m_pause_auto_get_current_status_task = FALSE;
    sfu_auto_get_current_status_task_start_c();

    return 0;
}

static int get_dmx_per_c(unsigned int argc, unsigned char * argv[])
{
    INT32 ret = -1;
    UINT32 per = 0, packet_count_before = 0, packet_count_after = 0;
    UINT8 lock = 0;
    struct nim_device *nim_dev = NULL;

    if (1 != argc)
    {
        SH_PRINTF("Usage: getper_dmx\n");
        return -1;
    }

    nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, sys_data_get_sfu_nim_id());
    if(NULL == nim_dev)
    {
        SH_PRINTF("GetDmxPER: nim_dev null!\n");
        return -1;
    }
    nim_get_lock(nim_dev, &lock);
    if(lock)
    {
        struct dmx_device *dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
        if(NULL == dmx_dev)
        {
            SH_PRINTF("GetDmxPER: dmx_dev null!\n");
            return -1;
        }

        //struct dmx_private *prv = (struct dmx_private *)dmx_dev->priv;
        if(!m_reset_dmx_per)
        {
            dmx_io_control(dmx_dev, IO_GET_DISCONTINUE_COUNT, (UINT32)&per);//prv->disconti_counter;
            //In order to avoid this situation: 
            //Tuner had locked the signal, but no data package can passed to the DMX, because the quality is too bad.
            //'disconti_counter' of DMX driver has not to be counted at this time.
            //So 'getper_dmx' command will return result 'LOCK 0 0'.
            //It will make the SFU auto testing procedure confuse.
            if(0 == per)
            {
                dmx_io_control(dmx_dev, IO_DMX_GET_PACKET_NUM, (UINT32)&packet_count_before);
                //IntPrintf(packet_count_before);
                osal_delay((UINT16)100000);
                //SciEcho(" ");
                dmx_io_control(dmx_dev, IO_DMX_GET_PACKET_NUM, (UINT32)&packet_count_after);
                //IntPrintf(packet_count_after);                SciEcho(" ");
                if(packet_count_before == packet_count_after)
                {
                    per = 1000000;
                }
            }
        }
        else
        {
            per = 0;
            m_reset_dmx_per = FALSE;
        }
        //prv->disconti_counter = 0;
        dmx_io_control(dmx_dev, IO_CLEAR_DISCONTINUE_COUNT, 0);
    }

    if(!lock)
    {
        sci_echo("UNLOCK ");
    }
    else
    {
        sci_echo("LOCK ");
    }
    int_printf(0); // Don't need BER any more, always return 0
    sci_echo(" ");
    int_printf(per);
    sci_echo("\r\n");

    return ret;
}

#endif

/************************************************************************/
//Common

static int set_nim(unsigned int argc, unsigned char *argv[])
{
    //unsigned char cmd[16];

    if (2 != argc)
    {
        SH_PRINTF("Usage: settuner <nim_id> \n");
        return -1;
    }

    if(!strcasecmp("2",(char*)argv[1]))
    {
        m_nim_id  = 1;
    }
    else
    {
        m_nim_id = 0;
    }
    LIB_ASH_OC('N');
    LIB_ASH_OC('I');
    LIB_ASH_OC('M');
    LIB_ASH_OC((m_nim_id+'0'));
    LIB_ASH_OC(' ');
    LIB_ASH_OC('O');
    LIB_ASH_OC('K');

    LIB_ASH_OC('\r');
    LIB_ASH_OC('\n');

/* add api_play_channel() here to set nimid from UART  when there are two tuner in the same type,such as 3805 2T */
    sys_data_set_sfu_nim_id(m_nim_id);
    sys_data_save(0);
    osal_task_sleep(100);
    api_play_channel(sys_data_get_cur_group_cur_mode_channel(), TRUE, FALSE, FALSE);

#ifdef DVBS_SUPPORT
    if(board_get_frontend_type(m_nim_id) == FRONTEND_TYPE_S)
    {
        LIB_ASH_OC('D');
        LIB_ASH_OC('V');
        LIB_ASH_OC('B');
        LIB_ASH_OC('S');
        LIB_ASH_OC(' ');
        LIB_ASH_OC('O');
        LIB_ASH_OC('K');

        LIB_ASH_OC('\r');
        LIB_ASH_OC('\n');

        ash_cm_register_sfu_test(sfutest_command_s);
        reset_perflag();
    }
#endif

#if (defined(DVBT_SUPPORT) || defined(ISDBT_SUPPORT))
    if(board_get_frontend_type(m_nim_id) == FRONTEND_TYPE_T)
    {
        LIB_ASH_OC('D');
        LIB_ASH_OC('V');
        LIB_ASH_OC('B');
        LIB_ASH_OC('T');
        LIB_ASH_OC(' ');
        LIB_ASH_OC('O');
        LIB_ASH_OC('K');

        LIB_ASH_OC('\r');
        LIB_ASH_OC('\n');

        ash_cm_register_sfu_test(sfutest_command_t);
    }
#endif
#ifdef DVBC_SUPPORT 
    if(board_get_frontend_type(m_nim_id) == FRONTEND_TYPE_C)
    {
        LIB_ASH_OC('D');
        LIB_ASH_OC('V');
        LIB_ASH_OC('B');
        LIB_ASH_OC('C');
        LIB_ASH_OC(' ');
        LIB_ASH_OC('O');
        LIB_ASH_OC('K');

        LIB_ASH_OC('\r');
        LIB_ASH_OC('\n');

        ash_cm_register_sfu_test(sfutest_command_c);
        reset_perflag();
    }
#endif

    return 0;
}

int int_printf(unsigned int src)
{
    UINT8 num_stack[10] = {0};
    UINT8 pos = 0;
    INT32 i = 0;

    do{
        num_stack[pos] = src%10;
        src = src /10;
        pos++;
    } while(src);

    for(i = 0; i < pos; i++)
    {
        LIB_ASH_OC((num_stack[pos - i -1] + '0'));
    }

    return 0;
}

BOOL sfutesting_is_turn_on()
{
    return m_turn_on_sfutesting;
}

BOOL bootup_checking_for_sfutesting()  //CHN_Init() must be called first before call BootupCheckingForSFUTesting().
{
    UINT8 __MAYBE_UNUSED__ bdw = 0;
    UINT32 fre = 0;
    UINT32 symbol = 0;
    UINT32 nim_id = 0;
    UINT32 constellation = 0;
    //unsigned char cmd_buffer[30];
    //unsigned char *argv[2];

#if (defined(DVBT_SUPPORT) || defined(ISDBT_SUPPORT))
    fre = sys_data_get_sfu_frequency_t();
    bdw = sys_data_get_sfu_bandwidth_t();
    nim_id = sys_data_get_sfu_nim_id_t();
    set_sfutesting_parameter_t(fre, bdw, nim_id);
    if (0 ==  m_freq || 0 == m_bandwidth )
    {
        return FALSE;
    }

    sys_data_set_sfu_frequency_t(0);
    sys_data_set_sfu_bandwidth_t(0);
    sys_data_save(0);
    osal_delay(40);

    return sfutest_init_t();
#elif (defined(DVBC_SUPPORT))
    fre = sys_data_get_sfu_frequency_c();
    constellation = sys_data_get_sfu_constellation_c();
    symbol = sys_data_get_sfu_symbol_c();
    nim_id = sys_data_get_sfu_nim_id_c();
    set_sfutesting_parameter_c(fre, constellation, symbol, nim_id);
    if ( m_freq == 0 || m_sym == 0 )
    {
        return FALSE;
    }
    sys_data_set_sfu_frequency_c(0);
    sys_data_set_sfu_constellation_c(0);
    sys_data_set_sfu_symbol_c(0);
    sys_data_save(0);
    osal_delay(40);

    return sfutest_init_c();
#else
    return FALSE;   //DVBS_SUPPORT  don't support 'reboot' command now.
#endif
}

void sfutest_init()
{
#ifdef DVBS_SUPPORT
    sfutest_init_s();
#elif (defined(DVBT_SUPPORT) || defined(ISDBT_SUPPORT))
    sfutest_init_t();
#elif (defined(DVBC_SUPPORT))
    sfutest_init_c();
#endif

}

#endif
