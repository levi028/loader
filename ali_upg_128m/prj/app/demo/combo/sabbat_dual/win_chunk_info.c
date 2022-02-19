/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_chunk_info.c
*
*    Description: To realize the UI for display system information
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#ifdef _INVW_JUICE
#include <api/libfs2/stdio.h>
#endif
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <bus/otp/otp.h>

#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <hal/hal_gpio.h>
#ifdef _INVW_JUICE
#else
#include <api/libfs2/stdio.h>
#endif
#include <api/libfs2/statvfs.h>
 #include <asm/chip.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "win_com_menu_define.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "ap_ctrl_display.h"
//add by bill 2012-02-13
#include "platform/board.h"
#include "win_chunk_info.h"
#include "win_diagnosis.h"
#include "menus_root.h"
#include "win_prog_detail.h"

#ifdef _NV_PROJECT_SUPPORT_
#include <udi/nv/ca_block.h>
#include <udi/nv/ca_mmap.h>
#endif


/*******************************************************************************
* Objects declaration
*******************************************************************************/
static UINT16 info_name_str[30];
static UINT16 info1_str[30];
static UINT16 info2_str[30];

static VACTION info_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT info_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC_STD))&&(!defined _BC_CA_ENABLE_))
static void win_epg_set_show_state(void);
#endif
extern void api_stop_play(UINT32 bpause);
extern void ap_send_msg(control_msg_type_t msg_type, UINT32 msg_code, BOOL if_clear_buffer);

#ifndef SD_UI
#ifndef SUPPORT_CAS_A
/* define in win_com_menu_define.h already */
#define W_L         248
#define W_T         98
#define W_W         692
#define W_H         488
#endif

#define CON_L       (W_L+20)
#define CON_T       (W_T + 4)
#define CON_W       (W_W - 60)
#define CON_H       40
#define CON_GAP     12

#define INFO_TXT_NAME_L 20
#define INFO_TXT_NAME_W 230//200
#define INFO_TXT1_L (INFO_TXT_NAME_L+INFO_TXT_NAME_W)
#define INFO_TXT1_W 230
#define INFO_TXT2_L (INFO_TXT1_L+INFO_TXT1_W)
#define INFO_TXT2_W 180

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4

#else
#define W_L         105//384
#define W_T         57 //138
#define W_W         482
#define W_H         370

#ifdef CON_L
#undef CON_L
#endif

#ifdef CON_T
#undef CON_T
#endif

#ifdef CON_W
#undef CON_W
#endif

#ifdef CON_H
#undef CON_H
#endif

#ifdef CON_GAP
#undef CON_GAP
#endif

#define CON_L       (W_L+10)
#define CON_T       (W_T + 8)
#define CON_W       (W_W - 25)
#define CON_H       30
#define CON_GAP     8

#define INFO_TXT_NAME_L 10
#define INFO_TXT_NAME_W 180//200
#define INFO_TXT1_L (INFO_TXT_NAME_L+INFO_TXT_NAME_W)
#define INFO_TXT1_W 140
#define INFO_TXT2_L (INFO_TXT1_L+INFO_TXT1_W)
#define INFO_TXT2_W 100

#define LINE_L_OF   0
#define LINE_T_OF   (CON_H+4)
#define LINE_W      CON_W
#define LINE_H      4
#endif

#define FLAG_LOG_ENABLE    0x00000100
#define FLAG_LOG_END       0x00000001
#define MEMCFG_POSITION_IN_CHUNK 2
#define LOG_THRESHOLD 9999
#define SAMPLE_CHIP_ID_HIGH_VALUE 0x1e
#define SAMPLE_CHIP_ID_LOW_VALUE  0x22
#define LENGTH_2BYTES  16
#define LENGTH_3BYTES  24
#define LINE_1  0
#define LINE_2  1
#define LINE_3  2

#define LDEF_CON(root, var_con,nxt_obj,l,t,w,h,conobj)        \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
     NULL,NULL,  \
    conobj, 0,0)

#define LDEF_TXT(root,var_txt,nxt_obj,ID,l,t,w,h,res_id,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,res_id,str)

#define LDEF_LIST_ITEM(root,var_con,nxt_obj,var_txt_name,var_txt1,var_txt2,var_line,l,t,w,h)   \
    LDEF_CON(&root, var_con,nxt_obj,l,t,w,h,&var_txt_name)  \
    LDEF_TXT(&var_con,var_txt_name,&var_txt1 ,1,l + INFO_TXT_NAME_L, t,INFO_TXT_NAME_W,h,0,info_name_str)   \
    LDEF_TXT(&var_con,var_txt1,&var_txt2 ,2,l + INFO_TXT1_L, t,INFO_TXT1_W,h,0,info1_str)  \
    LDEF_TXT(&var_con,var_txt2,NULL/*&varLine*/ ,3,l + INFO_TXT2_L, t,INFO_TXT2_W,h,0,info2_str)  \
    LDEF_LINE(&var_con,var_line,NULL,1,1,1,1,1,l + LINE_L_OF, t + LINE_T_OF,LINE_W,LINE_H,0,0)


LDEF_LIST_ITEM(win_info_con,info_item_con,NULL,info_txt_name,info_txt1,info_txt2,info_line,\
    CON_L, CON_T + (CON_H + CON_GAP)*0,CON_W,CON_H)

DEF_CONTAINER(win_info_con,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    info_con_keymap,info_con_callback,  \
    (POBJECT_HEAD)&info_item_con, 0,0)

static BOOL epg_show_logo = TRUE;
static const UINT32 chip_id_const = 505580010;
#ifndef DISABLE_PRINTF
  #if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
  static BOOL log_nim = FALSE;
  #endif
#endif
static int key_cnt_factest = 0;
static int key_cnt_intest = 0;
static int key_cnt_diag = 0;
static int key_cnt_hdmi_factest = 0;
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
static int key_cnt_epg_logo = 0;
static int key_cnt_otp_bonding = 0;
static int key_cnt_adc = 0;
static char log_file_name[128];
static FILE *m_log_file = NULL;
static OSAL_ID log_flag = OSAL_INVALID_ID;
#endif
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
static int key_cnt_lognim = 0;
#endif
static int key_cnt_mute = 0;
static int key_cnt_uart = 0;
//------------------------------------------------------------------------------
// Output string to the serial port, and log to file.
//------------------------------------------------------------------------------
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
static BOOL fs_is_available(void)
{
    struct statvfs usb_info;
    BOOL ret = TRUE;

    MEMSET(&usb_info, 0, sizeof(struct statvfs));
    if (fs_statvfs("/mnt/uda1", &usb_info) <  0)
    {
        ret = FALSE;
    }
    else if(0 == usb_info.f_bfree)//usb device do not exist or mount failure
    {
        ret = FALSE;
    }
    return ret;
}

static BOOL log_open(void)
{
    FILE *fp = NULL;
    int i=0;

    if (log_flag != OSAL_INVALID_ID)
    {
        libc_printf("%s: The log had been turned on already - %s.\n", __FUNCTION__, log_file_name);
        return TRUE;
    }

    log_flag = osal_flag_create(FLAG_LOG_ENABLE);
    if (OSAL_INVALID_ID == log_flag)
    {
        libc_printf("%s: Fail to turn on.\n", __FUNCTION__);
        return FALSE;
    }

    do
    {
        log_file_name[0] = 0;   //clean up first.
        if( !fs_is_available() )
        {
            break;  //Output to the serial port only.
        }
        i = 1;
        while(1)
        {
            if(i > LOG_THRESHOLD)
            {
                break;
            }
            snprintf(log_file_name, 128, "/mnt/uda1/ali_log_%04d.txt",i++); //"/c/ali_log_%04d.txt"
            fp = fopen(log_file_name, "rb");
            if(NULL == fp)
            {
                break;
            }
            fclose(fp);
        }

        m_log_file = fopen(log_file_name, "wb");
    }while(0);

    //if ( !m_log_file && log_file_name[0]!=0 )
    //libc_printf("%s: Can't create log file - %s. Output to the serial port only.\n", __FUNCTION__, log_file_name);
    osal_flag_set(log_flag, FLAG_LOG_END);
    return TRUE;
}

static BOOL log_close(void)
{
    UINT32 flgptn=0;
    OSAL_ER result=0;

    result = osal_flag_wait(&flgptn, log_flag, FLAG_LOG_END, OSAL_TWF_ANDW|OSAL_TWF_CLR, 4*1000);
    if(OSAL_E_OK != result)
    {
        return FALSE;
    }
    if(m_log_file != NULL)
    {
        fclose(m_log_file);
        fsync("/mnt/uda1");
        m_log_file = NULL;
    }

    if (log_flag != OSAL_INVALID_ID)
    {
        osal_flag_delete(log_flag);
        log_flag = OSAL_INVALID_ID;
    }
    return TRUE;
}

static void log_file_write(char *buffer)
{
    INT32 len=0;

    if(NULL == m_log_file)
    {
        return;
    }
    len = strlen(buffer);

//        if( ! fs_is_available() )
//            return FALSE;

    if(len)
    {
        fwrite(buffer, 1, len, m_log_file);
        fflush(m_log_file);
    }
}

static void log_string(char *buffer)
{
    UINT32 flgptn=0;
    OSAL_ER result=0;

    if (OSAL_INVALID_ID == log_flag)
    {
        libc_printf("%s: log_open() shall be called first.\n", __FUNCTION__);
        return;
    }
    result = osal_flag_wait(&flgptn, log_flag, FLAG_LOG_END, OSAL_TWF_ANDW|OSAL_TWF_CLR, 4*1000);
    if(OSAL_E_OK != result)
    {
        return;
    }
    libc_printf("%s", buffer);
    log_file_write(buffer);

    osal_flag_set(log_flag,FLAG_LOG_END);
}
#endif
//------------------------------------------------------------------------------


/*******************************************************************************
*   Local functions & variables declare
*******************************************************************************/
static void set_info_con_rect(UINT16 l, UINT16 t, UINT16 w, UINT16 h)
{
    osd_set_rect(&info_item_con.head.frame, l,t,w,h);
    osd_set_rect(&info_txt_name.head.frame, l + INFO_TXT_NAME_L, t,INFO_TXT_NAME_W,h);
    osd_set_rect(&info_txt1.head.frame, l + INFO_TXT1_L, t,INFO_TXT1_W,h);
    osd_set_rect(&info_txt2.head.frame, l + INFO_TXT2_L, t,INFO_TXT2_W,h);
    osd_set_rect(&info_line.head.frame, l + LINE_L_OF, t+LINE_T_OF,LINE_W,LINE_H);
}

static void display_sys_infomation(UINT8 flag)
{
    CHUNK_HEADER blk_header;
    UINT8 i=0;
    UINT32 id=0;
    UINT32 mask=0;
    char str[30] = {0};
    BYTE info_block_number=0;
    UINT32 len __MAYBE_UNUSED__=0;
    UINT32 chunkid __MAYBE_UNUSED__=0;
    UINT32 addr __MAYBE_UNUSED__=0;
    UINT32 top = 0;
    UINT32 stb_sn_pre __MAYBE_UNUSED__= 0;
    UINT32 stb_sn_lst __MAYBE_UNUSED__= 0;
    UINT32 sc_chip_id __MAYBE_UNUSED__= 0;
    int tmp_ret __MAYBE_UNUSED__= 0;
#if (defined(_CAS9_CA_ENABLE_)|| defined(_BC_CA_ENABLE_) || defined (_BC_CA_STD_ENABLE_) )
    UINT8 *sn_buf = MALLOC(128);
    if (NULL == sn_buf)
    {
        return;
    }
#endif
#ifdef _NV_PROJECT_SUPPORT_
    block_info_t sto_block_infor;
#endif
    unsigned char *hardware_date __MAYBE_UNUSED__= (unsigned char *)"2014-12-10";

    MEMSET(&blk_header, 0, sizeof(CHUNK_HEADER));
#if (defined( SUPPORT_C2000A) ||defined( SUPPORT_CAS9) || defined( SUPPORT_CAS7 )||defined(SUPPORT_BC) \
|| defined( SUPPORT_BC_STD) || defined(FTA_ONLY))
    info_block_number = sto_chunk_count(0,0);
    for(i=0;i<info_block_number;i++)
    {
        id =0;
        mask = 0;
        sto_chunk_goto(&id,mask,i + 1);
        tmp_ret = sto_get_chunk_header(id, &blk_header);
        if(tmp_ret != 1)
        {
            return;
        }
        if(MAINCODE_ID == id )
        {
            break;
        }
    }
    for(i=0;i<3;i++)
    {        
        MEMSET(str,0,sizeof(str));
        if(LINE_1 == i)
        {
            snprintf(str, 30, "Hardware Version");
        }
        else if(LINE_2 == i)
        {
            snprintf(str, 30, "Software Version");
        }
        else if(LINE_3 == i)
        {
            snprintf(str, 30, "STB SN");
            #ifdef _GEN_CA_ENABLE_
            //gca project, CSTM need to modify the SN display rules as needed.
            //so demo just skip the SN.
            continue;
            #endif 
        }
        else
        {
#if (defined(_CAS9_CA_ENABLE_)|| defined(_BC_CA_ENABLE_) || defined (_BC_CA_STD_ENABLE_) )
            free(sn_buf);
            sn_buf = NULL;
#endif
            break;
        }
        //re-arrange the container.
        set_info_con_rect(CON_L, CON_T + (CON_H + CON_GAP)*(i+top),CON_W,CON_H);
        osd_set_text_field_content(&info_txt_name,STRING_ANSI, (UINT32)str);
        if(0 == flag)
        {
            if(LINE_1 == i)
            {
                //sprintf(str,"%x.%x.%x.%x",0,0,0,1);
                snprintf(str, 30, "%d.%d.%d.%d",(SYS_HW_VERSION>>12)&0x0f,(SYS_HW_VERSION>>8)&0x0f,\
                                (SYS_HW_VERSION>>4)&0x0f,SYS_HW_VERSION&0x0f);
            }
            else if(LINE_2 == i)
            {
                snprintf(str, 30, "%d.%d.%d.%d",blk_header.version[4]-'0',blk_header.version[5]-'0',\
                                blk_header.version[6]-'0',blk_header.version[7]-'0');
            }
            else// if((LINE_3 ==  i))   //STB SN
            {
#if (defined(_CAS9_CA_ENABLE_)|| defined(_BC_CA_ENABLE_) || defined (_BC_CA_STD_ENABLE_) )
                chunkid = 0x20DF0100;
                addr =  (UINT32)sto_chunk_goto(&chunkid,0xFFFFFFFF,1);//chunk addr               
                MEMSET(sn_buf, 0x0, sizeof (*sn_buf));
                sto_get_data((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO,0),sn_buf,addr+CHUNK_HEADER_SIZE,64);
                len = sn_buf[0];
                if((len >0) && (len<0x7F))
                {
                    sn_buf[len+1]=0;
                    snprintf(str, 30, "%s",&sn_buf[1]);
                }
                else
                {
#endif
                    otp_read(0, (UINT8 *)&sc_chip_id, 4);
                    //sc_chip_id = 505580011;
                    if((((sc_chip_id>>LENGTH_3BYTES)&0xff) != SAMPLE_CHIP_ID_HIGH_VALUE)
                        && (((sc_chip_id>>LENGTH_2BYTES)&0xff) != SAMPLE_CHIP_ID_LOW_VALUE))  //not commecial card
                    {
                        stb_sn_pre = 0;
                        stb_sn_lst = 0;
                        strncpy(str, "00000000000000000", 30);
                    }
                    else
                    {
                        stb_sn_pre = 254110;
                        stb_sn_lst = 40000001+(sc_chip_id-chip_id_const);
                        snprintf(str, 30, "000%6lu%8lu", stb_sn_pre,stb_sn_lst);
                    }
#if (defined(_CAS9_CA_ENABLE_)|| defined(_BC_CA_ENABLE_)  || (defined _BC_CA_STD_ENABLE_))
                }
                FREE(sn_buf);
                sn_buf = NULL;
#endif
#ifndef SD_UI
                osd_set_rect(&info_txt1.head.frame, CON_L + INFO_TXT1_L, CON_T + (CON_H + CON_GAP)*(i+top),\
                                    INFO_TXT1_W+180,CON_H);
                osd_set_rect(&info_txt2.head.frame, CON_L + INFO_TXT1_L+180, CON_T + (CON_H + CON_GAP)*(i+top),0,CON_H);
#else
                osd_set_rect(&info_txt1.head.frame, CON_L + INFO_TXT1_L, CON_T + (CON_H + CON_GAP)*(i+top),\
                                    INFO_TXT1_W+150,CON_H);
                osd_set_rect(&info_txt2.head.frame, CON_L + INFO_TXT1_L+150, CON_T + (CON_H + CON_GAP)*(i+top),0,CON_H);
#endif
            }
            osd_set_text_field_content(&info_txt1,STRING_ANSI, (UINT32)str);
            if(0 == i)
            {
                osd_set_text_field_content(&info_txt2,STRING_ANSI, (UINT32)hardware_date);
            }
            else if(1 == i)
            {
                osd_set_text_field_content(&info_txt2,STRING_ANSI, (UINT32)blk_header.time);
            }
        }
        else
        {
            /*length*/
            snprintf(str, 30, "0x%lu",blk_header.len);
            osd_set_text_field_content(&info_txt1,STRING_ANSI, (UINT32)str);
            /* Offset */
            snprintf(str, 30, "0x%lu",blk_header.offset);
            osd_set_text_field_content(&info_txt2,STRING_ANSI, (UINT32)str);
        }
        osd_draw_object((POBJECT_HEAD )&info_item_con,C_UPDATE_ALL);
    }
#elif defined(_NV_PROJECT_SUPPORT_)
    for(i=0;i<3;i++)
    {
        set_info_con_rect(CON_L, CON_T + (CON_H + CON_GAP)*(i+top),CON_W,CON_H);
        MEMSET(str,0,sizeof(str));
        if(LINE_1 == i)
        {
            snprintf(str, 30, "MAIN_SW Version");
        }
        else if(LINE_2 == i)
        {
            snprintf(str, 30, "SEE_SW Version");
        }
        else if(LINE_3 == i)
        {
            snprintf(str, 30, "LOGO Version");
        }
        else
        {
            break;
        }
        osd_set_text_field_content(&info_txt_name,STRING_ANSI, (UINT32)str);
        if(0 == flag)
        {
            if(LINE_1 == i)
            {
                sto_block_info_get(FLASH_BLK_MAIN_SW, &sto_block_infor);
            }
            else if(LINE_2 == i)
            {
                sto_block_info_get(FLASH_BLK_SEE_SW, &sto_block_infor);

            }
            else// if((LINE_3 ==  i))   //STB SN
            {
                sto_block_info_get(FLASH_BLK_LOGO, &sto_block_infor);
            }
            //snprintf(str, 30, "%4lu", sto_block_infor.version);
            snprintf(str, 30, "  %d.%d.%d.%d",(sto_block_infor.version>>12)&0x0f,(sto_block_infor.version>>8)&0x0f,\
                                (sto_block_infor.version>>4)&0x0f,sto_block_infor.version&0x0f);

            osd_set_text_field_content(&info_txt1,STRING_ANSI, (UINT32)str);

            //osd_set_text_field_content(&info_txt2,STRING_ANSI, (UINT32)hardware_date);

            osd_draw_object((POBJECT_HEAD )&info_item_con,C_UPDATE_ALL);
        }
    }
#else
    info_block_number = sto_chunk_count(0,0);
    for(i=0;i<info_block_number;i++)
    {
        id = mask = 0;
        sto_chunk_goto(&id,mask,i + 1);
#ifndef USBUPG_MEMCFG_SUPPORT  //for 3503 not show memcfg info in menu
        if(ALI_S3503==sys_ic_get_chip_id())
        {
            if(i>=(info_block_number-1))
                break;
            if(i>=MEMCFG_POSITION_IN_CHUNK)
                sto_chunk_goto(&id,mask,i+1+1);
        }
#endif

        sto_get_chunk_header(id, &blk_header);
        set_info_con_rect(CON_L, CON_T + (CON_H + CON_GAP)*(i+top),CON_W,CON_H);
        /* Name */
        osd_set_text_field_content(&info_txt_name,STRING_ANSI, (UINT32)blk_header.name);

        if(0 == flag)
        {//show version & time
            if(0 == i) /* Version */
                osd_set_text_field_content(&info_txt1,STRING_ANSI, (UINT32)&blk_header.version[8]);
            else
                osd_set_text_field_content(&info_txt1,STRING_ANSI, (UINT32)blk_header.version);
            osd_set_text_field_content(&info_txt2,STRING_ANSI, (UINT32)blk_header.time);/* Time */
        }
        else
        {//show length & offset
            snprintf(str, 30, "0x%x",(int)blk_header.len); /*length*/
            osd_set_text_field_content(&info_txt1,STRING_ANSI, (UINT32)str);
            snprintf(str, 30, "0x%x",(int)blk_header.offset); /* Offset */
            osd_set_text_field_content(&info_txt2,STRING_ANSI, (UINT32)str);
        }
        osd_draw_object((POBJECT_HEAD )&info_item_con,C_UPDATE_ALL);
    }
#endif

#if (defined(_CAS9_CA_ENABLE_)|| defined(_BC_CA_ENABLE_) || defined (_BC_CA_STD_ENABLE_) )
    if (NULL != sn_buf)
    {
        free(sn_buf);
        sn_buf = NULL;
    }
#endif
}


static BOOL factorytest_check_magic_key(UINT32 code __MAYBE_UNUSED__)
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    key_cnt_factest = 0;
    return FALSE;
#endif
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
    
	UINT8 magic_key[] = { V_KEY_1, V_KEY_1,V_KEY_1,V_KEY_1,V_KEY_1 };

    if (magic_key[key_cnt_factest++] == code)
    {
        if (key_cnt_factest == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt_factest = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
#endif
    key_cnt_factest = 0;
    return FALSE;
}

static BOOL factorytest_check_magic_key_inte(UINT32 code __MAYBE_UNUSED__)
{
//#ifdef REMOVE_HIDEN_MENU_FUNCTION
//    key_cnt_factest = 0;
//    return FALSE;
//#endif
#if(defined(INTERNAL_TEST))
    
	UINT8 magic_key[] = { V_KEY_1, V_KEY_1,V_KEY_1,V_KEY_1,V_KEY_1 };

    if (magic_key[key_cnt_intest++] == code)
    {
        if (key_cnt_intest == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt_intest = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
#endif
    key_cnt_intest = 0;
    return FALSE;
}


BOOL factorytest_check_magic_key_diag(UINT32 code __MAYBE_UNUSED__)
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    key_cnt_factest = 0;
    return FALSE;
#endif    
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
    UINT8 magic_key[] = { V_KEY_9, V_KEY_8,V_KEY_7,V_KEY_6,V_KEY_0 };

    if (magic_key[key_cnt_diag++] == code)
    {
        if (key_cnt_diag == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt_diag = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
#endif
    key_cnt_diag = 0;
    return FALSE;
}
//
static BOOL factorytest_check_hdmi_hdcp_key(UINT32 code __MAYBE_UNUSED__)
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    key_cnt_hdmi_factest = 0;
    return FALSE;
#endif    
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
    UINT8 magic_key[] = { V_KEY_6, V_KEY_6,V_KEY_6,V_KEY_6,V_KEY_6 };

    if (magic_key[key_cnt_hdmi_factest++] == code)
    {
        if (key_cnt_hdmi_factest == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt_hdmi_factest = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
#endif
    key_cnt_hdmi_factest = 0;
    return FALSE;
}
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
static BOOL factorytest_check_epg_show_logo_key(UINT32 code )
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    key_cnt_epg_logo = 0;
    return FALSE;
#endif
    UINT8 magic_key[] = { V_KEY_8, V_KEY_8,V_KEY_8,V_KEY_8,V_KEY_8 };

    if (magic_key[key_cnt_epg_logo++] == code)
    {
        if (key_cnt_epg_logo == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt_epg_logo = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
    key_cnt_epg_logo = 0;
    return FALSE;
}

static BOOL factorytest_check_opt_bonding_key(UINT32 code)
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    key_cnt_otp_bonding = 0;
    return FALSE;
#endif    
    UINT8 magic_key[] = { V_KEY_5, V_KEY_5,V_KEY_5,V_KEY_5,V_KEY_5 };

    if (magic_key[key_cnt_otp_bonding++] == code)
    {
        if (key_cnt_otp_bonding == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt_otp_bonding = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
    key_cnt_otp_bonding = 0;
    return FALSE;
}

static BOOL factorytest_check_adc_key(UINT32 code)
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    key_cnt_adc = 0;
    return FALSE;
#endif    
    UINT8 magic_key[] = { V_KEY_7, V_KEY_7,V_KEY_7,V_KEY_7,V_KEY_7 };

    if (magic_key[key_cnt_adc++] == code)
    {
        if (key_cnt_adc == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt_adc = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
    key_cnt_adc = 0;
    return FALSE;
}

static BOOL factorytest_check_log_nim_key(UINT32 code)
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    key_cnt_lognim = 0;
    return FALSE;
#endif     
    UINT8 magic_log[] = { V_KEY_3, V_KEY_3,V_KEY_3,V_KEY_3,V_KEY_3 };

    if (magic_log[key_cnt_lognim++] == code)
    {
        if (key_cnt_lognim == (int)(sizeof(magic_log)/sizeof(UINT8)))
        {
            key_cnt_lognim = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
    key_cnt_lognim = 0;

    return FALSE;
}
#endif

static BOOL factorytest_mute_gpio_key(UINT32 code __MAYBE_UNUSED__)
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    key_cnt_mute = 0;
    return FALSE;
#endif     
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
    UINT8 magic_key[] = { V_KEY_0, V_KEY_0,V_KEY_0,V_KEY_0,V_KEY_0 };

    if (magic_key[key_cnt_mute++] == code)
    {
        if (key_cnt_mute == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt_mute = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
#endif
    key_cnt_mute = 0;
    return FALSE;
}
static BOOL factorytest_mute_uart_key(UINT32 code __MAYBE_UNUSED__)
{
#ifdef REMOVE_HIDEN_MENU_FUNCTION
    key_cnt_uart = 0;
    return FALSE;
#endif    
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))
    UINT8 magic_key[] = { V_KEY_9, V_KEY_9,V_KEY_9,V_KEY_9,V_KEY_9 };

    if (magic_key[key_cnt_uart++] == code)
    {
        if (key_cnt_uart == (int)(sizeof(magic_key)/sizeof(UINT8)))
        {
            key_cnt_uart = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
#endif
    key_cnt_uart = 0;
    return FALSE;
}
/*******************************************************************************
*   Window's keymap, proc and  callback
*******************************************************************************/
static VACTION info_con_keymap(POBJECT_HEAD pobj __MAYBE_UNUSED__, UINT32 vkey)
{
    VACTION act=0;

    switch(vkey)
    {
    case V_KEY_MENU:
    case V_KEY_EXIT:
        act = VACT_CLOSE;
        break;
#if (!defined(_GEN_CA_ENABLE_)) || defined(INTERNAL_TEST)       
    case V_KEY_0:   case V_KEY_1:   case V_KEY_2:   case V_KEY_3:
    case V_KEY_4:   case V_KEY_5:   case V_KEY_6:   case V_KEY_7:
    case V_KEY_8:   case V_KEY_9:
        act = vkey - V_KEY_0 + VACT_NUM_0;
        break;
#endif        
    default:
        act = VACT_PASS;
    break;
    }

    return act;
}

static PRESULT info_con_callback(POBJECT_HEAD pobj __MAYBE_UNUSED__, 
    VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    POBJECT_HEAD submenu = NULL;
    INT32 result = 0;
    UINT16 channel = sys_data_get_cur_group_cur_mode_channel();
    UINT16 group_idx = sys_data_get_cur_group_index();

	if(0 == result)
	{
		;
	}
    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&win_info_con,RS_TOOLS_INFORMATION, 0);
        break;
    case EVN_POST_OPEN:
        display_sys_infomation(0);
        break;
    case EVN_PRE_CLOSE:
        /* Make OSD not flickering */
        *((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        break;
    case EVN_UNKNOWN_ACTION:
        param1 &= 0xff;
#ifdef INTERNAL_TEST
		if (factorytest_check_magic_key_inte(param1))
        {
			submenu = (POBJECT_HEAD)&g_win_internal_test;
			if(osd_obj_open(submenu, MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
				menu_stack_push(submenu);
        	break;
		}
#endif
#if(!defined(_NV_PROJECT_SUPPORT_))   
        if (factorytest_check_magic_key(param1))
        {
            submenu = (POBJECT_HEAD)&g_win_factory_test;
            if(osd_obj_open(submenu, MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
            {
                menu_stack_push(submenu);
            }
            break;
        }

        if (factorytest_check_magic_key_diag(param1))
        {
            sys_data_change_group(group_idx);

            api_stop_play(0); //to call si_monitor_off for fixing bug 13806 --Michael 2008/2/18
            api_play_channel(channel, TRUE, FALSE, FALSE);
            ap_clear_all_menus();

            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)&win_diag, TRUE);
            break;
        }

        if(factorytest_check_hdmi_hdcp_key(param1))
        {
            submenu = (POBJECT_HEAD)&g_win_hdmi_factory_test;
            if(osd_obj_open(submenu,MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
            {
                menu_stack_push(submenu);
            }
            break;
        }

#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC_STD))&&(!defined _BC_CA_ENABLE_))
        if(factorytest_check_epg_show_logo_key(param1))
        {
            UINT8 back_saved;
            win_epg_set_show_state();
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            if(win_epg_is_show_logo())
            {
                win_compopup_set_msg("EPG Show Logo", NULL, 0);
            }
            else
            {
                win_compopup_set_msg("EPG Normal", NULL, 0);
            }
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
            break;
        }
#endif

        if(check_magic_key_firmware_info(param1))
        {
            if (sys_ic_get_chip_id() >= ALI_S3602F)
            {
                show_firmware_info();
            }
            break;
        }

#ifndef DISABLE_PRINTF  //_CAS9_CA_ENABLE_

#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))

        if(factorytest_check_adc_key(param1))
        {
    #ifdef  NIM_REG_ENABLE
            submenu = (POBJECT_HEAD)&g_win_adc;
            if(osd_obj_open(submenu,MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
            {
                menu_stack_push(submenu);
            }
            break;
    #endif
        }

        if(factorytest_check_log_nim_key(param1))
        {
            UINT8 back_saved;
            char info[255];

            struct nim_device * dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
            if (!dev)
                break;

            win_compopup_init(WIN_POPUP_TYPE_SMSG);
            if (log_nim)
            {
                if ( m_log_file )
                    snprintf(info, 255, "log_close: %s",log_file_name);
                else
                    snprintf(info, 255, "log_close: Serial port");
                win_compopup_set_msg(info, NULL, 0);

                nim_io_control(dev, NIM_DRIVER_DISABLE_DEBUG_LOG, (UINT32)NULL);
                if (log_close())
                    log_nim = FALSE;
            }
            else
            {
                log_open();
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC))&&(!defined(SUPPORT_BC_STD)))          
                nim_io_control(dev, NIM_DRIVER_ENABLE_DEBUG_LOG, (UINT32)log_string);
#endif
                log_nim = TRUE;

                if ( m_log_file )
                    snprintf(info, 255, "log_open: %s",log_file_name);
                else
                    snprintf(info, 255, "log_open: Serial port");
                win_compopup_set_msg(info, NULL, 0);
            }

            win_compopup_open_ext(&back_saved);
            osal_task_sleep(4000);
            win_compopup_smsg_restoreback();

            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&win_info_con,FALSE);
            break;
        }
#endif

#endif
        //add by bill on 2012-02-13
        if(factorytest_mute_gpio_key(param1))
        {
            //*(volatile UINT32*)(0xb8000088) &= ~(1<<8);
            *(volatile UINT32*)(0xb8000088) &= ~(3);
            *(volatile UINT32*)(0xb8000430) |=(1<<16);
            HAL_GPIO_BIT_DIR_SET( 16, 1);
            *(volatile UINT32*)(0xb8000054)&=~(1<<15);
            *(volatile UINT32*)(0xb8000430) |=(1<<15);
            HAL_GPIO_BIT_DIR_SET( 15, 1);
            *(volatile UINT32*)(0xb8000054)|=(1<<15);

            //HAL_GPIO_BIT_DIR_SET( 16, 1);
            //HAL_GPIO_BIT_SET( 16,0 );
            result = pan_display(g_pan_dev, "0000", 4);
            break;
        }

        if(factorytest_mute_uart_key(param1))
        {
            *(volatile UINT32*)(0xb8000430) &=~(3<<15);
            //*(volatile UINT32*)(0xb8000088) |= 1<<8;
            *(volatile UINT32*)(0xb8000088) |= 3;
            //printf("UART MODE\n");
            libc_printf("UART MODE\n");
            result = pan_display(g_pan_dev, "9999", 4);
            break;
        }
#endif //if((!defined(_NV_PROJECT_SUPPORT_))  
        break;
    default:
        break;
    }
    return ret;
}

BOOL win_epg_is_show_logo(void)
{
    return epg_show_logo;
}
#if((!defined(SUPPORT_CAS9))&&(!defined(SUPPORT_BC_STD))&&(!defined _BC_CA_ENABLE_))
static void win_epg_set_show_state(void)
{
    epg_show_logo = epg_show_logo?FALSE:TRUE;
}
#endif


