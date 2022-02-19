/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_disk_info.c
*
*    Description: The ui of disk info
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <sys_config.h>

#ifdef DVR_PVR_SUPPORT
#include <types.h>
#include <osal/osal.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libfs2/types.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/unistd.h>
#include <api/libvfs/vfs.h>
#include <api/libvfs/vfs_utils.h>
#include <api/libvfs/device.h>
#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"
#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com_popup.h"
#include "win_com.h"
#include "pvr_control.h"
#ifdef PVR2IP_SERVER_SUPPORT
#include <api/libsat2ip/sat2ip_msg.h>
#include <api/libsat2ip/libprovider.h>
#endif

#ifdef DISK_MANAGER_SUPPORT
#include "disk_manager.h"
#else
#define MAX_USB_DISK_NUM    16
#define MAX_SD_DISK_NUM     1
#define MAX_IDE_DISK_NUM    1
#endif
#include "menus_root.h"
#include "win_disk_operate.h"
#include "win_disk_info.h"
#include "ctrl_key_proc.h"
#include "control.h"

/*******************************************************************************
*   data definition
*******************************************************************************/
#define MAX_DEVICE_NS   10
#define MAX_PARTITIONS_IN_DISK  16

#define DEVICE_TYPE_SUPPORT 4 // HDD, USB, SD, SATA
#define DEVICE_PAGE_NUM 2//show 2 device in 1 page!

#define WIN_DISK_INFO_WIDTH_DRAW_4 4
#define WIN_DISK_INFO_PARTITION_ITEM_WIDTH_8 8
#define WIN_DISK_INFO_DIR_NAME_LEN_3 3

#define VACT_FORMAT (VACT_PASS + 1)
#define VACT_PVR_SELECT (VACT_PASS + 2)

#ifndef SD_UI
#define INFO_L  CON_L + 4
#define INFO_T  340//195//200
#define INFO_H  35//24
#define INFO_V_GAP 2
#define INFO_N_W 250//150
#define INFO_C_W 250//150
#define INFO_H_GAP  50
#else
#define INFO_L  CON_L + 4
#define INFO_T  210//195//200
#define INFO_H  24//24
#define INFO_V_GAP 2
#define INFO_N_W 150//150
#define INFO_C_W 150//150
#define INFO_H_GAP  100
#endif

struct dev_info
{
    unsigned char   part_type;
    char dev_name[16];
    unsigned long   part_start; //in sectors
    unsigned long   part_size;  //in sectors
    unsigned long   part_sector_size; //in bytes

	char vol_name[16];
	char fs_name[16];
	unsigned long long	total_size; //in bytes
	unsigned long long	free_size;  //in bytes
	uint8 overflow;           // default is 0; 1 means over_2T 
	unsigned long long part_start_ex; //extend for over 2T HDD
	unsigned long long part_size_ex;//extend for over 2T HDD
};

struct dev_manager
{
    UINT8 num;
    UINT8 focus;
    UINT8 top;
    UINT8 page;
    struct
    {
        BOOL valid;
        char name[16];
        UINT8 num;
        UINT8 focus;
        struct dev_info part[MAX_PARTITIONS_IN_DISK];
    }dev[MAX_DEVICE_NS];
};

BOOL                        need_init_mp = TRUE;
static struct dev_manager   g_dev_info;
static UINT16               str_disk_title[5][64];
static BOOL                 b_update_part_info = FALSE;

/*******************************************************************************
*   Objects definition
*******************************************************************************/

static CONTAINER partitions_item_con[DEVICE_PAGE_NUM][MAX_PARTITIONS_IN_DISK];
static TEXT_FIELD partitions_item_txt[DEVICE_PAGE_NUM][MAX_PARTITIONS_IN_DISK];

static VACTION usb_state_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT usb_state_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION usb_disk_item_con_keymap(POBJECT_HEAD pobj, UINT32 key);
static PRESULT usb_disk_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);
static void update_pvr_device_bmp(void);
static void win_disk_info_update_focus_device(void);

#define TXT_INF_SH_IDX      WSTL_BUTTON_01_HD//WSTL_BUTTON_01

#define LDEF_CON(root, var_con,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,conobj,focus_id)     \
    DEF_CONTAINER(var_con,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, WSTL_DSK_CON,WSTL_DSK_CON,WSTL_DSK_CON,CON_GRY_IDX,  \
    usb_disk_item_con_keymap,usb_disk_item_con_callback,  \
    conobj, focus_id,0)

#define LDEF_DISK_TITLE_TXT(root,var_txt,nxt_obj,l,t,w,h,res_id,str)\
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_DISK_TITLE,WSTL_DISK_TITLE,WSTL_DISK_TITLE,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_TOP, 10,0,res_id,str)

#define LDEF_TXT_TITLE(root,var_txt,nxt_obj,l,t,w,h,str)      \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_INF_SH_IDX,TXT_INF_SH_IDX,TXT_INF_SH_IDX,TXT_INF_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_TXT_INFO(root,var_txt,nxt_obj,l,t,w,h,str)       \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_INF_SH_IDX,TXT_INF_SH_IDX,TXT_INF_SH_IDX,TXT_INF_SH_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT| C_ALIGN_VCENTER, 0,0,0,str)

#define LDEF_BMP(root,varbmp,nxt_obj,l,t,w,h,sh,bmp_id)       \
    DEF_BITMAP(varbmp,root,nxt_obj,C_ATTR_ACTIVE,0, \
    0, 0, 0, 0, 0, l,t,w,h, sh,sh,sh,sh,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,bmp_id)

#define LDEF_LISTBAR(root,var_scb,page,l,t,w,h)  \
    DEF_SCROLLBAR(var_scb, &root, NULL, C_ATTR_ACTIVE, 0, \
        0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
        NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
        0, 10, w, h - 20, 100, 1)

#define LDEF_OL(root,var_ol,nxt_obj,l,t,w,h,style,dep,count,flds,sb,mark,selary)  \
  DEF_OBJECTLIST(var_ol,&root,nxt_obj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_SH_IDX,LST_SH_IDX,LST_SH_IDX,   \
    usb_state_con_keymap,usb_state_con_callback,  \
    flds,sb,mark,style,dep,count,selary)

#ifndef SD_UI
#undef CON_H
#define CON_H 85//64
#undef CON_GAP
#define CON_GAP 4
#undef CON_W
#define CON_W (W_W - 55)

#define DISK_TITLE_OFFSET_TOP   4
#define DISK_TITLE_OFFSET_LEFT  4
#define DISK_TITLE_WIDTH        (CON_W - 8)
#define DISK_TITLE_HEIGHT       35//24

#define SCB_L (CON_L + CON_W)
#define SCB_T CON_T
#define SCB_W 12//20
#define SCB_H (CON_H * 2 + CON_GAP)
#else
#undef CON_H
#define CON_H 64//64
#undef CON_GAP
#define CON_GAP 4
#undef CON_W
#define CON_W (W_W - 40)
#undef CON_T
#define CON_T (W_T +8)

#define DISK_TITLE_OFFSET_TOP   4
#define DISK_TITLE_OFFSET_LEFT  4
#define DISK_TITLE_WIDTH        (CON_W - 8)
#define DISK_TITLE_HEIGHT       24//24

#define SCB_L (CON_L + CON_W)
#define SCB_T CON_T
#define SCB_W 20
#define SCB_H (CON_H * 2 + CON_GAP)
#endif

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_01_8BIT
#define LIST_BAR_MID_RECT_IDX   WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX      WSTL_SCROLLBAR_02_8BIT
#define LST_SH_IDX  WSTL_NOSHOW_IDX

static CONTAINER *p_disk_con[] =
{
    &disk_item_con1,
    &disk_item_con2,
};

LDEF_LISTBAR(disk_info_ol,disk_info_scb,DEVICE_PAGE_NUM,SCB_L,SCB_T, SCB_W, SCB_H)

#define LIST_STYLE (LIST_VER | LIST_SINGLE_SLECT | LIST_ITEMS_NOCOMPLETE | \
             LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_disk_info, disk_info_ol, NULL, CON_L, CON_T, CON_W, SCB_H, \
    LIST_STYLE, DEVICE_PAGE_NUM, 0, p_disk_con, &disk_info_scb,NULL,NULL)

#define PART_CON_OFFSET_TOP  (DISK_TITLE_OFFSET_TOP + DISK_TITLE_HEIGHT + 2)
#define PART_CON_OFFSET_LEFT  DISK_TITLE_OFFSET_LEFT
#define PART_CON_WIDTH        DISK_TITLE_WIDTH
#define PART_CON_HEIGHT       28

LDEF_CON(&g_win_disk_info, disk_item_con1, NULL, 1, 1, 1, 1, 1, CON_L, \
        CON_T + (CON_H + CON_GAP) * 0, CON_W, CON_H, &disk_item_title_txt1, 0x11)
LDEF_CON(&g_win_disk_info, disk_item_con2, NULL, 2, 2, 2, 2, 2, CON_L, \
        CON_T + (CON_H + CON_GAP) * 1, CON_W, CON_H, &disk_item_title_txt2, 0x21)

LDEF_DISK_TITLE_TXT(&disk_item_con1, disk_item_title_txt1, NULL, \
                        CON_L + DISK_TITLE_OFFSET_LEFT, \
                        CON_T + (CON_H + CON_GAP) * 0 + DISK_TITLE_OFFSET_TOP, \
                        DISK_TITLE_WIDTH, \
                        DISK_TITLE_HEIGHT, \
                        0, (UINT16 *)str_disk_title[0])
LDEF_DISK_TITLE_TXT(&disk_item_con2, disk_item_title_txt2, NULL, \
                        CON_L + DISK_TITLE_OFFSET_LEFT, \
                        CON_T + (CON_H + CON_GAP) * 1 + DISK_TITLE_OFFSET_TOP, \
                        DISK_TITLE_WIDTH, \
                        DISK_TITLE_HEIGHT, \
                        0, (UINT16 *)str_disk_title[1])

DEF_CONTAINER(g_win_disk_info,NULL,NULL,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, W_L,W_T,W_W,W_H, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    usb_state_con_keymap,usb_state_con_callback,  \
    NULL, 0,0)

LDEF_TXT_TITLE(&g_win_disk_info,disk_detail_title_txt,NULL,0,0,0,0, (UINT16 *)str_disk_title[3])
LDEF_TXT_INFO(&g_win_disk_info,disk_detail_info_txt,NULL,0,0,0,0, (UINT16 *)str_disk_title[4])

//LDEF_BMP(NULL, dvr_disk_image, NULL, 0, 0, 19, 19, WSTL_MIXBACK_BLACK_IDX, IM_MEDIAPLAY_DOT_01)
LDEF_BMP(&g_win_disk_info, pvr_bmp, NULL, 0, 0, 0, 0, WSTL_MIX_BMP_HD, 0) //WSTL_MIXBACK_BLACK_IDX

static TEXT_FIELD *p_disk_title_txt[] =
{
    &disk_item_title_txt1,
    &disk_item_title_txt2
};

static struct help_item_resource  disk_helpinfo[] =
{
    {IM_EPG_COLORBUTTON_YELLOW, RS_FORMAT},
    {IM_EPG_COLORBUTTON_BLUE,   RS_DVR_SET},
};
#define HELP_CNT    (sizeof(disk_helpinfo)/sizeof(struct help_item_resource))

/****************************************************************************
* sub functions
****************************************************************************/
static void convert_size_to_str(unsigned long long size, char *str, int str_size);
static int _part_compare( const void *a, const void *b );
static int update_dev_info(void);
static void update_disk_ui(BOOL disk_change);
//static void do_format(int id);
static void update_disk_detail_info(struct dev_info *pdisk);
static struct dev_info *get_focus_device(void);
static PRESULT usb_state_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2);

static void convert_size_to_str(unsigned long long size, char *str, int str_size)
{
    unsigned long sz_mask [] = {1024 * 1024 * 1024, 1024 * 1024, 1024, 1};
    char *szs[] = {"G", "M", "K", 0};
    int int_part=0;
    int dec_part=0;

    strncpy(str, "", str_size-1);
    str[str_size-1] = 0;

    unsigned int i=0;

    if(0 == size)
    {
        strncpy(str, "0 bytes", str_size-1);
        str[str_size-1] = 0;
        return;
    }

    for(i = 0; i < ARRAY_SIZE(sz_mask); i++)
    {

        if(size >= sz_mask[i])
        {
            int_part = (int)(size / sz_mask[i]);
            size %= sz_mask[i];
            if(sz_mask[i] > 1)
            {
                dec_part = (int)(size * 100 / sz_mask[i]);
                snprintf(str, 64,"%d.%02d%s bytes", int_part, dec_part, szs[i]);
            }
            else
            {
                dec_part = 0;
                snprintf(str, 64, "%d bytes", int_part);
            }

            break;
        }
    }

}

static int _part_compare( const void *a, const void *b )
{
	struct dev_info *pdev1 =  (struct dev_info *)a;
	struct dev_info *pdev2 =  (struct dev_info *)b;
	if(0 == pdev1->overflow)
	{
		if(pdev1->part_start > pdev2->part_start)
		{
			return 1;
		}
		else if(pdev1->part_start < pdev2->part_start)
		{
			return -1;
		}
		else
		{ 
			return 0;
		}
	}
	else
	{
	    if(pdev1->part_start_ex> pdev2->part_start_ex)
		{
			return 1;
	    }
		else if(pdev1->part_start_ex < pdev2->part_start_ex)
		{
			return -1;
	    }
		else
		{ 
			return 0;
		}
	}
}

#if 0
PRESULT ap_pvr_detach(char *mount_name)
{
    api_pvr_clear_up_all();
}
#endif

#ifdef DISK_MANAGER_SUPPORT
static BOOL api_pvr_partition_change(const char *mount_name, UINT8 partition_mode)
{
    return (0 == pvr_change_part(mount_name, partition_mode)) ? TRUE : FALSE;
}
#else
static BOOL api_pvr_partition_change(const char *mount_name, UINT8 partition_mode)
{
    BOOL ret = TRUE;
    struct pvr_register_info pvr_reg_info;
    SYSTEM_DATA* sys_data;
    enum PVR_DISKMODE cur_pvr_mode = PVR_DISK_INVALID;
    char rec_disk[16] = {0};
    char tms_disk[16] = {0};
    char string[16] = {0};
    struct dvr_hdd_info hdd_info;
    SYSTEM_DATA* p_sys_data = sys_data_get();

#ifndef PVR3_SUPPORT
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
    win_compopup_set_msg("Changing PVR partition, please wait", NULL, 0);
    win_compopup_open();
    api_pvr_clear_up_all();
    osal_task_sleep(1000);
    strncpy(string, mount_name, 15);
    storage_add_pvr_root(string, 16);
    pvr_set_root_dir_name(string);
    pvr_register(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN);
    osal_task_sleep(1000);
    win_msg_popup_close();

    api_pvr_adjust_tms_space();
    if(p_sys_data->timeshift == 0)
        pvr_set_tms_size(0);
    else if(hdd_info.tms_size == 0)
        pvr_set_tms_size(hdd_info.free_size);
    pvr_get_hdd_info(&hdd_info);
    api_pvr_check_level(&hdd_info);
#else //need debug under new PVR
    BOOL register_part = FALSE;
    cur_pvr_mode = pvr_get_cur_mode(rec_disk,sizeof(rec_disk), tms_disk,sizeof(tms_disk));
    MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
    strncpy(pvr_reg_info.mount_name, mount_name, 15);
    pvr_reg_info.disk_usage = partition_mode;
    pvr_reg_info.sync = 1;
    sys_data = sys_data_get();
    switch(partition_mode)
    {
        case PVR_REC_AND_TMS_DISK:
#if 1
            if (cur_pvr_mode == PVR_DISK_REC_AND_TMS)
            {
                pvr_detach_part(rec_disk, PVR_REC_AND_TMS_DISK);
            }
            else
            {
                pvr_detach_part(rec_disk, PVR_REC_ONLY_DISK);
                pvr_detach_part(tms_disk, PVR_TMS_ONLY_DISK);
            }
#else
            if((cur_pvr_mode == PVR_DISK_REC_AND_TMS) && (!STRCMP(rec_disk, mount_name)))
            {
                break;
            }
            ap_pvr_detach(rec_disk);
            ap_pvr_detach(tms_disk);
            //link_device(focus_dev->vol_name, USB_MOUNT_NAME);
/*                      fs_unlink(USB_MOUNT_NAME);
            if(fs_symlink(focus_dev->vol_name, USB_MOUNT_NAME) < 0)
            {
                win_compopup_smsg_restoreback();
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
//              win_compopup_set_frame(200, 160, 200, 100);
                win_compopup_set_msg_ext("pvr select fail!",NULL,0);
                win_compopup_open_ext(&back_saved);
                osal_task_sleep(2000);
                win_compopup_smsg_restoreback();
                break;
            } */
#endif
            pvr_reg_info.init_list = pvr_reg_info.disk_usage != PVR_TMS_ONLY_DISK;
#ifndef PVR_SPEED_PRETEST_DISABLE
            pvr_reg_info.check_speed = 1;
#endif
            osal_task_sleep(1000); // wait for pvr cleanup finished
            if(SUCCESS != pvr_register((UINT32)&pvr_reg_info, 1))
            {
                ret = FALSE;
            }
            strncpy(sys_data->rec_disk, pvr_reg_info.mount_name, 15);
            strncpy(sys_data->tms_disk, pvr_reg_info.mount_name, 15);
            sys_data_save(0);
            break;
        case PVR_REC_ONLY_DISK:
#if 1
            if ((STRLEN(rec_disk) > 0) && !STRCMP(rec_disk, mount_name)) // same partition
            {
                if ((cur_pvr_mode == PVR_DISK_ONLY_REC) ||
                    (cur_pvr_mode == PVR_DISK_REC_WITH_TMS))
                {
                    break;
                }
                else if (cur_pvr_mode == PVR_DISK_REC_AND_TMS)
                {
                    pvr_set_disk_use(PVR_REC_ONLY_DISK, mount_name);
                }
                else
                {
                    libc_printf("rec_disk: %s, mount_name: %s, mode: %d, %d\n", \
                                rec_disk, mount_name, cur_pvr_mode, partition_mode);
                    register_part = TRUE;
                    //SDBBP();
                }
            }
            else // diff partition
            {
                register_part = TRUE;
                if (cur_pvr_mode == PVR_DISK_REC_AND_TMS)
                {
                    pvr_set_disk_use(PVR_TMS_ONLY_DISK, tms_disk);
                }

                if ((cur_pvr_mode == PVR_DISK_ONLY_REC) ||
                    (cur_pvr_mode == PVR_DISK_REC_WITH_TMS))
                {
                    pvr_detach_part(rec_disk, PVR_REC_ONLY_DISK);
                }

                if ((!STRCMP(tms_disk, mount_name)) &&
                    ((cur_pvr_mode == PVR_DISK_ONLY_TMS) ||
                     (cur_pvr_mode == PVR_DISK_REC_WITH_TMS)))
                {
                    pvr_detach_part(tms_disk, PVR_TMS_ONLY_DISK);
                }
            }
#else
            if(((cur_pvr_mode == PVR_DISK_REC_WITH_TMS) || (cur_pvr_mode == PVR_DISK_ONLY_REC))
            && (!STRCMP(rec_disk, mount_name)))
            {
                break;
            }
            if(cur_pvr_mode == PVR_DISK_REC_AND_TMS)
            {
                pvr_set_disk_use(PVR_TMS_ONLY_DISK, rec_disk);
            }
            else
            {
                ap_pvr_detach(rec_disk);
            }
            if(!STRCMP(tms_disk, mount_name))
            {
                ap_pvr_detach(tms_disk);
                // automaitc move pvr to other disk!
                //pvr_reg_info.disk_usage = PVR_REC_AND_TMS_DISK;
            }
#endif
            if (register_part)
            {
                pvr_reg_info.init_list = pvr_reg_info.disk_usage != PVR_TMS_ONLY_DISK;
#ifndef PVR_SPEED_PRETEST_DISABLE
                pvr_reg_info.check_speed = 1;
#endif
                osal_task_sleep(1000); // wait for pvr cleanup finished
                if(SUCCESS != pvr_register((UINT32)&pvr_reg_info, 1))
                {
                    ret = FALSE;
                }
            }
            strncpy(sys_data->rec_disk, pvr_reg_info.mount_name, 15);
            if(!STRCMP(sys_data->tms_disk, pvr_reg_info.mount_name))
                strncpy(sys_data->tms_disk, "", 15);
            sys_data_save(0);
            break;
        case PVR_TMS_ONLY_DISK:
#if 1
            if ((STRLEN(tms_disk) > 0) && !STRCMP(tms_disk, mount_name)) // same partition
            {
                if ((cur_pvr_mode == PVR_DISK_ONLY_TMS) ||
                    (cur_pvr_mode == PVR_DISK_REC_WITH_TMS))
                {
                    break;
                }
                else if (cur_pvr_mode == PVR_DISK_REC_AND_TMS)
                {
                    pvr_set_disk_use(PVR_TMS_ONLY_DISK, mount_name);
                }
                else
                {
                    libc_printf("rec_disk: %s, mount_name: %s, mode: %d, %d\n", \
                                rec_disk, mount_name, cur_pvr_mode, partition_mode);
                    register_part = TRUE;
                    //SDBBP();
                }
            }
            else // diff partition
            {
                register_part = TRUE;
                if (cur_pvr_mode == PVR_DISK_REC_AND_TMS)
                {
                    pvr_set_disk_use(PVR_REC_ONLY_DISK, rec_disk);
                }

                if ((cur_pvr_mode == PVR_DISK_ONLY_TMS) ||
                    (cur_pvr_mode == PVR_DISK_REC_WITH_TMS))
                {
                    pvr_detach_part(tms_disk, PVR_TMS_ONLY_DISK);
                }

                if ((!STRCMP(rec_disk, mount_name)) &&
                    ((cur_pvr_mode == PVR_DISK_ONLY_REC) ||
                     (cur_pvr_mode == PVR_DISK_REC_WITH_TMS)))
                {
                    pvr_detach_part(rec_disk, PVR_REC_ONLY_DISK);
                }
            }

#else
            if(((cur_pvr_mode == PVR_DISK_REC_WITH_TMS) || (cur_pvr_mode == PVR_DISK_ONLY_TMS))
             && (!STRCMP(tms_disk, mount_name)))
            {
                break;
            }
            if(cur_pvr_mode == PVR_DISK_REC_AND_TMS)
            {
                pvr_set_disk_use(PVR_REC_ONLY_DISK, rec_disk);
            }
            else
            {
                ap_pvr_detach(tms_disk);
            }
            if(!STRCMP(rec_disk, mount_name))
            {
                ap_pvr_detach(rec_disk);
                // automaitc move pvr to other disk!
                //pvr_reg_info.disk_usage = PVR_REC_AND_TMS_DISK;
            }
#endif
            if (register_part)
            {
                pvr_reg_info.init_list = pvr_reg_info.disk_usage != PVR_TMS_ONLY_DISK;
#ifndef PVR_SPEED_PRETEST_DISABLE
                pvr_reg_info.check_speed = 1;
#endif
                osal_task_sleep(1000); // wait for pvr cleanup finished
                if(SUCCESS != pvr_register((UINT32)&pvr_reg_info, 1))
                {
                    ret = FALSE;
                }
            }
            strncpy(sys_data->tms_disk, pvr_reg_info.mount_name, 15);
            if(!STRCMP(sys_data->rec_disk, pvr_reg_info.mount_name))
                strncpy(sys_data->rec_disk, "", 15);
            sys_data_save(0);
            break;
        default:
            break;
    }
    pvr_get_hdd_info(&hdd_info);
    api_pvr_check_level(&hdd_info);
#endif
    return ret;
}
#endif

#ifndef PVR3_SUPPORT
static char m_mount_name[16];
char *pvr_get_mount_name(void)
{
    MEMSET(m_mount_name, 0, 16);
    MEMCPY(m_mount_name, pvr_get_root_dir_name(), 9);

    return m_mount_name;
};

INT8 pvr_get_disk_mode(char *mount_name)
{
    INT8 ret = -1;
    BOOL rec_disk = FALSE, tms_disk = FALSE;

    rec_disk = (STRCMP(mount_name, pvr_get_mount_name()) == 0);
    tms_disk = (STRCMP(mount_name, pvr_get_mount_name()) == 0);
    if(rec_disk && tms_disk)
        ret = PVR_REC_AND_TMS_DISK;
    else if(rec_disk)
        ret = PVR_REC_ONLY_DISK;
    else if(tms_disk)
        ret = PVR_TMS_ONLY_DISK;

    return ret;
}


#endif /* PVR3_SUPPORT */

static int update_dev_info(void)
{
    struct dev_info *pinfo = NULL;
    struct device_geometry geo;
    struct statvfs  stvfs;
    int fd=0;
    int fdir=0;
    int i=0;
    int j=0;
    int ret=0;
    char direbuffer[sizeof(struct dirent)+32] = {0};
    struct dirent *dire = (struct dirent *)direbuffer;
    int dev_idx=0;
    int part_idx=0;
    char dev_path[16] = {0};
    int max_dev_num=0;

    MEMSET(&geo, 0, sizeof(struct device_geometry));
    MEMSET(&stvfs, 0, sizeof(struct statvfs));
    MEMSET(&g_dev_info, 0, sizeof(struct dev_manager));
    g_dev_info.page = DEVICE_PAGE_NUM;

    // search by type
    dev_idx = 0;
    for (i = 0; i < DEVICE_TYPE_SUPPORT; i++)
    {
        switch (i)
        {
            case 0:
                strncpy(dev_path, "/dev/hda", 15);
                max_dev_num = MAX_IDE_DISK_NUM;
                break;
            case 1:
                strncpy(dev_path, "/dev/sda", 15);
                max_dev_num = MAX_SD_DISK_NUM;
                break;
            case 2:
                strncpy(dev_path, "/dev/uda", 15);
                max_dev_num = MAX_USB_DISK_NUM;
                break;
            case 3:
                strncpy(dev_path, "/dev/sha", 15);
                max_dev_num = MAX_USB_DISK_NUM;
                break;
            default:
                strncpy(dev_path, "", 15);
                max_dev_num = 0;
                break;
        }

        for (j = 0; j < max_dev_num; j++)
        {
            strncpy(g_dev_info.dev[dev_idx].name, dev_path, 15);
            g_dev_info.dev[dev_idx].name[7] += j;

			fd = fs_open(g_dev_info.dev[dev_idx].name, O_RDONLY, 0777);
			if(fd < 0)
			{
				continue;
			}
			
			memset(&geo, 0, sizeof(device_geometry));
			ret = fs_ioctl(fd, IOCTL_GET_DEVICE_GEOMETRY, &geo, sizeof(struct device_geometry));
			fs_close(fd);
			if(ret < 0)
			{
				continue;
			}

			g_dev_info.dev[dev_idx].valid = TRUE;
			strncpy(g_dev_info.dev[dev_idx].part[0].dev_name, g_dev_info.dev[dev_idx].name, 15);
			g_dev_info.dev[dev_idx].part[0].part_start = 0;
			g_dev_info.dev[dev_idx].part[0].part_start_ex = g_dev_info.dev[dev_idx].part[0].part_start;
			g_dev_info.dev[dev_idx].part[0].part_size = geo.sector_count;
			g_dev_info.dev[dev_idx].part[0].part_size_ex = g_dev_info.dev[dev_idx].part[0].part_size;
			g_dev_info.dev[dev_idx].part[0].part_sector_size = geo.bytes_per_sector;
            if(geo.overflow)
            {
			    g_dev_info.dev[dev_idx].part[0].overflow = 1;
			    g_dev_info.dev[dev_idx].part[0].part_start_ex = 0;
			    g_dev_info.dev[dev_idx].part[0].part_size_ex = geo.sector_count;
            }
			part_idx = 0; // partitions info start from index 1
			//loop all the /dev to find the partition
			fdir = fs_opendir("/dev");
			while(fs_readdir(fdir, dire) > 0)
			{
				if((MEMCMP(&g_dev_info.dev[dev_idx].name[5], dire->d_name, 3) == 0) && (STRLEN(dire->d_name) > 3))
				{
					pinfo = &g_dev_info.dev[dev_idx].part[part_idx+1];// 1st part is total info
					MEMSET(pinfo, 0, sizeof(struct dev_info));
					STRCPY(pinfo->dev_name, "/dev/");
					strcat(pinfo->dev_name, dire->d_name);

					fd = fs_open(pinfo->dev_name, O_RDONLY, 0777);
					memset(&geo, 0, sizeof(device_geometry));
					ret = fs_ioctl(fd, IOCTL_GET_DEVICE_GEOMETRY, &geo, sizeof(struct device_geometry));
					fs_close(fd);
					
					pinfo->part_start = geo.start_sector;
					pinfo->part_start_ex = pinfo->part_start;
					pinfo->part_size = geo.sector_count;
					pinfo->part_size_ex = pinfo->part_size;
					pinfo->part_sector_size = geo.bytes_per_sector;
					if(geo.overflow)
					{
					    pinfo->part_start_ex = geo.start_sector_ex;
					    pinfo->part_size_ex = geo.sector_count;
					    pinfo->overflow = 1;
					}

                    //check if the dev has been mounted
                    strncpy(pinfo->vol_name, "/mnt/", 15);
                    strncat(pinfo->vol_name, dire->d_name, 16 - 1 - strlen(pinfo->vol_name));
                    ret = fs_statvfs(pinfo->vol_name, &stvfs);
                    if(ret < 0)
                    {
                        pinfo->vol_name[0] = 0;
                    }
                    else
                    {
                        if(STRCMP(pinfo->dev_name, stvfs.f_device_name) != 0)
                        {
                            pinfo->vol_name[0] = 0;
                        }
                        else
                        {
                            strncpy(pinfo->fs_name, stvfs.f_fsh_name, 15);
                            pinfo->total_size = (unsigned long long)stvfs.f_blocks * stvfs.f_frsize;
                            pinfo->free_size  = (unsigned long long)stvfs.f_bfree * stvfs.f_frsize;
                        }
                    }
                    part_idx++;
                }
            }

            fs_closedir(fdir);
            g_dev_info.dev[dev_idx].num = part_idx;
            g_dev_info.dev[dev_idx].focus = 1;
            if(g_dev_info.dev[dev_idx].num > 1) //sort the partition
            {
                qsort(&g_dev_info.dev[dev_idx].part[1], part_idx, sizeof(struct dev_info), _part_compare);
            }

            dev_idx++;
        }
    }

    g_dev_info.num = dev_idx;

    //reset to first device if some device removed
    if(g_dev_info.top >= g_dev_info.num)
    {
        g_dev_info.top = 0;
    }

    return 0;
}

static void update_disk_ui(BOOL disk_change)
{
    UINT32 i=0;
    int j=0;
    int part_ns=0;
    char tmp_str[64] = {0};
    char id = 0;
    int wd = 0;
    int row_w = 0;
    int edge_w = 0;
    int rest_w =0;
    int l=0;
    int w=0;
    unsigned long long disk_size = 0;

    MEMSET(&partitions_item_con, 0, sizeof(partitions_item_con));
    MEMSET(&partitions_item_txt, 0, sizeof(partitions_item_txt));
    g_win_disk_info.p_next_in_cntn = NULL;
    for(i=0; i<g_dev_info.page; i++)
    {
        if (((i + g_dev_info.top) >= MAX_DEVICE_NS) || (i >= ARRAY_SIZE(p_disk_con)))
        {
            break;
        }

        //to check if the disk is exists
        if(FALSE == g_dev_info.dev[i+g_dev_info.top].valid)
        {
            p_disk_con[i]->head.b_attr = C_ATTR_INACTIVE;
            osd_set_text_field_content(p_disk_title_txt[i], STRING_ANSI, (UINT32)"");
            break;
        }
        p_disk_con[i]->head.b_attr = C_ATTR_ACTIVE;

        //to cal the partition size;
        part_ns = g_dev_info.dev[i+g_dev_info.top].num;
        if(part_ns > 0)
        {
            //every partition con have 2 edge, every edge has 2 pixel
            row_w = PART_CON_WIDTH;
            edge_w = 2 * (part_ns + 1); // 2 * 2 * part_ns;
            rest_w = row_w - edge_w;
            l=0;
            w=0;
            disk_size = g_dev_info.dev[i+g_dev_info.top].part[0].part_size_ex;

            //to draw every partition
            for(j=1; j<=part_ns; j++)
            {
                partitions_item_con[i][j].head.b_type = OT_CONTAINER;
                partitions_item_con[i][j].head.b_attr = C_ATTR_ACTIVE;
                partitions_item_con[i][j].head.p_root = (POBJECT_HEAD)p_disk_con[i];
                partitions_item_con[i][j].head.b_id = (i + 1) << 4 | j;

                if(1 == j)
                {
                    partitions_item_con[i][j].head.b_left_id = (i + 1) << 4 | part_ns;
                }
                else
                {
                    partitions_item_con[i][j].head.b_left_id = (i + 1) << 4 | (j - 1);
                }

                if( j == part_ns)
                {
                    partitions_item_con[i][j].head.b_right_id = (i + 1) << 4 | 1;
                }
                else
                {
                    partitions_item_con[i][j].head.b_right_id = (i + 1) << 4 | (j + 1);
                }
                partitions_item_con[i][j].head.b_up_id = p_disk_con[i]->head.b_id;
                partitions_item_con[i][j].head.b_down_id = p_disk_con[i]->head.b_id;

                //call the rect
                l = p_disk_con[i]->head.frame.u_left + PART_CON_OFFSET_LEFT + 2 * j  +  \
                    (unsigned long long)g_dev_info.dev[i+g_dev_info.top].part[j].part_start_ex * rest_w / disk_size;
                w = (unsigned long long)g_dev_info.dev[i+g_dev_info.top].part[j].part_size_ex * rest_w / disk_size + 2;
                if(j > 1) // to avoid interspace between partitions according to div error!
                {
                    l = partitions_item_con[i][j-1].head.frame.u_left + partitions_item_con[i][j-1].head.frame.u_width;
                }
                partitions_item_con[i][j].head.frame.u_left = l;
                partitions_item_con[i][j].head.frame.u_top = p_disk_con[i]->head.frame.u_top + PART_CON_OFFSET_TOP;
                partitions_item_con[i][j].head.frame.u_height = PART_CON_HEIGHT;
                partitions_item_con[i][j].head.frame.u_width = w;
                //set the style
                if(0 == g_dev_info.dev[i+g_dev_info.top].part[j].vol_name[0])
                {
                    partitions_item_con[i][j].head.style.b_show_idx = WSTL_NONVOL_CON;
                    partitions_item_con[i][j].head.style.b_hlidx   = WSTL_NONVOL_CON_HL;
                    partitions_item_con[i][j].head.style.b_sel_idx  = WSTL_NONVOL_CON_HL;
                    partitions_item_con[i][j].head.style.b_gray_idx = WSTL_NONVOL_CON;

                }
                else
                {
                    partitions_item_con[i][j].head.style.b_show_idx = WSTL_VOL_CON;
                    partitions_item_con[i][j].head.style.b_hlidx   = WSTL_VOL_CON_HL;
                    partitions_item_con[i][j].head.style.b_sel_idx  = WSTL_VOL_CON_HL;
                    partitions_item_con[i][j].head.style.b_gray_idx = WSTL_VOL_CON;

                    if(partitions_item_con[i][j].head.frame.u_width > WIN_DISK_INFO_PARTITION_ITEM_WIDTH_8)
                    {
                        wd = 0;
                        if(g_dev_info.dev[i+g_dev_info.top].part[j].total_size > 0)
                        {
                            wd = (partitions_item_con[i][j].head.frame.u_width - 4) \
                                    * (g_dev_info.dev[i+g_dev_info.top].part[j].total_size \
                                    - g_dev_info.dev[i+g_dev_info.top].part[j].free_size) \
                                    / g_dev_info.dev[i+g_dev_info.top].part[j].total_size;
                        }
                        if(wd > WIN_DISK_INFO_WIDTH_DRAW_4)
                        {
                            //draw the textfield to show how many free space
                            partitions_item_txt[i][j].head.b_type = OT_TEXTFIELD;
                            partitions_item_txt[i][j].head.b_attr = C_ATTR_ACTIVE;
                            partitions_item_txt[i][j].head.p_root = (POBJECT_HEAD)&partitions_item_con[i][j];
                            partitions_item_txt[i][j].head.style.b_show_idx = WSTL_VOL_TXT;
                            partitions_item_txt[i][j].head.style.b_hlidx   = WSTL_VOL_TXT;
                            partitions_item_txt[i][j].head.style.b_sel_idx  = WSTL_VOL_TXT;
                            partitions_item_txt[i][j].head.style.b_gray_idx = WSTL_VOL_TXT;

                            partitions_item_txt[i][j].head.frame.u_left = partitions_item_con[i][j].head.frame.u_left + 2;
                            partitions_item_txt[i][j].head.frame.u_top  = partitions_item_con[i][j].head.frame.u_top + 2;
                            partitions_item_txt[i][j].head.frame.u_height = partitions_item_con[i][j].head.frame.u_height - 4;
                            partitions_item_txt[i][j].head.frame.u_width = (partitions_item_con[i][j].head.frame.u_width - 4) \
                                                                  * (g_dev_info.dev[i+g_dev_info.top].part[j].total_size \
                                                                  - g_dev_info.dev[i+g_dev_info.top].part[j].free_size) \
                                                                  / g_dev_info.dev[i+g_dev_info.top].part[j].total_size;

                            partitions_item_con[i][j].p_next_in_cntn = (POBJECT_HEAD)&partitions_item_txt[i][j];

                        }
                    }

                }

                if(j < part_ns)
                {
                    partitions_item_con[i][j].head.p_next = (POBJECT_HEAD)&partitions_item_con[i][j + 1];
                }
            }

            p_disk_title_txt[i]->head.p_next = (POBJECT_HEAD)&partitions_item_con[i][1];
            id = g_dev_info.dev[i+g_dev_info.top].part[0].dev_name[7] - 'a' + 'A';

            if('h' == g_dev_info.dev[i+g_dev_info.top].part[0].dev_name[5])
            {
                snprintf(tmp_str, 64, "IDE Disk %c", id);
            }
            else if('u' == g_dev_info.dev[i+g_dev_info.top].part[0].dev_name[5])
            {
                snprintf(tmp_str, 64, "USB Disk %c", id);
            }
            else if('s' == g_dev_info.dev[i+g_dev_info.top].part[0].dev_name[5])
            {
                if('d' == g_dev_info.dev[i+g_dev_info.top].part[0].dev_name[6])
                {
                    snprintf(tmp_str, 64, "SD Card %c", id);
                }
                else
                {
                    snprintf(tmp_str, 64, "SATA Disk %c", id);
                }
            }

            strncat(tmp_str, " : ", 3);
            convert_size_to_str((unsigned long long)g_dev_info.dev[i+g_dev_info.top].part[0].part_sector_size \
                * g_dev_info.dev[i+g_dev_info.top].part[0].part_size_ex, tmp_str + STRLEN(tmp_str), (64-strlen(tmp_str)));
            osd_set_text_field_content(p_disk_title_txt[i], STRING_ANSI, (UINT32)tmp_str);
            if((disk_change) || (p_disk_con[i]->focus_object_id > ((i + 1) << 4 | part_ns)))
            {
                p_disk_con[i]->focus_object_id = partitions_item_con[i][1].head.b_id;
            }
            p_disk_con[i]->p_next_in_cntn = (POBJECT_HEAD)p_disk_title_txt[i];

        }

        if(0 == i)
        {
            g_win_disk_info.p_next_in_cntn = (POBJECT_HEAD)p_disk_con[i];
            if(disk_change || (!g_win_disk_info.focus_object_id/* no highlight */))
            {
                g_win_disk_info.focus_object_id = p_disk_con[i]->head.b_id;
            }
        }
        else
        {
            p_disk_con[i - 1]->head.p_next = (POBJECT_HEAD)p_disk_con[i];
            p_disk_con[i - 1]->head.b_down_id = p_disk_con[i]->head.b_id;
            p_disk_con[i]->head.b_up_id = p_disk_con[i - 1]->head.b_id;
            p_disk_con[i]->head.p_next = NULL;
        }

    }
    g_win_disk_info.focus_object_id = (g_dev_info.focus - g_dev_info.top + 1);
}

void win_disk_info_update(BOOL disk_change)
{
    struct dev_info *focus_dev = NULL;
    UINT16 cnt=0;
    UINT16 page=0;
    UINT16 cur=0;
    OBJLIST *ol = NULL;
    UINT32 msg_code=0;

    if(disk_operate_opened)
    {
        msg_code=0;
        ap_vk_to_hk(0, V_KEY_EXIT, &msg_code);
        ap_send_msg(CTRL_MSG_SUBTYPE_KEY, msg_code, FALSE);
    }

    if(disk_change)
    {
        update_dev_info();
    }

    update_disk_ui(disk_change);
    if(menu_stack_get_top() == (POBJECT_HEAD)&g_win_disk_info)
    {
        focus_dev = get_focus_device();
        if(focus_dev != NULL)
        {
            ol = &disk_info_ol;
            cnt=0;
            page=0;
            cur=0;
            osd_track_object((POBJECT_HEAD)&g_win_disk_info, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            page = osd_get_obj_list_page(ol);
            cnt = g_dev_info.num;
            cur = osd_get_obj_list_cur_point(ol);
            if(cur >= cnt)
            {
                cur = 0;
            }

            osd_set_obj_list_cur_point(ol, cur);
            osd_set_obj_list_new_point(ol, cur);
            osd_set_obj_list_top(ol, g_dev_info.top);
            osd_set_obj_list_count(ol, g_dev_info.num);
            osd_set_obj_list_single_select(ol, INVALID_POS_NUM);
            osd_set_attr(&disk_info_scb, (cnt <= page) ? C_ATTR_HIDDEN : C_ATTR_ACTIVE);
            if(cnt > page)
            {
                osd_track_object((POBJECT_HEAD)ol,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            }

            update_disk_detail_info(focus_dev);
            update_pvr_device_bmp();
        }
        else
        {
            //get dev handle error!
            ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT,1,TRUE);
        }
    }
}

#ifdef PVR_MULTI_VOLUME_SUPPORT
void usb_state_set_str(char *str)
{
}
#endif

static void update_disk_detail_info(struct dev_info *pdisk)
{
    unsigned long long size = 0;
    TEXT_FIELD *txt = NULL;
    TEXT_FIELD *info = NULL;
    char str[64] = {0};
    char rec_disk[16] = {0};
    char tms_disk[16] = {0};
    struct dvr_hdd_info hdd_info;
    int row = 0;
    UINT16 i= 0;
    struct statvfs stfs;
    INT8 disk_pvr_mode = -1;
    int t_len = 0;
    int s_size = 64;

    txt = &disk_detail_title_txt;
    info = &disk_detail_info_txt;
    MEMSET(&hdd_info,0,sizeof(struct dvr_hdd_info));
    MEMSET(&stfs,0,sizeof(struct statvfs));
    /* volume */
    snprintf(str, 64, "[%s]", &pdisk->dev_name[5]);
    t_len = strlen(str);
    if(0 == pdisk->vol_name[0])
    {
        if ((s_size - t_len) > (int)strlen("unmounted"))
        {
            strncat(str, "unmounted", s_size - 1 - t_len);
        }
    }
    else
    {
        //fs_statvfs(pdisk->vol_name, &stfs);
        if(fs_statvfs(pdisk->vol_name, &stfs) < 0)
        {
            return;
        }
        if ((s_size - t_len) > (int)strlen(stfs.f_volume_name))
        {
            strncat(str, stfs.f_volume_name, s_size - 1 - t_len);
        }
    }
    osd_set_rect(&txt->head.frame, INFO_L, INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_N_W, INFO_H);
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"Volume:");
    osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);

    osd_set_rect(&info->head.frame, INFO_L + (INFO_N_W + INFO_H_GAP), INFO_T \
            + (INFO_H + INFO_V_GAP)*row, INFO_C_W, INFO_H);
    osd_set_text_field_content(info, STRING_ANSI, (UINT32)str);
    osd_draw_object((POBJECT_HEAD)info, C_UPDATE_ALL);
    row++;

    // if partition has tms remain, need use pvr calculate the total size (include the tms recycle size)
    disk_pvr_mode = pvr_get_disk_mode(pdisk->vol_name);
	STRCPY(hdd_info.mount_name , pdisk->vol_name);
    if(((PVR_REC_AND_TMS_DISK == disk_pvr_mode) || (PVR_REC_ONLY_DISK == disk_pvr_mode) || (PVR_TMS_ONLY_DISK == disk_pvr_mode))
        && (pvr_get_hdd_info(&hdd_info)) && (hdd_info.total_size > 0))
    {
        pdisk->total_size = hdd_info.total_size;
        pdisk->free_size = hdd_info.free_size;
        pdisk->total_size *= 1024;
        pdisk->free_size *= 1024;
    }

    //total size
    if(0 == pdisk->vol_name[0])
    {
        size = (unsigned long long)pdisk->part_sector_size * pdisk->part_size_ex;
    }
    else
    {
        size = pdisk->total_size;
    }

    osd_set_rect(&txt->head.frame, INFO_L, INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_N_W, INFO_H);
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"Total Size:");
    osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);

    //convert_size_to_str(, str);
    convert_size_to_str(size, str, 64);

    osd_set_rect(&info->head.frame, INFO_L + (INFO_N_W + INFO_H_GAP), \
        INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_C_W, INFO_H);
    osd_set_text_field_content(info, STRING_ANSI, (UINT32)str);
    osd_draw_object((POBJECT_HEAD)info, C_UPDATE_ALL);
    row++;

    //free size
    if(0 == pdisk->vol_name[0])
    {
        size = (unsigned long long)pdisk->part_sector_size * pdisk->part_size_ex;
    }
    else
    {
        size = pdisk->free_size;
    }

    osd_set_rect(&txt->head.frame, INFO_L, INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_N_W, INFO_H);
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"Free Size:");
    osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);

    convert_size_to_str(size, str, 64);

    osd_set_rect(&info->head.frame, INFO_L + (INFO_N_W + INFO_H_GAP), \
        INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_C_W, INFO_H);
    osd_set_text_field_content(info, STRING_ANSI, (UINT32)str);
    osd_draw_object((POBJECT_HEAD)info, C_UPDATE_ALL);
    row++;

    // clear next 3 items!!
    for(i=0; i<3; i++)
    {
        osd_set_rect(&txt->head.frame, INFO_L, INFO_T + (INFO_H + INFO_V_GAP)*(row+i), INFO_N_W, INFO_H);
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"");
        osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);
        osd_set_rect(&info->head.frame, INFO_L + (INFO_N_W + INFO_H_GAP), \
            INFO_T + (INFO_H + INFO_V_GAP)*(row+i), INFO_C_W, INFO_H);
        osd_set_text_field_content(info, STRING_ANSI, (UINT32)"");
        osd_draw_object((POBJECT_HEAD)info, C_UPDATE_ALL);
    }

    //rec size
    pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));
    if(STRLEN(rec_disk) && (!STRCMP(rec_disk, pdisk->vol_name)) && (pvr_get_hdd_info(&hdd_info)))
    {
        size = (unsigned long long)hdd_info.rec_size * 1024;

        osd_set_rect(&txt->head.frame, INFO_L, INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_N_W, INFO_H);
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"Rec Size:");
        osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);

        convert_size_to_str(size, str, 64);

        osd_set_rect(&info->head.frame, INFO_L + (INFO_N_W + INFO_H_GAP), \
            INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_C_W, INFO_H);
        osd_set_text_field_content(info, STRING_ANSI, (UINT32)str);
        osd_draw_object((POBJECT_HEAD)info, C_UPDATE_ALL);
        row++;
    }
    if(STRLEN(tms_disk) && (!STRCMP(tms_disk, pdisk->vol_name)) && (pvr_get_hdd_info(&hdd_info)))
    {
        //tms size
        size = (unsigned long long)hdd_info.tms_size * 1024;

        osd_set_rect(&txt->head.frame, INFO_L, INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_N_W, INFO_H);
        osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"TMS Size:");
        osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);

        convert_size_to_str(size, str, 64);

        osd_set_rect(&info->head.frame, INFO_L + (INFO_N_W + INFO_H_GAP), \
            INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_C_W, INFO_H);
        osd_set_text_field_content(info, STRING_ANSI, (UINT32)str);
        osd_draw_object((POBJECT_HEAD)info, C_UPDATE_ALL);
        row++;
    }

    //file system
    if(0 == pdisk->vol_name[0])
    {
        strncpy(str, "unknown", 63);
    }
    else
    {
        strncpy(str, pdisk->fs_name, 63);
    }

    osd_set_rect(&txt->head.frame, INFO_L, INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_N_W, INFO_H);
    osd_set_text_field_content(txt, STRING_ANSI, (UINT32)"File System:");
    osd_draw_object((POBJECT_HEAD)txt, C_UPDATE_ALL);

    osd_set_rect(&info->head.frame, INFO_L + (INFO_N_W + INFO_H_GAP), \
        INFO_T + (INFO_H + INFO_V_GAP)*row, INFO_C_W, INFO_H);
    osd_set_text_field_content(info, STRING_ANSI, (UINT32)str);
    osd_draw_object((POBJECT_HEAD)info, C_UPDATE_ALL);
    row++;


}
static VACTION usb_disk_item_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act= PROC_PASS;

    switch(key)
    {
    case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
        break;
    default:
        act = VACT_PASS;
        break;
    }

    return act;
}

static PRESULT usb_disk_item_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact= 0;
    struct dev_info *focus_dev = NULL;

    switch(event)
    {
        case EVN_KEY_GOT:
            unact = (VACTION)(param1>>16);
            if(VACT_CURSOR_LEFT == unact)
            {
                g_dev_info.dev[g_dev_info.focus].focus--;
                if(0 == g_dev_info.dev[g_dev_info.focus].focus)
        {
                    g_dev_info.dev[g_dev_info.focus].focus = g_dev_info.dev[g_dev_info.focus].num;
        }
            }
            else if(VACT_CURSOR_RIGHT == unact)
            {
                g_dev_info.dev[g_dev_info.focus].focus++;
                if(g_dev_info.dev[g_dev_info.focus].focus > g_dev_info.dev[g_dev_info.focus].num)
        {
                    g_dev_info.dev[g_dev_info.focus].focus = 1;
        }
            }
            break;
        case EVN_ITEM_POST_CHANGE:
        case EVN_FOCUS_POST_GET:
            focus_dev = get_focus_device();
            if(focus_dev != NULL)
            {
                update_disk_detail_info(focus_dev);
                update_pvr_device_bmp();
            }
            break;
        default:
            break;

    }

    return ret;
}

static VACTION usb_state_con_keymap(POBJECT_HEAD pobj, UINT32 key)
{
    VACTION act = PROC_PASS;

    switch(key)
    {
    case V_KEY_UP:
        act = VACT_CURSOR_UP;
        break;
    case V_KEY_DOWN:
        act = VACT_CURSOR_DOWN;
        break;
    case V_KEY_LEFT:
        act = VACT_CURSOR_LEFT;
        break;
    case V_KEY_RIGHT:
        act = VACT_CURSOR_RIGHT;
        break;
    case V_KEY_EXIT:
    case V_KEY_MENU:
        act = VACT_CLOSE;
        break;
    case V_KEY_BLUE:
        act = VACT_PVR_SELECT;
        break;
    case V_KEY_YELLOW:
        act = VACT_FORMAT;
        break;
    default:
        act = VACT_PASS;
        break;
    }
    return act;
}

//return value
//PROC_LOOP ==> just return PROC_LOOP, FALSE ==> just break, TRUE ==> OK
static PRESULT win_disk_info_format(struct dev_info *focus_dev,UINT8 operate_result,BOOL *used_for_dvr)
{
    UINT8 back_saved= 0;
    PRESULT ret = PROC_LOOP;
    INT8 disk_pvr_mode = -1;
    BOOL unmount_dev = FALSE;
    struct pvr_register_info pvr_reg_info;
    struct statvfs stfs;
    INT32 pvr_ret = -1;
    char err_msg[64] = {0};
    int retval = 0;
    char fs_name[16] = {0};
    char *r_path __MAYBE_UNUSED__= NULL;

    b_update_part_info = FALSE;
    #if 0
	MEMSET(fs_name, 0, sizeof(fs_name));
    win_disk_mode_get_str(operate_result, fs_name, sizeof(fs_name));
    //Following windows os, if partition is over 32G(32*1000*1000*1000), not support FAT
    if((0 == strncasecmp(fs_name, "fat", 3)) && (focus_dev->total_size > (0x773594000LL)))
    {
		win_compopup_init(WIN_POPUP_TYPE_SMSG); 					   
		win_compopup_set_msg_ext("Partition is too large for FAT, please select another!",NULL,0);
		win_compopup_open_ext(&back_saved);
		osal_task_sleep(2000);
		win_compopup_smsg_restoreback();
		return FALSE;
    }
    #endif
    win_compopup_init(WIN_POPUP_TYPE_SMSG);
	//if partition size is over 2T (2*1000*1000*1000*1000), format may take much time
	if(focus_dev->total_size < (0x1d1a94a2000LL))
	{
    	win_compopup_set_msg_ext("formating wait!",NULL,0);
	}
	else
	{
		win_compopup_set_msg_ext("formating wait, it may take 2~3 minutes!",NULL,0);
	}
    win_compopup_open_ext(&back_saved);
    //pvr_format_HDD((choice == WIN_POP_CHOICE_YES) ? 1 : 2); //YES: FAT   NO: NTFS
    // step1: check pvr clear
#ifdef NEW_DEMO_FRAME
    disk_pvr_mode = pvr_get_disk_mode(focus_dev->vol_name);
    if(disk_pvr_mode >= PVR_REC_AND_TMS_DISK)
    //((!STRCMP(rec_disk, focus_dev->vol_name)) || (!STRCMP(tms_disk, focus_dev->vol_name)))
#endif
    {
        *used_for_dvr = TRUE;
    }

#ifdef DVR_PVR_SUPPORT
    if(*used_for_dvr)
    {
#if 1
        pvr_detach_part(focus_dev->vol_name, disk_pvr_mode);
#else
        ap_pvr_detach(focus_dev->vol_name);
#endif
    }
#endif

#ifdef WIFISTORAGE_SUPPORT
            r_path = wifi_storage_service_get_rootpath();
            if (!strncmp(focus_dev->vol_name, r_path, 9))
            {
                wifi_storage_service_stop(0, 0);
                osal_task_sleep(100);
            }
#endif

    // step2: unmount
    if(0 == STRLEN(focus_dev->vol_name))
    {
        unmount_dev = TRUE;
    }
    if(!unmount_dev)
    {
        //fs_statvfs(focus_dev->vol_name, &stfs);
        if(fs_statvfs(focus_dev->vol_name, &stfs) < 0)
        {
            return ret;
        }
        retval = fs_unmount(focus_dev->vol_name, 0);
        if (retval < 0)
        {
            if (-EBUSY == retval)
            {
                strncpy(err_msg, "Device busy, please format later!", 63);
            }
            else
            {
                strncpy(err_msg, "format fail!", 63);
            }
            win_compopup_smsg_restoreback();
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
        //win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
            win_compopup_set_msg_ext(err_msg,NULL,0);
            win_compopup_open_ext(&back_saved);
            osal_task_sleep(2000);
            win_compopup_smsg_restoreback();
            return FALSE;//break;
        }
    }

    b_update_part_info = TRUE;

    // step3: format
	MEMSET(fs_name, 0, sizeof(fs_name));
    win_disk_mode_get_str(operate_result, fs_name, 16);
    if(fs_mkfs(focus_dev->dev_name, fs_name) < 0)
    {
        win_compopup_smsg_restoreback();
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        //win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
        win_compopup_set_msg_ext("format fail!",NULL,0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(2000);
        win_compopup_smsg_restoreback();
        if (!unmount_dev) // partition is mounted before, try to mount it again
        {
            if (0 == STRCMP(focus_dev->fs_name, "NTFS"))
            {
                strncpy(fs_name, "NTFS", 15);
            }
            else
            {
                strncpy(fs_name, "FAT", 15);
            }
        }
        else
        {
            return FALSE;//break; // partition is not mounted, just break
        }
    }

    // step4: mount
    strncpy(focus_dev->vol_name, focus_dev->dev_name, 15);
    focus_dev->vol_name[1] = 'm';
    focus_dev->vol_name[2] = 'n';
    focus_dev->vol_name[3] = 't';
    if(unmount_dev)
    {
        fs_mkdir(focus_dev->vol_name, 0);
    }
    if(fs_mount(focus_dev->vol_name, focus_dev->dev_name, fs_name, 0, NULL) < 0)
    {
        fs_rmdir(focus_dev->vol_name);
        win_compopup_smsg_restoreback();
        win_compopup_init(WIN_POPUP_TYPE_SMSG);
        win_compopup_set_msg_ext("format fail!",NULL,0);
        win_compopup_open_ext(&back_saved);
        osal_task_sleep(2000);
        win_compopup_smsg_restoreback();
        return FALSE;//break;
    }
    if(!unmount_dev)
    {
        fs_write_statvfs(focus_dev->vol_name, &stfs, WFSSTAT_NAME);
        fs_sync(focus_dev->vol_name);
    }
#ifdef DVR_PVR_SUPPORT
    need_init_mp = FALSE;
    // step5: check pvr init
    if(*used_for_dvr)
    {
#ifndef PVR3_SUPPORT
        strncpy(string, focus_dev->vol_name, (60*2-1));
        storage_add_pvr_root(string, 60);
        pvr_set_root_dir_name(string);
        pvr_register(__MM_PVR_VOB_BUFFER_ADDR,__MM_PVR_VOB_BUFFER_LEN);
#else
        MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
        strncpy(pvr_reg_info.mount_name, focus_dev->vol_name, 15);
        pvr_reg_info.disk_usage = disk_pvr_mode;
        pvr_reg_info.sync = 1;
        pvr_reg_info.init_list = pvr_reg_info.disk_usage != PVR_TMS_ONLY_DISK;
#ifndef PVR_SPEED_PRETEST_DISABLE
        pvr_reg_info.check_speed = 1;
#endif
        pvr_ret = pvr_register((UINT32)&pvr_reg_info, 1);
        if(pvr_ret != SUCCESS)
        {
            win_compopup_smsg_restoreback();
            win_compopup_init(WIN_POPUP_TYPE_SMSG);
//          win_compopup_set_frame(200, 160, 200, 100);
            win_compopup_set_msg_ext("Pvr Initial Fail!",NULL,0);
            win_compopup_open_ext(&back_saved);
        }
        else
        {
            *used_for_dvr = FALSE;
        }
#endif
    }
#endif

#ifdef WIFISTORAGE_SUPPORT
    wifi_storage_service_start(focus_dev->vol_name, 0);
#endif

    win_compopup_smsg_restoreback();
    if(unmount_dev)
    {
        win_disk_info_update(FALSE);
    }
    win_disk_info_update_focus_device();
    return TRUE;
}

static PRESULT win_disk_info_unkown_act_proc(VACTION act)
{
    PRESULT ret = PROC_LOOP;
    PRESULT format_ret = PROC_LOOP;
    UINT8 back_saved= 0;
    struct dvr_hdd_info hdd_info;
    struct dev_info *focus_dev = NULL;
    BOOL used_for_dvr = FALSE;
#ifndef PVR3_SUPPORT
    char string[1024] = {0};
#endif
#ifdef DISK_MANAGER_SUPPORT
    int disk_type = 0;
    int disk_idx = 0;
    int part_idx = 0;
#endif
    UINT32 choice= 0;
    UINT8 operate_result= 0;
    enum PVR_DISKMODE  __MAYBE_UNUSED__ cur_pvr_mode = PVR_DISK_INVALID;
    char rec_disk[16] = {0};
    char tms_disk[16] = {0};
    pvr_play_rec_t  *pvr_info = NULL;

    MEMSET(pvr_info, 0, sizeof(pvr_play_rec_t));
    pvr_info = api_get_pvr_info();
    b_update_part_info = FALSE;

    focus_dev = get_focus_device();
    if((NULL == focus_dev) || (0 == g_dev_info.dev[g_dev_info.focus].num)) // no partition
    {
        return ret;
    }
    cur_pvr_mode = pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));
    switch(act)
    {
        case VACT_FORMAT:
            if((!STRCMP(rec_disk, focus_dev->vol_name)) && (ap_pvr_set_state() != PVR_STATE_IDEL))
            {
                break;
            }
            choice = win_disk_operate_open(OPERATE_TYPE_PART_FORMAT, &operate_result);
            //win_disk_info_update();
            osd_track_object((POBJECT_HEAD)&g_win_disk_info, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            update_disk_detail_info(focus_dev);
            update_pvr_device_bmp();
            if(choice)
            {
                format_ret = win_disk_info_format(focus_dev,operate_result,&used_for_dvr);
                if(PROC_LOOP == format_ret)
                {
                    return format_ret;
                }
            }
            break;
        case VACT_PVR_SELECT:
#ifdef DVR_PVR_SUPPORT
    #ifdef NEW_DEMO_FRAME
            if((0 == focus_dev->vol_name[0])
            || (pvr_info->rec_num > 0))// || (!pvr_info.hdd_valid))
            {
                break;
            }
            choice = win_disk_operate_open(OPERATE_TYPE_DVR_SET, &operate_result);
            //win_disk_info_update();
            osd_track_object((POBJECT_HEAD)&g_win_disk_info, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            update_disk_detail_info(focus_dev);
            update_pvr_device_bmp();
            if(choice)
            {
#ifdef RAM_TMS_TEST
                if (sys_data_get_ram_tms_en() &&
                    (win_dvr_type_get_mode(operate_result) != PVR_REC_ONLY_DISK))
                {
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
                    win_compopup_set_msg_ext("RAM disk timeshift enable, cannot change timeshift partition!",NULL,0);
                    win_compopup_open_ext(&back_saved);
                    osal_task_sleep(1000);
                    win_compopup_smsg_restoreback();
                    break;
                }
#endif
                win_compopup_init(WIN_POPUP_TYPE_SMSG);
//              win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                win_compopup_set_msg_ext("please wait!",NULL,0);
                win_compopup_open_ext(&back_saved);
                need_init_mp = FALSE;
                if(!api_pvr_partition_change(focus_dev->vol_name, win_dvr_type_get_mode(operate_result)))
                {
                    win_compopup_smsg_restoreback();
                    win_compopup_init(WIN_POPUP_TYPE_SMSG);
//                  win_compopup_set_frame(MSG_POPUP_LAYER_L, MSG_POPUP_LAYER_T, MSG_POPUP_LAYER_W, MSG_POPUP_LAYER_H);
                    win_compopup_set_msg_ext("Pvr Initial Fail!",NULL,0);
                    win_compopup_open_ext(&back_saved);
                }
		#ifdef PVR2IP_SERVER_SUPPORT
                char tmp_rec_disk[32] = {0};
                char *pvr2ip_msg = "PVR directory change.";
                if (media_provider_pvr_is_enable())
				{
                    snprintf(tmp_rec_disk, 32, "%s/%s", rec_disk, PVR_ROOT_DIR);
                    media_provider_pvr_del_dir(tmp_rec_disk);
                    memset(tmp_rec_disk, 0, 32);
                    snprintf(tmp_rec_disk, 32, "%s/%s", focus_dev->vol_name, PVR_ROOT_DIR);
                    media_provider_pvr_add_dir(tmp_rec_disk);
                    sat2ip_comm_sendmsg_all(SAT2IP_MSG_PVR_DIR_UPDATE, (UINT8 *)pvr2ip_msg, (UINT16)strlen(pvr2ip_msg));
                }
		#endif
                win_compopup_smsg_restoreback();
                win_disk_info_update(FALSE);
                update_disk_detail_info(focus_dev);
                update_pvr_device_bmp();
            }
    #endif
#endif
            break;
        default:
            break;
    }
#ifdef DISK_MANAGER_SUPPORT
    if (b_update_part_info)
    {
        disk_type = disk_name_prefix2type(&focus_dev->dev_name[5]);
        disk_idx = focus_dev->dev_name[7] - 'a';
        part_idx = focus_dev->dev_name[8] - '1';

        dm_ioctl(DM_CMD3_UPDATE_VOL_INFO, DM_CMD_PARAM(disk_type, disk_idx, part_idx), NULL, 0);
    }
#endif
    if (used_for_dvr)
    {
        pvr_get_hdd_info(&hdd_info);
        api_pvr_check_level(&hdd_info);
    }
    return ret;
}

static struct dev_info *get_focus_device(void)
{
    PCONTAINER pcon = NULL;
    struct dev_info *ret = NULL;
    OBJLIST *ol = &disk_info_ol;

    pcon = (PCONTAINER)osd_get_focus_object((POBJECT_HEAD)&g_win_disk_info);
    if(pcon)
    {
        ret = &g_dev_info.dev[g_dev_info.focus].part[g_dev_info.dev[g_dev_info.focus].focus];
    }
    else
    {
        g_dev_info.focus = g_dev_info.top;
        g_win_disk_info.focus_object_id = (g_dev_info.focus - g_dev_info.top + 1);
        pcon = (PCONTAINER)osd_get_focus_object((POBJECT_HEAD)&g_win_disk_info);
        if(pcon)
        {
            ret = &g_dev_info.dev[g_dev_info.focus].part[g_dev_info.dev[g_dev_info.focus].focus];
        }
    }
    //reflash highlight to avoid highlight device not match detail info
    osd_set_obj_list_cur_point(ol, g_dev_info.focus);
    osd_set_obj_list_new_point(ol, g_dev_info.focus);
    osd_set_obj_list_top(ol, g_dev_info.top);
    osd_set_obj_list_single_select(ol, INVALID_POS_NUM);
    return ret;
}

static void update_pvr_device_bmp(void)
{
    INT16 rec_i = -1;
    INT16 rec_j = -1;
    INT16 tms_i = -1;
    INT16 tms_j = -1;
    INT16 uleft = 0;
    INT16 utop = 0;
    INT16 uwidth = 0;
    INT16 uheight = 0;
    UINT16 i= 0;
    UINT16 j= 0;
    char rec_disk[16] = {0};
    char tms_disk[16] = {0};
    BITMAP *bmp = &pvr_bmp;

    pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));

    for(i=0; i<DEVICE_PAGE_NUM; i++)
    {
        if ((i + g_dev_info.top) < MAX_DEVICE_NS)
        {
            for(j=1; j<=g_dev_info.dev[i+g_dev_info.top].num; j++)
            {
                if((strlen(rec_disk) > 0) && (!STRCMP(rec_disk, g_dev_info.dev[i+g_dev_info.top].part[j].vol_name)))
                {
                    rec_i = i;
                    rec_j = j;
                }
                if((strlen(tms_disk) > 0) && (!STRCMP(tms_disk, g_dev_info.dev[i+g_dev_info.top].part[j].vol_name)))
                {
                    tms_i = i;
                    tms_j = j;
                }
            }
        }
    }
    if((rec_i != -1) && (rec_j != -1))
    {
        uleft = partitions_item_con[rec_i][rec_j].head.frame.u_left \
    + (partitions_item_con[rec_i][rec_j].head.frame.u_width - 4)/2 - (((rec_i == tms_i) && (rec_j == tms_j)) ? 8 : 0);
        utop = partitions_item_con[rec_i][rec_j].head.frame.u_top \
            + partitions_item_con[rec_i][rec_j].head.frame.u_height - 9;
        uwidth = 14;
        uheight = partitions_item_con[rec_i][rec_j].head.frame.u_height;
        osd_set_rect(&bmp->head.frame, uleft, utop, uwidth, uheight);
        osd_set_bitmap_content(bmp, IM_PVR_ICON_RECORD);
        osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
    }
    if((tms_i != -1) && (tms_j != -1))
    {
        uleft = partitions_item_con[tms_i][tms_j].head.frame.u_left + \
    (partitions_item_con[tms_i][tms_j].head.frame.u_width - 4) / 2 + (((rec_i == tms_i) && (rec_j == tms_j)) ? 8 : 0);
        utop = partitions_item_con[tms_i][tms_j].head.frame.u_top \
            + partitions_item_con[tms_i][tms_j].head.frame.u_height - 9;
        uwidth = 14;
        uheight = partitions_item_con[tms_i][tms_j].head.frame.u_height;
        osd_set_rect(&bmp->head.frame, uleft, utop, uwidth, uheight);
        osd_set_bitmap_content(bmp, IM_PVR_ICON_TIMESHIFT);
        osd_draw_object((POBJECT_HEAD)bmp, C_UPDATE_ALL);
    }
}

static void win_disk_info_update_focus_device(void)
{
    UINT32 i= 0;
    UINT32 j= 0;
    POBJECT_HEAD ptmpobj = NULL;
    struct dev_info *part = NULL;
    UINT8 dev_focus = 0;
    UINT8 par_focus = 0;

    ptmpobj = osd_get_focus_object((POBJECT_HEAD)&g_win_disk_info);
    if((ptmpobj) && (OT_CONTAINER == ptmpobj->b_type))
    {
//      UINT8 num = g_dev_info.num;
        dev_focus = g_dev_info.focus;
        par_focus = g_dev_info.dev[g_dev_info.focus].focus;

        i = g_dev_info.focus;
        j = g_dev_info.dev[g_dev_info.focus].focus;
        part = &g_dev_info.dev[i].part[j];
        update_dev_info();
        partitions_item_txt[i][j].head.frame.u_width = (partitions_item_con[i][j].head.frame.u_width - 4) \
            * (part->total_size - part->free_size) / part->total_size;
        osd_track_object((POBJECT_HEAD)&partitions_item_con[i][j], C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
//      MEMSET(&g_dev_info, 0, sizeof(struct dev_manager));
        g_dev_info.focus = dev_focus;
        g_dev_info.dev[g_dev_info.focus].focus = par_focus;
        update_disk_detail_info(&g_dev_info.dev[g_dev_info.focus].part[g_dev_info.dev[g_dev_info.focus].focus]);
        update_pvr_device_bmp();
    }
}

UINT8 init_valid_partition(char disk_partition[][16], UINT8 max)
{
    UINT16 i= 0;
    UINT16 j= 0;
    UINT8 ret = 0;
    char rec_disk[16] = {0};
    char tms_disk[16] = {0};

    pvr_get_cur_mode(rec_disk, sizeof(rec_disk), tms_disk, sizeof(tms_disk));
    for(i=0; i<g_dev_info.num; i++)
    {
        if(FALSE == g_dev_info.dev[i].valid)
    {
            continue;
        }

        for(j=1; j<=g_dev_info.dev[i].num; j++)
        {
            if((g_dev_info.dev[i].part[j].vol_name[0] != 0) && (STRCMP(rec_disk, g_dev_info.dev[i].part[j].vol_name))
            && (g_dev_info.dev[i].part[j].vol_name[7] != rec_disk[7]))
            {
                strncpy(disk_partition[ret++], &g_dev_info.dev[i].part[j].vol_name[5], 15);
            }
        }
    }
    return ret;
}

static PRESULT usb_state_con_callback(POBJECT_HEAD pobj, VEVENT event, UINT32 param1, UINT32 param2)
{
    PRESULT ret = PROC_PASS;
    UINT8   unact= 0;
    struct dev_info *focus_dev = NULL;
    OBJLIST *ol = NULL;
    UINT16 cnt= 0;
    UINT16 page= 0;
    UINT16 cur= 0;

    ol = &disk_info_ol;
    switch(event)
    {
    case EVN_PRE_OPEN:
        wincom_open_title((POBJECT_HEAD)&g_win_disk_info, RS_HDD_NFORMATION, 0);
        page = osd_get_obj_list_page(ol);
        cnt = g_dev_info.num;
        cur = osd_get_obj_list_cur_point(ol);
        //g_dev_info.top = cur / page*page;
        win_disk_info_update(FALSE);
        osd_set_obj_list_count(ol, g_dev_info.num);
        osd_set_attr(&disk_info_scb, (cnt <= page) ? C_ATTR_HIDDEN : C_ATTR_ACTIVE);
        if(cur >= cnt)
        {
            cur = 0;
        }
        osd_set_obj_list_cur_point(ol, cur);
        osd_set_obj_list_new_point(ol, cur);
        osd_set_obj_list_top(ol, g_dev_info.top);
        osd_set_obj_list_single_select(ol, INVALID_POS_NUM);
        break;
    case EVN_POST_OPEN:
        page = osd_get_obj_list_page(ol);
        cnt = g_dev_info.num;
        if(cnt > page)
    {
            osd_track_object((POBJECT_HEAD)ol,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
        }
        wincom_open_help(pobj, disk_helpinfo , HELP_CNT);
        //draw the first focus information
        focus_dev = get_focus_device();
        if(focus_dev != NULL)
        {
            update_disk_detail_info(focus_dev);
        }
        update_pvr_device_bmp();
        break;
    case EVN_PRE_CLOSE:
        *((UINT32 *)param2) &= ~C_CLOSE_CLRBACK_FLG;
        break;
    case EVN_POST_CLOSE:
        wincom_close_help();
        sys_data_check_channel_groups();
        break;
    case EVN_KEY_GOT:
        unact = (VACTION)(param1>>16);
        if((VACT_CURSOR_UP == unact) || (VACT_CURSOR_DOWN == unact))
        {
            if (0 == g_dev_info.num)
            {
                break;
            }
            page = osd_get_obj_list_page(ol);
            cnt = g_dev_info.num;

            if((0 == cnt) ||(0 == page))
            {
                break;//when no device attached,it will cause dived zero assert
            }

            cur = osd_get_obj_list_cur_point(ol);
            cur = ((VACT_CURSOR_DOWN == unact) ? (cur+1)%cnt : (cur+cnt-1)%cnt);
            if(VACT_CURSOR_UP == unact)
            {
                if(0 == g_dev_info.focus)
                {
                    break;//g_dev_info.focus = g_dev_info.num - 1;
                }
                else
                {
                    g_dev_info.focus--;
                }
            }
            else if(VACT_CURSOR_DOWN == unact)
            {
                g_dev_info.focus++;
                if(g_dev_info.focus == g_dev_info.num)
                {
                    //g_dev_info.focus = 0;
                    g_dev_info.focus--;
                    break;
                }
            }
            if((g_dev_info.focus < g_dev_info.top) || (g_dev_info.focus > (g_dev_info.top + g_dev_info.page - 1)))
            {
                if(cur >= cnt)
        {
                    cur = 0;
        }
                g_dev_info.top = cur/page*page;
                //if((cnt > page) && ((cnt - g_dev_info.top - 1) < page))
                //  g_dev_info.top = cnt - page;
                osd_set_obj_list_cur_point(ol, cur);
                osd_set_obj_list_new_point(ol, cur);
                osd_set_obj_list_top(ol, g_dev_info.top);
                osd_set_obj_list_count(ol, g_dev_info.num);
                osd_set_obj_list_single_select(ol, INVALID_POS_NUM);
            }
            else
            {
                osd_set_obj_list_count(ol, cnt);
                osd_set_attr(&disk_info_scb, (cnt <= page) ? C_ATTR_HIDDEN : C_ATTR_ACTIVE);
                if(cur >= cnt)
        {
                    cur = 0;
        }
                osd_set_obj_list_cur_point(ol, cur);
                osd_set_obj_list_new_point(ol, cur);
                osd_set_obj_list_top(ol, g_dev_info.top);
                osd_set_obj_list_single_select(ol, INVALID_POS_NUM);
                //if(cnt > page)
                //  OSD_TrackObject((POBJECT_HEAD)ol,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            }
            win_disk_info_update(FALSE);
            //OSD_TrackObject((POBJECT_HEAD)ol,C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
            update_pvr_device_bmp();
        }
        break;
    case EVN_UNKNOWN_ACTION:
        unact = (VACTION)(param1>>16);
        ret = win_disk_info_unkown_act_proc(unact);
        break;
    case EVN_MSG_GOT:
        if(CTRL_MSG_SUBTYPE_CMD_EXIT == param1)
        {
            ret = PROC_LEAVE;
        }
        else if ((CTRL_MSG_SUBTYPE_CMD_STO == param1) && (USB_STATUS_OVER == param2))
        {
            win_disk_info_update(FALSE);
        }
        break;
    default:
        break;
    }
    return ret;
}
////////////////////////////////////////////////
#endif//#ifdef DVR_PVR_SUPPORT
