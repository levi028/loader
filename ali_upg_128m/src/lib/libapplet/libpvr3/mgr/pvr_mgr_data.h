#ifndef __PVR_MGR_DATA_H__
#define __PVR_MGR_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include <hld/hld_dev.h>
#include <osal/osal_timer.h>
#include <api/libc/list.h>

#include <api/libsi/si_tdt.h>
#include <hld/deca/deca.h>

#include <api/libttx/lib_ttx.h>
#include <api/libsubt/lib_subt.h>
#include <api/libisdbtcc/lib_isdbtcc.h>
#include <api/libfs2/types.h>
#include <api/libfs2/errno.h>
#include <api/libfs2/dirent.h>
#include <api/libfs2/stat.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/fcntl.h>
#include <api/libfs2/unistd.h>
#include <api/libfs2/stdio.h>

//
#include <api/libpvr/lib_pvr_eng.h>
#include <api/libpvr/lib_pvr_mgr.h>
#include <api/libpvr/lib_pvr_crypto.h>
#include <api/libmp/lib_mp.h>
#include <api/libtsi/sec_pmt.h>

#define PVR_MGR_DEBUG(fmt, args...) if(g_pvr_mgr_info->config.cfg_ext.debug_enable){\
        libc_printf("PVR->MGR " fmt, ##args);\
    }
#define ENTER_FUNCTION PVR_MGR_DEBUG("Enter Function %s\n", __FUNCTION__)
#define LEAVE_FUNCTION PVR_MGR_DEBUG("Leave Function %s, from line: %d\n", __FUNCTION__, __LINE__);
#define PDEBUG(fmt, args...) PVR_MGR_DEBUG(fmt, ##args)

#define P_M_ASSERT(expression) \
    {                                   \
        if (!(expression)){             \
            PDEBUG("assertion(%s) failed: file \"%s\", line %d\n",  \
                   #expression, __FILE__, __LINE__);}  \
    }

#define pvr_return_if_fail(expr)        do{     \
        if (expr) { } else                      \
        {                           \
            PVR_MGR_DEBUG("file %s: line %d: assertion `%s' failed\n",  \
                          __FILE__,                   \
                          __LINE__,                   \
                          #expr);                     \
            return;                     \
        };      }while(0)

#define pvr_return_val_if_fail(expr, val)   do{     \
        if (expr) { } else                      \
        {                           \
            PVR_MGR_DEBUG("file %s: line %d: assertion `%s' failed\n",  \
                          __FILE__,                   \
                          __LINE__,                   \
                          #expr);                     \
            return (val);                       \
        };      }while(0);

#define pvr_fs_error(expr, val) do{     \
        if (expr < 0)                       \
        {                           \
            PVR_MGR_DEBUG("fs error!"); \
            return (val);                       \
        };      }while(0);

#define PERROR(fmt, args...) libc_printf("PVR MGR-> " fmt, ##args)

#define BOOKMARK_THRESHOLD_TIME 10*1000//10second

// 0~200M: do nothing; 200~500M: only tms; 500~:total pvr
#define MIN_PVR_FREESIZE (200*1024*1024) //don't do any dvr if disk free space < 200M
#define MIN_TMS_FREESIZE (500*1024*1024) //don't do rec if disk free space < 500M

#define MAX_FAT_FILE_SIZE   (4*1000*1000)   // real max fat file size is 4GB

#define PVR_MGR_CALLER_DEPTH 8
#define PVR_MGR_REC_PARTITION_NUM 3
//use record start time and one random num as record name
#define DVR_RECORD_NAME_FORMAT "/%4d-%02d-%02d.%02d.%02d.%02d-%s-%2d"
#define PVR_RW_TEST_TIMES       128

#define _pvr_mgr_malloc(ptr, size, reset) do\
    { \
        ptr = MALLOC(size);\
        P_M_ASSERT(ptr != NULL);\
        if(NULL != ptr)\
        { \
            if(reset) {\
                MEMSET(ptr, 0, size);\
            }\
        }\
    }while(0)

#define _pvr_mgr_free(ptr) do\
    { \
        if(ptr != NULL)\
        { \
            FREE(ptr);\
            ptr = NULL;\
        }\
    }while(0)

#if 0//chunpin_conax6_eng
#define CONAX6_CHUNK_DEBUG(fmt, arg...) libc_printf(fmt, ##arg)
#else
#define CONAX6_CHUNK_DEBUG(...) do{} while(0)
#endif

enum PVR_MGR_REC_TYPE
{
    PVR_MGR_REC_IDLE = 0,
    PVR_MGR_REC_RECORD,
    PVR_MGR_REC_PLAYBACK,
    PVR_MGR_REC_RECANDPLY,
};

enum PVR_MGR_CMD_TYPE
{
    PVR_MGR_REGISTER            = 0x0101,
    PVR_MGR_CLEANUP             = 0x0102,
    PVR_MGR_CLEANUP_DEVICE      = 0x0103,
    PVR_MGR_CLEANUP_PARTITION   = 0x0104,
    PVR_MGR_NOTICE              = 0x0105,
    PVR_MGR_COPY                = 0x0106,
};

typedef struct
{
    UINT32 id;
    char path[1024];
} PVR_DIR;

typedef struct
{
    WORD    yyyy;
    BYTE    mm;
    BYTE    dd;
} pvr_date;

typedef struct
{
    BYTE    hh;
    BYTE    mm;
    BYTE    ss;
} pvr_time;

typedef struct
{
    UINT8 is_dir;
    int size;   /* if size < 0, than it is a directory */
    char    name[550];
    pvr_date date;
    pvr_time time;
} pvr_dirent;

typedef struct
{
    UINT16      cmd;            //the message
    UINT16      param16;        //the 16 bit parameter general for the sub command!
    UINT32      param32;        //the 32 bit parameter
    UINT32      param32_ext;    //the 32 bit parameter for extern
    UINT32      *p_result;      //the result pointer
    PVR_HANDLE  handle;         //the player or recorder handle
    UINT32      flag;           //the waiting flag
    UINT32      sync;           //if is the sync mode
} pvr_mgr_cmd, *ppvr_mgr_cmd;



typedef struct
{
    UINT32 ptm;
    UINT16 file_idx;
    UINT16 resv;
    UINT32 vobu_idx;
} record_bookmark_item; //12B

typedef struct
{
    struct list_head    listpointer;
    record_bookmark_item item;
} bookmark_list_item, *pbookmark_list_item;

typedef struct
{
    UINT16 record_idx;
    char channel_name[TXTI_LEN];         //34Bytes
    UINT8 is_locked;
    enum sort_type  sort_type;
    UINT16  subt_num;
    UINT16  ttx_num;
    UINT16  ttx_subt_num;
    UINT16  agelimit_num;
    UINT8   ratingctl_num;  // for ATSC rating
    struct t_subt_lang  subt_array[SUBTITLE_LANG_NUM];//12 Bytes * 10
    struct t_ttx_lang   ttx_array[TTX_SUBT_LANG_NUM];//8 Bytes * 11
    struct t_ttx_lang   ttx_subt_array[TTX_SUBT_LANG_NUM];//8 Bytes * 11
    record_bookmark_item    bookmark_array[MAX_BOOKMARK_NUM];//12 B * 20
    UINT32  last_play_pos; //for last play remember, not so precise because of vob cache
    UINT32  last_play_ptm; //absolute ptm from record head (0), in second
    UINT8   need_pack; //inluding small files!
    UINT8   repeat_ab; // 578
    record_bookmark_item    ab_array[2];//12 B * 2
    record_agelimit_item    agelimit_array[MAX_AGELIMIT_NUM];//4B*512=2KB
    record_ratingctl_item   ratingctl_array[MAX_RATINGCTL_NUM]; //8byte*256 = 2kB for ATSC rating
    UINT32  reserved[64]; //add for customer usage
    UINT8   is_deleted;
    UINT8   bookmark_num;
    UINT16  isdbtcc_num;
    struct t_isdbtcc_lang   isdbtcc_array[ISDBTCC_LANG_NUM];//8 Bytes * 8
    UINT8   resv1[132];
    UINT8 event_name[EVENT_NAME_MAX_LEN];// DTG_PVR event name
    UINT8 event_short_detail[EVENT_DETAIL_MAX_LEN];//unicode event short detail
    UINT8 event_ext_detail[EVENT_DETAIL_MAX_LEN];//unicode event ext detail
    UINT8 multi_audio_type[MAX_PVR_AUDIO_PID]; // for audio description, 10 Bytes

    UINT32 sto_id[2];    // STO
    UINT8 ciplus_key[PVR_CIPLUS_KEY_LEN]; //36 B
    UINT8 key_len;
    UINT8 uri_num;
    UINT8 resv3[2];
    struct
    {
        UINT8 ciplus_key_uri[PVR_CIPLUS_URI_LEN];
        UINT32 uri_ptm;
    } uri_array[PVR_MAX_URI_NUM];   //URI'
    enum PVR_MGR_COPY_TYPE copy_type;    //EMI
    UINT8 retention_limit;    //RL

    UINT8 resv2[7283];          //   8*1024 + 111
    //UINT8 resv2[11*1024];//resv2[14*1024];//resv2[16*1024];
} pvr_mgr_user_data, *ppvr_mgr_user_data;

#define MAX_CA_CWS_NUM  204
#define CA_CW_LEN       8

typedef struct
{
    UINT32 ptm;
    UINT8  e_cw[CA_CW_LEN];
    UINT8  o_cw[CA_CW_LEN];
} record_ca_cw; //20B

typedef struct
{
    UINT32          ca_cws_number;
    record_ca_cw    ca_cws_array[MAX_CA_CWS_NUM];
} record_ca_cw_info; /* saved in ts file header's priv_data[] */


struct stack_node
{
    struct list_head    listpointer;
    struct list_head    *left;
    struct list_head    *right;
};      //used for quick sort

//compare function, used for list sort
typedef UINT8(*p_cmp_function)(enum sort_type mode, struct list_head *dest, struct list_head *src);

struct cmp_param
{
    enum sort_type mode;    //sort mode
    p_cmp_function cmpfun;
};


typedef struct
{
    OSAL_ID task_id;
    OSAL_ID msgbuf_id;
    OSAL_ID flag_id;
    UINT32 task_state;
    OSAL_ID caller_task_ids[PVR_MGR_CALLER_DEPTH];
} pvr_mgr_tsk_info, *ppvr_mgr_tsk_info;

typedef struct
{
    struct list_head listpointer;
    struct dvr_hdd_info partition_info;
} pvr_partition_info, *ppvr_partition_info;

typedef struct
{
    UINT8 partition_num;
    BOOL app_alloc_tms_size;
    ppvr_partition_info tms_partition;
    ppvr_partition_info rec_partition[PVR_MGR_REC_PARTITION_NUM];
    struct list_head partition_list;
} pvr_mgr_partition_info;

typedef struct
{
    struct list_head    listpointer;
    UINT8   finished;
    char    list_name[256];
    char    path_prex[256];
    enum sort_type  sorttype;
    UINT8   sortorder;//0:little order,!0:big order
    UINT16  subdir_num;
    UINT16  record_num; //total record number, not include tms item
    UINT8   is_group;
    struct list_head    head;
} pvr_mgr_list, *ppvr_mgr_list;

typedef struct
{
    // origin record idx since load from disk, not changed by any sort!
    // the max idx maybe not equal to current record num!!
    UINT32          record_idx;
    struct list_info record;
    UINT8           is_update;
    UINT8           dmx_id;
    ppvr_mgr_user_data puser_data;
} pvr_mgr_rec_info, *ppvr_mgr_rec_info;

typedef struct
{
    struct list_head    listpointer;
    char                record_dir_path[256]; //the recording dir path, like"/AliDvr/2007-07-10.15.23.34-11/"
    file_type       f_type;
    ppvr_mgr_rec_info   record_info;
    struct store_info  sinfo;
    FILE   *storeinfo_file;
#if 1 //specail modification for siti project 2016/06/16 doy.dong
    int  index;
#endif

} pvr_mgr_list_item, *ppvr_mgr_list_item;

typedef struct
{
    struct list_head    listpointer;
    pvr_mgr_list_item   *ptr_record;
} pvr_mgr_list_group_item, *ppvr_mgr_list_group_item;

typedef struct
{
    enum PVR_MGR_REC_TYPE type; // 1:recording, 2:playbacking, 3 both
    UINT16 index;
    PVR_HANDLE r_handle;
    PVR_HANDLE p_handle;
    INT8 p_param1, p_param2;//now used for ciplus URI flag
    struct list_head bookmark_list;
    struct list_head *listpointer;
} pvr_mgr_active_record, *ppvr_mgr_active_record;

typedef struct
{
    PVR_MGR_CFG cfg_ext;
    // record format config
    char dvr_path_prefix[256];  // like "AliDvr"
    char info_file_name[256];   // like "info.dvr"
    char info_file_name_new[256];   // like "info.dvr"
    char ts_file_format[256];   // like "dvr"
    char ts_file_format_new[256];   // like "ts"
    char ps_file_format[256];   // like "mpg"
    char test_file1[256];       // like "test_write1.dvr"
    char test_file2[256];       // like "test_write2.dvr"
    char storeinfo_file_name[256];  // like "info.dvr"
    char datinfo_file_name[256];  //"xxx.dat"
    UINT8 prj_mode;
    notice_callback event_callback;         // notice application callback
    get_local_time_callback local_time_callback; // local time callback
} pvr_mgr_config, *ppvr_mgr_config;

/********************************  manager global data   *********************************/

#define PLAYER_REENCRYPT_STATE_IDLE 0
#define PLAYER_REENCRYPT_STATE_BUSY 1
#define PLAYER_REENCRYPT_NEED_CLOSE 2

typedef struct
{
    UINT8 enable;
    INT8 register_state;
    UINT32 module_state;
    UINT8 player_reencrypt_state; //0 idle, 1 busy, 2 need close
    pvr_mgr_config config;
    pvr_mgr_tsk_info task_info;
    pvr_mgr_partition_info partition_info;
    struct list_head pvr_list;
    pvr_mgr_active_record active_record[PVR_MAX_REC_NUM + PVR_MAX_PLAY_NUM];
} pvr_mgr_info, *ppvr_mgr_info;

extern ppvr_mgr_info g_pvr_mgr_info; // pvr manager info

/********************************  manager internal apis   *********************************/
//----------------- extern function -----------------//

//----------------- pvr_mgr_common -----------------//
int _strncasecmp(const char *cs, const char *ct, int count);
int _strcasecmp(const char *cs, const char *ct);
BOOL _pvr_mgr_check_extname(const char *name, const char *ext_name);
void _pvr_mgr_get_mount_name(const char *full_path, char *mount_name, UINT32 mount_name_len);
BOOL _pvr_mgr_calc_fullpath(const char *path_prex, PREC_ENG_INFO info, PVR_FFORMAT_TYPE file_type,
                            UINT32 file_idx, char *full_path, UINT32 buff_size);
void _pvr_mgr_get_local_time(struct PVR_DTM *tm);
INT8 _pvr_mgr_event(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code);
INT32 _pvr_mgr_fpartition_info(struct dvr_hdd_info *partition_info);
RET_CODE _pvr_mgr_fpartition_test(struct dvr_hdd_info *partition_info);
RET_CODE _pvr_mgr_prepare_partition(const char *mount_name);
void _pvr_mgr_register_partition(struct pvr_register_info *partition);
ppvr_partition_info _pvr_mgr_get_partition(const char *mount_name);
RET_CODE _pvr_mgr_get_partition_info(char *mount_name, struct dvr_hdd_info *partition_info);
enum PVR_DISKMODE _pvr_mgr_get_cur_mode(char *rec_partition, UINT32 rec_len, char *tms_partition, UINT32 tms_len);
INT32 _pvr_mgr_cleanup_all(void);
INT32 _pvr_mgr_cleanup_device(char type);
INT32 _pvr_mgr_cleanup_partition(struct pvr_clean_info *pvr_cln_info);
BOOL _pvr_load_record_listinfo(ppvr_mgr_list_item record_item);
BOOL _pvr_mgr_add_record(ppvr_mgr_list_item record_item, UINT16 rec_mode);
BOOL _pvr_mgr_prepare_user_data(ppvr_mgr_list_item record_item, ppvr_mgr_record_param mgr_param,
                                struct record_prog_param *eng_param);
BOOL _pvr_mgr_update_userdata(PVR_HANDLE handle, UINT8 *user_data, UINT32 len);
UINT32 _pvr_mgr_update_record_size(PVR_HANDLE handle, INT32 size);
RET_CODE _pvr_mgr_set_partition_use(UINT8 use, char *mount_name);
ppvr_mgr_list _pvr_list_get_by_mountname(const char *mount_name);

ppvr_mgr_active_record _pvr_get_free_record(void);
ppvr_mgr_list   _pvr_list_get_cur_list(void);
RET_CODE    _pvr_list_chgdir(const char *path, UINT8 load_list);

RET_CODE _pvr_list_sort(ppvr_mgr_list list, enum sort_type mode, UINT8 order);
ppvr_mgr_list_item  _pvr_list_getitem_byorder(ppvr_mgr_list list, UINT16 order);
ppvr_mgr_list_item  _pvr_list_getitem_byidx(ppvr_mgr_list list, UINT16 idx);
ppvr_mgr_list_item  _pvr_list_getitem_bypath(ppvr_mgr_list list, const char *path);
ppvr_mgr_list_item  _pvr_list_getitem(ppvr_mgr_list list, UINT16 mode, UINT16 param);
ppvr_mgr_active_record _pvr_get_record_bypath(const char *path);
ppvr_mgr_active_record _pvr_get_record_byhandle(PVR_HANDLE handle);
ppvr_mgr_active_record _pvr_get_tms_record(void);
void _pvr_quick_sort(struct list_head *head, struct cmp_param para);
RET_CODE _pvr_list_remove_item(ppvr_mgr_list list, pvr_mgr_list_item *record_item);
RET_CODE _pvr_list_insert_item(ppvr_mgr_list list, pvr_mgr_list_item *record_item);
ppvr_mgr_list_item _pvr_list_handle_2_recorditem(UINT32 *handle);
UINT16  _pvr_list_get_record_num(void);
BOOL    _pvr_list_order_index(struct list_head *head);
UINT16 _pvr_list_add_item(ppvr_mgr_list list, const char *rec_path_prefix, PREC_ENG_INFO item_info);
RET_CODE _pvr_list_trans_eng_info(struct list_info *record, PREC_ENG_INFO info);
RET_CODE _pvr_list_trans_info_to_eng(PREC_ENG_INFO eng_info, struct list_info *list_info);
RET_CODE _pvr_list_trans_mgr_info(UINT8 update_pending, struct list_info *record, ppvr_mgr_user_data info);
UINT16 _pvr_list_get_record_info(ppvr_mgr_list list, UINT16 idx, UINT8 idx_mode, struct list_info *info);
RET_CODE    _pvr_list_delete_record(ppvr_mgr_list list, ppvr_mgr_list_item record_item);
RET_CODE    _pvr_list_del_by_idx(UINT16 index);
BOOL _pvr_list_check_del(ppvr_mgr_list list);
void _pvr_list_clear_del(ppvr_mgr_list list);
RET_CODE    _pvr_list_del_partition(const char *mount_name);
RET_CODE    _pvr_list_add_partition(const char *mount_name, UINT8 init_list);
ppvr_mgr_list _pvr_list_finddir(const char *path);
RET_CODE    _pvr_list_chgdir(const char *path, UINT8 load_list);
ppvr_mgr_list _pvr_list_get_by_mountname(const char *mount_name);

#if 1 //specail modification for siti project 2016/06/16 doy.dong
RET_CODE _pvr_list_save_idx_file(ppvr_mgr_list list);
RET_CODE _pvr_list_load_idx_file(const char *pvr_dir);
RET_CODE _pvr_list_idxfile_update(ppvr_mgr_list_item record_item);
#endif

BOOL _pvr_mgr_cmd_main(ppvr_mgr_cmd n_cmd);
RET_CODE pvr_mgr_get_cfg(PPVR_MGR_CFG *ppcfg);
BOOL pvr_mgr_detach(void);
RET_CODE pvr_mgr_p_play_mode(PVR_HANDLE handle, UINT8 new_state, UINT32 param);
INT32 pvr_register_partition(struct pvr_register_info *info);
RET_CODE pvr_mgr_copy(struct list_info *rl_info, struct copy_param cp_param);
RET_CODE pvr_mgr_set_extfile(char *path, BOOL full_path);
RET_CODE _pvr_mgr_add_file(char *dir_path);
RET_CODE pvr_mgr_add_extfile(char *dir_path);
UINT16 _pvr_list_alloc_index(ppvr_mgr_list list);
void _pvr_mgr_add_record_name(UINT8 *dest_name, UINT32 dest_len, UINT16 *src_name);
BOOL _pvr_mgr_get_parent_dir(const char *name, const char *parent_name);
INT8 _pvr_mgr_event_rec(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code);
INT8 _pvr_mgr_event_ply(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code);
void printf_hdd_info(struct dvr_hdd_info *partition_info);
RET_CODE _pvr_mgr_get_tms_hdd(struct dvr_hdd_info *partition_info);
BOOL _pvr_mgr_add_extfile(ppvr_mgr_list_item record_item);
BOOL _pvr_mgr_recycle_record(ppvr_mgr_list_item record_item);
RET_CODE _pvr_mgr_set_tms_size(UINT32 tms_size, UINT32 tms_file_min_size);
RET_CODE _pvr_mgr_set_tms_level(UINT8 percent, UINT32 tms_file_min_size);
UINT32 _pvr_mgr_chk_tms_capability(UINT32 ts_bitrate, PTMS_INFO tms_info);
UINT32 _pvr_data_trans_retention_time(UINT8 retention_limit);
BOOL _pvr_mgr_data_write_store_info_header(PVR_HANDLE handle, struct store_info_header *sheader);
BOOL _pvr_mgr_data_read_store_info_header(ppvr_mgr_list_item item, struct store_info_header *sheader);
BOOL _pvr_mgr_data_write_storeinfo(PVR_HANDLE handle, struct store_info_data_single *storeinfodata,
                                   UINT32 time);
BOOL _pvr_mgr_data_read_storeinfo(ppvr_mgr_list_item item, struct store_info_data *storeinfodata,
                                  UINT32 time);
RET_CODE    pvr_list_rename_record(UINT16 idx, const char *name);
RET_CODE _pvr_list_copy_record(UINT16 index, const char *dest_mount_name);
RET_CODE _pvr_list_copy_record_by_time(struct list_info *rl_info, char *path, const char *dest_mount_name,
                                       UINT32 time_start, UINT32 time_end);
BOOL _pvr_mgr_add_copy_record(struct list_info *rl_info, char *path);
RET_CODE _pvr_mgr_get_ca_cw(ppvr_mgr_list_item mgr_item, record_ca_cw_item *cw_item);
RET_CODE _pvr_mgr_set_ca_cw(ppvr_mgr_list_item mgr_item, record_ca_cw_item *cw_item);
RET_CODE _pvr_mgr_dump_ca_cw(ppvr_mgr_list_item mgr_item);
ppvr_mgr_active_record _pvr_update_record_handle(PVR_HANDLE handle);
ppvr_mgr_active_record _pvr_update_play_handle(PVR_HANDLE handle);
RET_CODE _pvr_list_update_mgr_info(ppvr_mgr_user_data info, struct list_info *record);
BOOL _pvr_list_save_record(ppvr_mgr_list list, ppvr_mgr_list_item record_item);
BOOL _pvr_list_save_list(ppvr_mgr_list list);
ppvr_mgr_list _pvr_list_get_mgrlist(const char *path);
void _pvr_mgr_declare_cmd_finish(UINT32 cmd_bit);
INT32 _pvr_mgr_send_message(PVR_HANDLE handle, UINT32 msg, UINT16 param16, UINT32 param32,
                            UINT32 param32_ext, BOOL sync);
void _pvr_mgr_frame(void);
INT32 _pvr_mgr_task_delete(void);
INT32 _pvr_mgr_task_init(void);
RET_CODE _pvr_mgr_bookmark_set(ppvr_mgr_list_item record_item, UINT32 mark_time);
UINT8 _pvr_mgr_bookmark_get(ppvr_mgr_list_item record_item, UINT32 *mark_ptm_array);
void _pvr_mgr_bookmark_dellist(ppvr_mgr_active_record record);
void _pvr_mgr_bookmark_load(ppvr_mgr_list_item record_item);
RET_CODE _pvr_mgr_repeatmark_set(ppvr_mgr_list_item record_item, UINT32 mark_time);
UINT8 _pvr_mgr_repeatmark_get(ppvr_mgr_list_item record_item, UINT32 *mark_ptm_array);
void _pvr_mgr_save_cur_pos(PVR_HANDLE handle);
UINT32 _pvr_mgr_get_last_pos(ppvr_mgr_list_item record_item);
UINT32 _pvr_mgr_get_last_posptm(ppvr_mgr_list_item record_item);
RET_CODE _pvr_mgr_agelimit_set(ppvr_mgr_list_item record_item, UINT32 ptm, UINT32 age);
UINT8 _pvr_mgr_agelimit_get(ppvr_mgr_list_item record_item, UINT32 ptm);
RET_CODE _pvr_mgr_ratingctl_set(ppvr_mgr_list_item record_item, UINT32 ptm, UINT32 ratingctl);
UINT32 _pvr_mgr_ratingctl_get(ppvr_mgr_list_item record_item, UINT32 ptm);
RET_CODE _pvr_data_set_event_detail(ppvr_mgr_list_item record_item, UINT8 *p_detail,
                                    UINT32 len, UINT8 type);
UINT8 *_pvr_data_get_event_detail(ppvr_mgr_list_item record_item, UINT8 type);
RET_CODE _pvr_data_set_event_name(ppvr_mgr_list_item record_item, UINT8 *event_name);
UINT8 *_pvr_data_get_event_name(ppvr_mgr_list_item record_item);
RET_CODE _pvr_mgr_audio_type_set(ppvr_mgr_list_item record_item, UINT8 *buf, UINT8 len);
RET_CODE _pvr_mgr_audio_type_get(ppvr_mgr_list_item record_item, UINT8 *buf, UINT8 len);
RET_CODE _pvr_mgr_set_ciplus_uri(ppvr_mgr_list_item record_item, record_ciplus_uri_item *item);
INT8 _pvr_mgr_get_ciplus_uri(ppvr_mgr_list_item record_item, UINT32 ptm, record_ciplus_uri_item *uri);
RET_CODE pvr_mgr_get_cfg(PPVR_MGR_CFG *ppcfg);
RET_CODE _pvr_mgr_set_tms_size(UINT32 tms_size, UINT32 tms_file_min_size);
RET_CODE _pvr_mgr_get_rl_idx_by_path(char *path, UINT16 *idx);


#ifdef __cplusplus
}
#endif

#endif// __PVR_MGR_DATA_H__

