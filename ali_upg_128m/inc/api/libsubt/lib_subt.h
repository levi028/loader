/*****************************************************************************
   *    Copyright (c) 2013 ALi Corp. All Rights Reserved
   *    This source is confidential and is ALi's proprietary information.
   *    This source is subject to ALi License Agreement, and shall not be
        disclosed to unauthorized individual.

   *    File: lib_subt.h

   *    Description:define the MACRO, structure uisng by the specification of
        DVB SUBTITLE FOR BROADCAST CABLE
   *    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
         KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
         IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
         PARTICULAR PURPOSE.
   *****************************************************************************/

#ifndef __LIB_SUBT_H__
#define __LIB_SUBT_H__


#ifdef __cplusplus
extern "C"
{
#endif

#include <sys_config.h>
#include <basic_types.h>

#define LIBSUBT_PRINTF(...)     do{}while(0)
//#define LIBSUBT_PRINTF         libc_printf
#define DESC_LENGTH_EIGHT 8
#define DESC_LENGTH_64 64
#define ADD_THREE 3
#ifndef DVB_SUBT_MAX_CNT
#define DVB_SUBT_MAX_CNT    10
#endif

#define SUBTITLE_LANG_NUM 10    //reserve for pvr


typedef enum
{
    DVB_SUBT,
    ATSC_SUBT
}SUBT_TYPE;

struct t_subt_lang
{
    //UINT8 lang_idx;
    UINT16 pid;
    UINT8 subt_type;
    UINT16 com_page;
    UINT16 anci_page;
    UINT8 lang[3];
};

enum subt_event
{
    SUBT_PID_UPDATE = 1,
    SUBT_PID_ADD = 3,        // Add for SAT>IP Client
};

//vicky20110322  buffer address for section data receiver,
//send to lib_subt_atsc_stream_identify() as 2nd para (Dual CPU Case)
struct atsc_subt_config_par
{
   UINT8 *bs_buf_addr;
   UINT16 bs_buf_len;
   UINT8 *sec_buf_addr;
   UINT16 sec_buf_len;
   UINT8 outline_thickness_from_stream;
   UINT8 drop_shadow_right_from_stream;
   UINT8 drop_shadow_bottom_from_stream;
   UINT8 outline_thickness;
   UINT8 drop_shadow_right;
   UINT8 drop_shadow_bottom;
};
typedef void (*SUBT_EVENT_CALLBACK)(enum subt_event event, UINT32 param);

INT32 subt_descriptor(UINT8 tag, INT32 desc_length, UINT8 *desc, void *param);
UINT32 subt_get_language(struct t_subt_lang** list ,UINT8* num);
UINT8 subt_get_cur_language(void);
INT32 subt_set_language(UINT8 lang);
INT32 subt_enable(BOOL);
void subt_pvr_set_language(struct t_subt_lang* list ,UINT8 num);
INT32 subt_pvr_enable(BOOL enable,UINT16 dmx_id);
INT32 subt_show_onoff(BOOL b_on);
INT32 subt_register(UINT32 monitor_id);
INT32 subt_unregister(UINT32 monitor_id);
void subt_reg_callback(SUBT_EVENT_CALLBACK callback);
void subt_unreg_callback();
BOOL subt_is_available();
BOOL subt_check_enable(void);
void lib_subt_atsc_attach(void *p_config);
INT32 subt_set_para(UINT16 pid, UINT16 subt_lang_num);
INT32 terminate_atsc_tasks(void);

INT32 start_atsc_tasks(UINT8 lang);

INT32 terminate_sdec_task(void);

INT32 start_sdec_task(UINT8 lang);



#ifdef ATSC_SUBTITLE_SUPPORT

BOOL create_lib_subt_atsc_section_parse_task(void);	
BOOL create_lib_subt_atsc_task(void);

#endif
#ifdef __cplusplus
 }
#endif

#endif /*__LIB_SUBT_H__  */
