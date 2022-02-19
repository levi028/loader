/*****************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: lib_pvr_mgn.h
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2009/3/4      Roman         3.0.0            Create
*    2009/3/6      Roman/Dukula/Larry 3.0.0        Correct api
*    2010/6/2      Jason                3.0.1                          Add mgrio api
*****************************************************************************/

#ifndef    __LIB_PVR_MGR_H__
#define    __LIB_PVR_MGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>
#include <sys_config.h>
#include <basic_types.h>
#include <osal/osal.h>
#include <api/libci/ci_plus.h>

//##: notice message from DVR module to APP/UI
#define PVR_HDD_READY            254
#define PVR_HDD_TEST_SPEED        253
#define PVR_HDD_PVR_CHECK        252
#define PVR_HDD_FAIL            251
#define PVR_HDD_FORMATTED        250
#define PVR_RECORD_PACK_OLDVERSION_START 249
#define PVR_RECORD_PACK_OLDVERSION_END 248
#define PVR_PLAYBACK_URI_NOTIFY        247
#define PVR_NO_CARD_FORCE_STOP            246
#define PVR_RECORD_PAUSE                245
#define PVR_RECORD_RESUME            244


//the time-shifting index, reverse the 1 for time shifting
#define TMS_INDEX                1    //? need keep this index??

//Roman: define for pvr manager(user data area), customer can change it by themself
#define MAX_BOOKMARK_NUM        20
#define MAX_AGELIMIT_NUM        512
#define MAX_RATINGCTL_NUM        256
#define C_PVR_ROOT_DIR_NAME_SIZE 32

#define PVR_REC_AND_TMS_DISK     0
#define PVR_REC_ONLY_DISK         1
#define PVR_TMS_ONLY_DISK         2
#define PVR_PLAY_ONLY_DISK        3

// DTG_PVR event name and detail
#define EVENT_NAME_MAX_LEN 70
#define EVENT_DETAIL_MAX_LEN 1024

//CI+ PVR key' and URI
#define PVR_CIPLUS_URI_LEN      32
#define PVR_MAX_URI_NUM            50

#define PVR_MOUNT_NAME_MAX_LEN      256

enum PVR_DETAIL_TYPE
{
    SHORT_DETAIL = 0,
    EXTEND_DETAIL,
};

typedef struct
{
    char *buf;
    UINT32 len;
} safe_buf;
typedef enum
{
    // PVR mgr IO
    // parameter                    // param1,                 param2
    PVR_MGRIO_RECORD_GETCNT = PVR_EIO_TOTAL,    // &cnt,                 0
    PVR_MGRIO_RECORD_GETINFO,        // &info,                mode|index(pos)
    PVR_MGRIO_RECORD_SETINFO,        // &info,                mode|index(pos)
    PVR_MGRIO_RECORD_GETIDX,        // &index,                 0
    PVR_MGRIO_RECORD_CHK_DEL,        // &index,                 0
    PVR_MGRIO_RECORD_CLR_DEL,        // &index,                 0
    PVR_MGRIO_RECORD_SAVE,            // &index,                 0
    PVR_MGRIO_RECORD_SAVE_ALL,        // &index,                 0
    PVR_MGRIO_PARTITION_GETINFO,
    PVR_MGRIO_PARTITION_TEST,
    PVR_MGRIO_PARTITION_GETREC,
    PVR_MGRIO_PARTITION_GETTMS,
    PVR_MGRIO_PARTITION_GETMODE,
    PVR_MGRIO_PARTITION_INITSIZE,
    PVR_MGRIO_SAVE_CUR_POS,
    PVR_MGRIO_GET_LAST_POS,
    PVR_MGRIO_GET_LAST_POSPTM,
    PVR_MGRIO_BOOKMARK_SET,
    PVR_MGRIO_BOOKMARK_GET,
    PVR_MGRIO_REPEATMARK_SET,
    PVR_MGRIO_REPEATMARK_GET,
    PVR_MGRIO_REPEATMARK_CLR,
    PVR_MGRIO_AGELIMIT_SET,
    PVR_MGRIO_AGELIMIT_GET,
    PVR_MGRIO_RATINGCTL_SET,
    PVR_MGRIO_RATINGCTL_GET,
    PVR_MGRIO_PARTITION_SETUSEAGE,
    PVR_MGRIO_CHECK_TMS2REC,
    PVR_MGRIO_CHECK_RECORD,
    PVR_MGRIO_CHECK_PLAYBACK,
    PVR_MGRIO_CACW_SET,
    PVR_MGRIO_CACW_GET,
    PVR_MGRIO_CACW_DUMP,
    PVR_MGRIO_EVENT_NAME_SET,
    PVR_MGRIO_EVENT_NAME_GET,
    PVR_MGRIO_EVENT_DETAIL_SET,
    PVR_MGRIO_EVENT_DETAIL_GET,
    PVR_MGRIO_RECORD_GET_PATH,
    PVR_MGRIO_GETIDX_BY_PATH,
    PVR_MGRIO_SECRTKEY_SET,
    PVR_MGRIO_SECRTKEY_GET,
    PVR_MGRIO_AUDIO_TYPE_SET,
    PVR_MGRIO_AUDIO_TYPE_GET,
    PVR_MGRIO_CHK_TMS_CAPABILITY,
    PVR_MGRIO_CIPLUS_URI_SET,
    PVR_MGRIO_CIPLUS_STOID_GET,
    PVR_MGRIO_IDENTIFY_RECORD, //use path to check whether a valid record
    PVR_MGRIO_SAVE_INFO_HEADER,
    PVR_MGRIO_GET_INFO_HEADER,
    PVR_MGRIO_SAVE_STORE_INFO,
    PVR_MGRIO_GET_STORE_INFO,
#if 1//chunpin_conax6_eng
    PVR_MGRIO_RECORD_SETURI,
    PVR_MGRIO_RECORD_GETURI,
    PVR_MGRIO_GETURI_CNT,
    PVR_MGRIO_GETURI_SETS,
#endif
} PVR_MGR_IO, *PPVR_MGR_IO;

enum PVR_MODE
{
    PVR_REC_SAME_FOLDER = 0,
    PVR_REC_SUB_FOLDER,
    PVR_REC_RANDOM_FOLDER,
};

enum PVR_DISKMODE
{
    PVR_DISK_INVALID = 0,
    PVR_DISK_REC_AND_TMS,
    PVR_DISK_REC_WITH_TMS,
    PVR_DISK_ONLY_REC,
    PVR_DISK_ONLY_TMS,
};

enum sort_type
{
    PVR_SORT_RAND = 0, //read order
    PVR_SORT_NAME, //alphabet
    PVR_SORT_TYPE, //a/v
    PVR_SORT_DATE, //record start time
    PVR_SORT_LEN, //record time len
    PVR_SORT_SIZE, //record size
};

enum dvr_hdd_info_status
{
    HDDS_SUCCESS         = 0,        // hdd can read,write.
    HDDS_NOT_EXIST         = (1 << 5),  // device not exist .
    HDDS_SPACE_FULL        = (1 << 6),  // hdd space is full, only enable read
    HDDS_CREATE_DIR_ERR    = (1 << 7),  // create root directory error, disable pvr read write
    HDDS_WRITE_ERR        = (1 << 8),  // hdd write error, maybe device is locked.
    HDDS_READ_ERR        = (1 << 9),    // hdd read error, reserved
};

enum PVR_MGR_COPY_TYPE
{
    PVR_COPY_NORMAL = 0,
    PVR_COPY_NO_MORE,            // copy no more
    PVR_COPY_ONE_GENR,            // copy one generation
    PVR_COPY_NEVER,                // copy never
};

struct dvr_hdd_info
{
    //size is in K
    UINT8     valid;
    UINT8    disk_mode;         // 0 usb disk, 1 ide disk
    char    mount_name[PVR_MOUNT_NAME_MAX_LEN];     // such as "/c"
    UINT8    type;            //1:FAT  2:NTFS
    UINT8    disk_usage;        // REC or TMS or total, default is total
    UINT8    init_list;            // inited pvr list
    UINT8    check_speed;
    UINT8   tms_level;      //0%-100%
    UINT32    total_size;        //the total size of the HDD.
    UINT32    free_size;        //free size
    UINT32    rec_size;            //record size
    UINT32    tms_size;            //for time shifting size
    UINT32    status;            //indicate current device status, refer to:enum dvr_HDD_info_status
    //used for internal
    char    disk_name[256];    //such as "/dev/hda1"
    UINT32    tms_free;
    UINT32    read_speed;
    UINT32    write_speed;
    UINT32    rw_speed;
};

struct pvr_register_info
{
    UINT8    disk_mode; //?? // 0 usb disk, 1 ide disk
    UINT8     disk_usage;
    char    mount_name[PVR_MOUNT_NAME_MAX_LEN]; // such as "/c"
    UINT8    init_list; // need init pvr record list
    UINT8    check_speed; // need to check speed
    UINT8     sync;
};

struct pvr_clean_info
{
    UINT8    disk_mode; // 0 usb disk, 1 ide disk
    char    mount_name[PVR_MOUNT_NAME_MAX_LEN]; // such as "/c"
};

//ZZL TODO: channel/group/prog_number/quantum_num/level/ don't know
struct list_info
{
    UINT8    pvr_version;
    UINT8     del_flag;// delete flag: <b>0</b> - Not deleted, <b>1</b> - Deleted
    UINT8    lock_flag;    // locked flag: <b>0</b> - Not locked, <b>1</b> - Locked
    UINT8    rec_mode;// the type of the Recording List: <b>0</b> - Normal, <b>1</b> - Time-shift
    UINT8    channel_type;// the channel type: <b>0</b> - TV, <b>1</b> - Radio
    UINT8    h264_flag;   //type defined in vdec.h: enum video_decoder_type{}
    UINT8    audio;// bit0~3: audio type(0 --MPEG 1, 1 --MPEG 2); bit4~7: audio channel
    UINT16    index;// the index of the Recording List
    enum sort_type sort_type;
    struct  PVR_DTM tm;// The start recording time information for the Recording List
    UINT32 start_ptm;
    UINT32  duration;// The playback/recording duration for this Recording List.
    UINT32    channel;// The channel number for the Recording List
    UINT8   txti[TXTI_LEN];// The text information (name of the Recording List)
    UINT16     group;// The Group for the Recording List
    UINT16  pid_pcr;
    UINT16    pid_v;
    UINT16    pid_a;
    UINT16     pid_pmt;
    UINT16     pid_cat;
    UINT16     prog_number;
    UINT32    quantum_num;
    UINT32    ts_bitrate;//bps
    UINT8    ca_mode; // $** or ** prog
    UINT8    is_scrambled;
    UINT8    is_recording;
    UINT8    level;//record level for QoS
    UINT32    size; //**K disk size
    UINT16    subt_num;
    UINT16    ttx_num;
    UINT16    ttx_subt_num;
    struct t_subt_lang *subt_list;
    struct t_ttx_lang *ttx_list;
    struct t_ttx_lang *ttx_subt_list;
    UINT8    audio_count;                        // total audio pid count
    UINT8    cur_audio_pid_sel;                    // current audio select pid.
    UINT16 multi_audio_pid[MAX_PVR_AUDIO_PID];    // multi audio pid
    UINT16 multi_audio_lang[MAX_PVR_AUDIO_PID]; // multi audio language

    UINT8 retention_limit: 6;
    UINT8  rec_type: 2; //0:TS, 1:PS

    UINT8 resv: 4;
    UINT8 copy_control: 2;
    UINT8 is_reencrypt: 1;
    UINT8 is_append: 1;

    struct pvr_pid_info pid_info;
    UINT8 record_pid_num; // add for dynamic pid
    UINT16 record_pids[PVR_MAX_PID_NUM]; // add for dynamic pid
    UINT16    isdbtcc_num;
    struct t_isdbtcc_lang *isdbtcc_list;
    UINT32 reserved[64];
    UINT8  event_txti[EVENT_NAME_MAX_LEN];
    UINT8  rec_special_mode;
    UINT8 record_de_encryp_key_mode;
    UINT8 is_not_finished;
};

typedef struct
{
    UINT8 full_path;                // 1: folder_name is full path.
    struct t_subt_lang *subt_list;
    struct t_ttx_lang *ttx_list;
    struct t_ttx_lang *ttx_subt_list;
    struct t_isdbtcc_lang *cclist;    // add for isdbt_cc
} pvr_mgr_record_param, *ppvr_mgr_record_param;

typedef struct
{
} pvr_mgr_playback_param, *ppvr_mgr_playback_param;


typedef struct
{
    UINT8 pvr_mode;
    UINT8 pvr_name_in_unicode;
    UINT8 debug_enable    : 1;
    UINT8 update_tms_space_disable : 1;
    UINT8 enable_list_idxfile : 1;
    UINT8 reserved0        : 5;
    UINT8 reserved1;
} PVR_MGR_CFG, *PPVR_MGR_CFG;

typedef struct
{
    UINT32 ptm  : 20; //in seconds
    UINT32 age  : 5;
    UINT32 resv : 7;
} record_agelimit_item; // 4B

typedef struct
{
    UINT32 ptm;    //in seconds
    UINT32 rating;
} record_ratingctl_item;

typedef struct
{
    UINT32  ptm;
    UINT8  *e_cw;
    UINT8  *o_cw;
} record_ca_cw_item;

typedef struct
{
    UINT8 *p_event_name;
} record_event_item;

typedef struct
{
    UINT8 *p_detail;
    UINT32 len;
    UINT8 type;
} record_event_detail_item;

typedef struct
{
    UINT8 u_detail[PVR_CIPLUS_URI_LEN];
    UINT32 ptm;
    UINT8 copy_type;
    UINT8 retention_limit;
} record_ciplus_uri_item;

typedef struct
{
    UINT16 v_pid;
    UINT16 a_pid;
    UINT8 dmx_id;
    UINT8 ca_mode;
    UINT8 key[260];
    UINT16 key_len;
} pvr_ciplus_param;

#if 0
#define STOREINFO_DEBUG libc_printf
#else
#define STOREINFO_DEBUG(...)    do{} while(0)
#endif

#define INFO_HEADER_SIZE 248
#define STORE_INFO_DATA_SIZE_MAX 64*1024

struct store_info_header
{
    UINT32 block_size;
    UINT32 storeinfoheader_len;
    UINT8 storeinfoheader[INFO_HEADER_SIZE];
};

struct store_info_data_single
{
    UINT32 time; //TS mode: time, Block mode:block count
    UINT32 encrypt_mode;
    UINT16 storeinfodata_len;
    UINT8 storeinfodata[STORE_INFO_DATA_SIZE_MAX];
};
struct store_info_data_single_dat
{
    UINT32 time;
    UINT32 encrypt_mode;
    UINT16 storeinfodata_len;
    UINT8 storeinfodata[512];
};

struct store_info_data
{
    struct store_info_data_single    store_info_data_pre;
    struct store_info_data_single    store_info_data_nex;
};

struct store_info
{
    UINT32 index;
    UINT32 ptm;
    UINT32 infor_pointer;
    UINT32 encrypt_mode;
};

struct store_info_param
{
    struct store_info_data *storeinfodata;
    UINT32 ptm;
};


#define STORE_INFO_HEADER_SIZE sizeof(struct store_info_header)
#define PTR_INFO_SIZE 32
#define STORE_HEADER_SIZE (STORE_INFO_HEADER_SIZE+PTR_INFO_SIZE)
#define STORE_PTR_SIZE 16
#define STORE_PTR_TABLE_SIZE 640*1024
#define STORE_INFO_LEN_SIZE 2


//##:
RET_CODE    pvr_mgr_attach(PPVR_MGR_CFG cfg, PPVR_CFG ini_param);

PVR_HANDLE     pvr_mgr_r_open(ppvr_mgr_record_param mgr_param, struct record_prog_param *eng_param);
BOOL        pvr_mgr_r_close(PVR_HANDLE *handle, struct record_stop_param *stop_param);

PVR_HANDLE     pvr_mgr_p_open(ppvr_mgr_playback_param mgr_param, struct playback_param *eng_param);
BOOL        pvr_mgr_p_close(PVR_HANDLE *handle, struct playback_stop_param *stop_param);
RET_CODE     pvr_mgr_ioctl(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2);


//##: register one partition.
INT32         pvr_cleanup_all(void);
INT32         pvr_cleanup_device(char type);
INT32         pvr_cleanup_partition(struct pvr_clean_info *param);

RET_CODE     pvr_set_disk_use(UINT8 use, const char *mount_name);
enum PVR_DISKMODE    pvr_get_cur_mode(char *rec_partition, UINT32 rec_len, char *tms_partition, UINT32 tms_len);
int pvr_mgr_get_cur_list_path(char *path, UINT16 size);

RET_CODE pvr_eng_set_uri(PVR_HANDLE handle, conax6_uri_item *uri);
RET_CODE pvr_eng_get_uri(PVR_HANDLE handle, conax6_uri_chunk_mgr *uri_mgr, UINT32 uri_ptm);
RET_CODE pvr_eng_get_uri_cnt(PVR_HANDLE handle, UINT32 *wcnt, char *file_path, conax6_uri_chunk_mgr *uri_mgr);
RET_CODE pvr_eng_get_uri_sets(UINT32 base, UINT32 cnt, char *file_path, conax6_uri_item *uri_sets);
RET_CODE pvr_mgr_idxfile_update(PVR_HANDLE handle);

UINT32 pvr_mgr_share_malloc(UINT32 size_k, UINT32 *addr);
void pvr_mgr_share_free(UINT32 addr);
UINT8 pvr_mgr_get_share_count();


#ifdef __cplusplus
}
#endif
#endif //__LIB_PVR_MGR_H__
