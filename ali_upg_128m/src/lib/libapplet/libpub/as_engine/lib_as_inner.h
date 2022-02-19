/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: lib_as_inner.h
*
* Description:
*     auto-scan service implementation for ali chip based IRD.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef _LIB_AS_INTER_H
#define _LIB_AS_INTER_H

#ifdef __cplusplus
extern "C"
{
#endif

#if (AS_DEBUG_LEVEL > 0)
#define AS_DUMP_SAT as_dump_sat
#define AS_DUMP_TP  as_dump_tp

#else
#define AS_DUMP_SAT(...)            do{}while(0)
#define AS_DUMP_TP(...)         do{}while(0)
#endif


#define SEARCH_NIT_STATUS_NOT_REC       0
#define SEARCH_NIT_STATUS_REC           1
#define SEARCH_NIT_STATUS_SEARCH        2

#define FREQ_DIV_NUM        100
#define FREQ_OFFSET         40

#define AS_DEBUG_LEVEL          0
#if (AS_DEBUG_LEVEL>0)
#define AS_PRINTF           libc_printf
#define AS_SDBBP            SDBBP
#else
#define AS_PRINTF(...)      do{}while(0)
#define AS_SDBBP(...)           do{}while(0)
#endif

#ifdef __MM_VBV_START_ADDR
 #ifdef DUAL_ENABLE
  #define __MM_AS_START_ADDR      (__MM_EPG_BUFFER_START&0x8FFFFFFF)
  #define __MM_PSI_START_ADDR     ((__MM_EPG_BUFFER_START&0x8FFFFFFF)\
                                    +sizeof(struct as_module_info))
 #else
  #define __MM_AS_START_ADDR      (__MM_VBV_START_ADDR&0x8FFFFFFF)
  #define __MM_PSI_START_ADDR     ((__MM_VBV_START_ADDR&0x8FFFFFFF)\
                                    +sizeof(struct as_module_info))
 #endif
#else
#define AS_BUF_LEN 0x48000
#define __MM_AS_START_ADDR      ((UINT32)g_as_buf&0x8FFFFFFF)
#define __MM_PSI_START_ADDR     (((UINT32)g_as_buf&0x8FFFFFFF)\
                                +sizeof(struct as_module_info))
#endif

#if (defined(SUPPORT_MTILINGUAL_SERVICE_NAME)||defined(SUPPORT_FRANCE_HD)||defined(_MHEG5_SUPPORT_))
#define __MM_PSI_BUF_LEN        (400*1024)
#else
#define __MM_PSI_BUF_LEN        (240*1024)
#endif

#if defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT)
extern UINT32 star_search_nit_staus;
#endif
#ifdef NIT_AUTO_SEARCH_SUPPORT
extern  BOOL b_found_nit_tp
#endif

#ifndef __MM_VBV_START_ADDR
extern void *g_as_buf;
#endif

//search step
extern UINT32 c_srh_tp_step;
//dvbc frequency end.
extern UINT32 dvbc_bs_freq_end;
void as_dump_sat(S_NODE *sat);
void as_dump_tp(T_NODE *tp);

/* set auto scan info */
void as_set_as_info(struct as_module_info *info);

/*description   : transpoder program search update callback.*/
UINT8 psi_callback(UINT8 progress, void *param);

/*NIt search callback for update tp*/
UINT8 as_update_tp(TP_INFO *t_info);

/* According to input as info and tp info, set anntenna and nim of frontend */
UINT32 as_frontend_tuning(struct as_module_info *info, T_NODE *tp);

/* Set as info(including tp id, timeout, and so on) for one specific tp */
void as_dvbs_preset_tuning(void *param);

//Try to auto detect the next signal pipe within this channel,
//such as the next PLP of (MPLP of DVB-T2), or the next priority
//signal(Hierarchy of DVB-T).Before USAGE_TYPE_NEXT_PIPE_SCAN can be used,
//you must call USAGE_TYPE_AUTOSCAN or USAGE_TYPE_CHANSCAN first.
BOOL as_next_pipe_tuning(struct as_module_info *info, T_NODE *tp);

/*DVBC blind scan in fact is tp scan by bandwith step*/
void as_dvbc_blind_tuning(void *param);

/*DVBT blind scan in fact is tp scan by bandwith step*/
void as_dvbt_blind_tuning(void *param);
/*DVBT  init search param */
BOOL dvbt_init_search_param(struct as_service_param *param, \
            struct as_tuning_param *atp);
#if(defined(POLAND_SPEC_SUPPORT)||defined(SUPPORT_FRANCE_HD))            
///add for poland and france_hd book to get signal of tp            
void get_signal_of_tp(T_NODE *tp);
#endif

//dvbs blind scan nim api callback function.
INT32 dvbs_as_callback(UINT8 signal, UINT8 polar, \
            UINT32 freq, UINT32 sym, UINT8 fec);

//dvbs blind scan.
INT32 as_dvbs_blind_scan(struct ft_antenna *antenna);


#ifdef __cplusplus
}
#endif

#endif /* _LIB_AS_INTER_H */

