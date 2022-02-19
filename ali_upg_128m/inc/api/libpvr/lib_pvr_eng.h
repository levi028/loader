/***********************************************************************************
*    Copyright (C) 2009 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*    File: lib_pvr_engine.h
*
*    Description:
*
*    History:
*    Date           Athor        Version        Reason
*    ========       ========     ========       ========
*    2009/3/4       Roman        3.0.0            Create
*    2010/6/2       Jason             3.0.1                   Add two message type
*    2010/6/7       Jason             3.0.2                   Add two message type
***********************************************************************************/
#ifndef    __LIB_PVR_ENG_H__
#define    __LIB_PVR_ENG_H__

#include <types.h>
#include <sys_config.h>
#include <basic_types.h>
#include <api/libfs2/lib_stdio.h>
#include <hld/dmx/dmx.h>

#define PVR_FS_API  //Use IO interfaces of VFS

extern UINT8 g_pvr_block_mode;
extern UINT32 g_pvr_block_size;
extern UINT32 g_pvr_file_idx;
extern UINT32 g_pvr_file_quantum_offset;
extern UINT32 eng_block_mode_dmx_inj_id;

#define PVR_VERSION_1               0
#define PVR_VERSION_2               1
#define PVR_VERSION_3               3
#define PVR_VERSION_4               4
#define PVR_DATA_VERSION_3        3

typedef UINT32    PVR_HANDLE;
#define REC_HND_MASK             0x52450000        //"RE--"
#define PLY_HND_MASK             0x504C0000        //"PL--"

#define PVR_MAX_REC_NUM            3
#define PVR_MAX_PLAY_NUM        1

#define PVR_HANDLE_2_INDEX(hnd)    ((UINT8)(((hnd)<<16)>>24))
#define PVR_IDX_2_REC_HND(idx)    ((PVR_HANDLE)(REC_HND_MASK+((idx)<<8)))
#define PVR_IDX_2_PLY_HND(idx)    ((PVR_HANDLE)(PLY_HND_MASK+((idx)<<8)))
#define PVR_HANDLE_IS_REC(hnd)    (REC_HND_MASK == ((hnd)>>16<<16))
#define PVR_HANDLE_IS_PLY(hnd)    (PLY_HND_MASK == ((hnd)>>16<<16))

#define DVR_DMX_FOR_TMSHIFT        0
#define DVR_DMX_FOR_RECORDING    1
#define DVR_DMX_FOR_PLAYBACK    2

#define TXTI_LEN                 34    // The text information len (name of the Recording List)
#define RECORDING_NORMAL         0    // recording as normal
#define RECORDING_TIMESHIFT        1    // recording as time-shifting
#define QUANTUM_SIZE             g_pvr_block_size//(47*1024)

#define PVR_MAX_PID_NUM            32
#define START_END_MAX_NUM         20

#define MAX_PVR_AUDIO_PID        10
#define MAX_PVR_ECM_PID            16
#define MAX_PVR_EMM_PID            16
#define MAX_PVR_SUBT_PID        11
#define MAX_PVR_TTX_PID            11
#define MAX_PVR_TTX_SUBT_PID    11
#define MAX_PVR_ISDBTCC_PID    8
#define MAX_PVR_ADDITION_PID    3
#define MAX_PVR_DMX_NUM         DMX_HW_SW_REAL_NUM

#define PVR_CIPLUS_KEY_LEN        36 //256 bit + 4 B

//##: notice message from PVR module to APP/UI
#define PVR_END_DATAEND            1   // the player playback to the file EOF
#define PVR_END_DISKFULL        2     // the HDD is full!
#define PVR_END_TMS                3    // in the time shifting mode, the player catch up the recorder
#define PVR_END_REVS            4    // in the backword mode, the player gets to the beginning of a file
#define PVR_END_WRITEFAIL        9   // write failed
#define PVR_END_READFAIL        10  // read failed
#define PVR_TMS_OVERLAP           12  // when time-shifting overlapped, notice this msg
#define PVR_STATUS_UPDATE        13    // the status of recorder/player updated, the msg is PREC_INFO.
#define PVR_STATUS_FILE_CHG      14    // the status of recorder/player file changed
#define PVR_STATUS_PID_CHG         15    // the status of PID changed when play file
#define PVR_SPEED_LOW              16    // Speed too low.
#define PVR_STATUS_CHN_CHG         17    // channel changed when playing for one record.

//====== other message type =====
#define PVR_MSG_REC_START        18
#define PVR_MSG_REC_STOP        19
#define PVR_MSG_PLAY_START        20
#define PVR_MSG_PLAY_STOP        21
#define PVR_MSG_UPDATE_KEY        22
#define PVR_MSG_UPDATE_CW        23
#define PVR_MSG_TMS_CAP_UPDATE    24    // timeshift capability update
#define PVR_MSG_REC_START_GET_HANDLE    25
#define PVR_MSG_REC_START_OP_STARTDMX    26
#define PVR_MSG_PLAY_START_GET_HANDLE    27
#define PVR_MSG_PLAY_START_OP_STARTDMX    28
#define PVR_MSG_PLAY_STOP_OP_STOPDMX    29
#define PVR_MSG_PLAY_PTM_UPDATE    30
#define PVR_MSG_PLAY_URI_NOTIFY    31
#define PVR_MSG_PLAY_RL_SHIFT    32
#define PVR_MSG_PLAY_RL_RESET    33 //need adjust to RL A point normal play since the play time become invalid
#define PVR_MSG_PLAY_RL_INVALID    34
#define PVR_READ_SPEED_LOW        35
#define PVR_WRITE_SPEED_LOW        36
#define PVR_MSG_REC_STOP_OP_STOPDMX        37
#define PVR_MSG_REC_GET_KREC            38
#define PVR_MSG_CRYPTO_DATA                39
#define PVR_MSG_REC_SET_REENCRYPT_PIDS    40
#define PVR_MSG_PLAY_SET_REENCRYPT_PIDS    41
#define PVR_MSG_CAL_CHUNK_HASH            42
//Add for CAS9_V6
#define PVR_MSG_REC_STOP_URICHANGE 43
#define PVR_MSG_TMS_STOP_URICHANGE 44
#define PVR_MSG_TMS_PAUSE_URICHANGE 45
#define PVR_MSG_TMS_RESUME_URICHANGE 46
//End
#define PVR_MSG_REC_GET_RP_INFO         47
#define PVR_MSG_REC_GET_FP_KEY_INFO     48
#define PVR_MSG_REC_GET_URI_KEY_INFO    49

#define PVR_MSG_PLAY_FILE_DEL    50
#define PVR_MSG_REC_TOO_SHORT   51
#define PVR_MSG_BLOCK_MODE_DECRYPT   52
//for CAS9
#define PVR_MSG_FORCE_STOP_DATAERR 53

#define PVR_MSG_PLAY_NEAR_END           55
#define PVR_MSG_DATAEND_AND_WAIT        56
#define PVR_MSG_REC_DATA_UPDATE         57

/* play_pos: bit22-31 for file_index, bit0-21 for vobu_index */
#define PLAY_FILE_IDX(pos)         ((UINT16)(((pos) >> 22) & 0x3FF))
#define PLAY_VOBU_IDX(pos)        ((UINT32)((pos) & 0x3FFFFF))
#define PLAY_POS(fidx, vidx)    (((UINT32)(fidx) << 22) | ((UINT32)(vidx) & 0x3FFFFF))

#define PVR_ENABLE_FLAG 0xA5A5

/* definition of Record Special Mode (RSM) */
#define RSM_NONE                    0
#define RSM_CAS9_RE_ENCRYPTION        1
#define RSM_CIPLUS_RE_ENCRYPTION    2
#define RSM_COMMON_RE_ENCRYPTION    3
#define RSM_FINGERP_RE_ENCRYPTION    4 //scrambled record, only fingerprint re-encryption
#define RSM_CAS9_MULTI_RE_ENCRYPTION 5 //multi prog record re-encrypt
#define RSM_BC_MULTI_RE_ENCRYPTION 6 //multi prog record re-encrypt
#define RSM_C0200A_MULTI_RE_ENCRYPTION 7 //for nagra
#define RSM_GEN_CA_MULTI_RE_ENCRYPTION 8//for genCA_PVR

#define GEN_CA_KEY_LEN_MAX      128//measured in byte
#define PVR_CA_MESSAGE_LEN 320  //for CHUNK_TYPE_CAS9_CAMSG
/* Definition of the PVR crypto mode
 *     1. You can add more crypto mode, but don't change the order of the existing ones.
 */
typedef enum
{
    PVR_CRYPTO_MODE_AES_CBC = 0,
    PVR_CRYPTO_MODE_AES_ECB,        // default for encrypting TS packet
    PVR_CRYPTO_MODE_AES_OFB,
    PVR_CRYPTO_MODE_AES_CFB,
    PVR_CRYPTO_MODE_AES_CTR,
    PVR_CRYPTO_MODE_TDES_CBC,        // default for encrypting CAS9 chunks
    PVR_CRYPTO_MODE_TDES_ECB,
    PVR_CRYPTO_MODE_TDES_OFB,
    PVR_CRYPTO_MODE_TDES_CFB,
} PVR_CRYPTO_MODE;

typedef enum
{
    KEY_MODE_OTP    =   0x0,
    KEY_MODE_USER   =   0x1,
} re_encryp_key_mode;

typedef enum
{
    KEY_MODE_DEFAULT    =  0x0,
    KEY_MODE_2_USER     =  0x1,
    key_mode_1_rd       =  0x2,
    key_mode_1_user     =  0x3,
} de_encryp_key_mode;
struct pvr_crypto_config
{
    re_encryp_key_mode key_mode;
    UINT8 *key_buffer;
};
typedef enum
{
    PVR_DEBUG_NONE         = 0,        // no debug information, default setting.
    PVR_DEBUG_PLAYER     = (1 << 0),    // debug for the player
    PVR_DEBUG_RECORDER     = (1 << 1),    // debug for the recorder
    PVR_DEBUG_DATA         = (1 << 2),    // debug for the pvr data information
    PVR_DEBUG_CACHE     = (1 << 3),    // debug the pvr cache infomation
    PVR_DEBUG_DEVICE     = (1 << 4),    // debug the pvr device information
    PVR_DEBUG_OTHER     = (1 << 5),    // debug the pvr other information
    PVR_DEBUG_TS         = (1 << 6),    // debug the pvr ts information
    PVR_DEBUG_FFB       = (1 << 7),     // debug the pvr trick mode
    PVR_DEBUG_ALL         = 0xff,        // open all debug information
} PVR_DEBUG_LEVEL;

typedef enum
{
    PVR_TYPE_INFO = 0,                // file format type info, like "*info.dvr"
    PVR_TYPE_HEAD = 1,
    PVR_TYPE_DATA = 2,                // file format type data, like "000.dvr or 000.mpg"
    PVR_TYPE_TEST = 3,
    PVR_TYPE_PREX = 4,                // '**/AliDvr'
    PVR_TYPE_BC = 5,
    PVR_TYPE_DAT = 6,
} PVR_FFORMAT_TYPE;

enum PVR_PROJECT_MODE
{
    PVR_DVBS2    = (1 << 0),
    PVR_DVBT    = (1 << 1),
    PVR_DVBT_2M = (1 << 2),
    PVR_ATSC    = (1 << 3),
    PVR_DVBS    = (1 << 4),
};

enum
{
    PVR_REC_ITEM_PVR3 = 0,
    PVR_REC_ITEM_OLD33 = 1,
    PVR_REC_ITEM_OLD36 = 2,
    PVR_REC_ITEM_NONE = 3,
};

enum PVR_REC_TYPE
{
    PVR_REC_TYPE_TS = 0,
    PVR_REC_TYPE_PS = 1,
};

struct PVR_DTM
{
    UINT32  mjd;
    UINT16  year;
    UINT8   month;
    UINT8   day;
    UINT8   weekday;
    UINT8   hour;
    UINT8   min;
    UINT8   sec;
};

enum vob_status
{
    VOB_START       = 0,
    VOB_NORMAL      = 1,
    VOB_END         = 2,
    VOB_INVALID     = 3,
};

enum p_open_config_t
{
    P_OPEN_DEFAULT         = 0,
    P_OPEN_VPO_NO_CTRL     = (1 << 0),
    P_OPEN_PREVIEW         = (1 << 1),
    P_OPEN_FROM_HEAD     = (1 << 2),
    P_OPEN_FROM_TAIL    = (1 << 3),
    P_OPEN_FROM_PTM        = (1 << 4),
    P_OPEN_FROM_POS        = (1 << 5),
    // need keep this macro, to compatible with old interface. manager need implement by P_OPEN_FROM_POS
    P_OPEN_FROM_LAST_POS = (1 << 6),
};

// for pvr_p_close stop_mode type define
enum p_stop_mode_t
{
    P_STOP_AND_REOPEN     = 0,    // the devices will be stopped and reopened to playback
    P_STOPPED_ONLY         = 1,    // the devices will be stopped
};

typedef enum
{
    NV_STOP            = 0,         // stop
    NV_PLAY          = 1,         // normal playback
    NV_PAUSE         = 2,         // pause
    NV_FF             = 3,        // fast forward
    NV_STEP          = 4,        // step
    NV_FB             = 5,        // fast backward
    NV_SLOW            = 6,        // slow
    NV_REVSLOW        = 7,        // reverse slow

    NV_RECORDING     = 11,
} PVR_STATE;

//for pvr_data_copy_by_time copy state define
typedef enum
{
    CP_NORMAL        = 0,         // copy to new and don't change source record
    CP_COPY               = 1,        // copy to new and copied part of source record invisible
    CP_CUT            = 2,        // copy to new and cut copied part of source record
    CP_RECOVER    = 3,          //  recover copy source file
} COPY_STATE;

typedef enum
{
    PVR_SPEED_1X = 1,
    PVR_SPEED_2X = 2,
    PVR_SPEED_4X = 4,
    PVR_SPEED_8X = 8,
    PVR_SPEED_16X = 16,
    PVR_SPEED_24X = 24,
    PVR_SPEED_32X = 32,
} PVR_SPEED;

struct copy_param
{
    char *cp_path;          // the destination path of copy record
    UINT16 cp_path_len;   // the length of cp_path
    COPY_STATE cp_state;
    UINT32 start_time;             // start time of the copy, in seconds
    UINT32 end_time;               // end time of the copy
};

struct _dvr_eng_state
{
    UINT8    state;                // the player and recorder state, PVR_STATE
    UINT8    direct;                // the direction
    UINT8    speed;                // the speed
    UINT8    stop_param;            // enum p_stop_mode_t
};

struct pvr_pid_info
{
    UINT8    total_num;            // be sure less than 32, redundant pids will be ignored by PVR and dmx!
    UINT8    audio_count;
    UINT8    ecm_pid_count;
    UINT8    emm_pid_count;
    UINT8    ttx_pid_count;
    UINT8    subt_pid_count;
    UINT8    ttx_subt_pid_count;
    UINT8   cur_audio_pid_sel;
    UINT8   addition_pid_count;
    UINT8    isdbtcc_pid_count;
    UINT8    resv0[2];
    UINT16    video_pid;
    UINT16    audio_pid[MAX_PVR_AUDIO_PID];
    UINT16    audio_lang[MAX_PVR_AUDIO_PID];
    UINT16    pcr_pid;
    UINT16    pat_pid;
    UINT16    pmt_pid;
    UINT16    sdt_pid;
    UINT16    eit_pid;
    UINT16    cat_pid;
    UINT16    nit_pid;
    UINT16    ecm_pids[MAX_PVR_ECM_PID];
    UINT16    emm_pids[MAX_PVR_EMM_PID];
    UINT16    ttx_pids[MAX_PVR_TTX_PID];
    UINT16    subt_pids[MAX_PVR_SUBT_PID];
    UINT16    ttx_subt_pids[MAX_PVR_TTX_SUBT_PID];
    UINT16    isdbtcc_pids[MAX_PVR_ISDBTCC_PID];
    UINT16  addition_pid[MAX_PVR_ADDITION_PID];
};//total 220 bytes! if updated, remember to update record info structure!!

struct record_stop_param
{
    UINT8 tms_clear;
    UINT8 sync;
};

typedef struct
{
    UINT16 v_pid;
    UINT16 a_pid;
    UINT8 dmx_id;
} pvr_eng_cb_param;

typedef struct
{
    struct pvr_pid_info *pid_info;             // input
    UINT16                 *pid_list;             // output
    UINT16                 pid_num;              // input/output
} pvr_crypto_pids_param;

typedef struct
{
    UINT32    pvr_hnd;
    UINT16 *pid_list;
    UINT16    pid_num;
    UINT8    dmx_id;
    UINT8    ca_mode;

    UINT32    key_num;            // total number of keys
    UINT32    key_len;            // the real length of one key, measured in bit
    UINT8  *keys_ptr;            // keys pointer
    UINT8    crypto_mode;        // crypto mode TDES/AES (CBC/ECB)
    //    UINT8    rec_special_mode;
    UINT8    resv[3];
    UINT32    qn_per_key;             // Quantum number per key for re-encrypted data, only used by CAS9

    // return values
    UINT32    crypto_stream_hnd;
    UINT32    crypto_stream_id;
    UINT32    crypto_key_pos;

    void *user_ce_key;
    UINT32    crypto_key_first_pos;
} pvr_crypto_general_param;

typedef struct
{
    UINT32    key_num;        // total number of keys
    UINT32    key_len;        // the real length of one key, measured in bit
    UINT8  *keys_ptr;        // keys' pointer
} pvr_crypto_key_param;

typedef struct
{
    UINT8  *output;
    UINT8  *input;
    UINT32    data_len;
    UINT8  *key_ptr;        // key pointer, the key is encrypted by PK (PVR Key)
    UINT8  *iv_ptr;            // Initialization Vector pointer, IV len equal to key_len
    UINT32    key_len;        // the real length of the key, measured in bit
    UINT8    crypto_mode;    // crypto mode TDES/AES (CBC/ECB)
    UINT8    encrypt;        // 1 for encrypt, 0 for decrypt
} pvr_crypto_data_param;

typedef struct
{
    UINT32  pvr_hnd;
    UINT8   *input;
    UINT32  length;
    UINT32  block_idx;
} pvr_block_decrypt_param;

typedef struct
{
    UINT32 ptm;                // in seconds
    UINT32 rating;
} pvr_cas9_mat_rating;        // Maturity Rating

typedef struct
{
    UINT32  pvr_handle;
    UINT8   *data_buffer;
    UINT32  data_size;
    UINT32  quantam_size;
    UINT32  quantam_idx;

    UINT16  video_pid;
    UINT8   video_type;
    UINT16  sequence_header_offset_out;     //out param,  count by ts packet number
} pvr_rec_data_param;

#if 1//chunpin_conax6_eng
#define  CONAX6_URI_CHUNK_NUM 2
//#define TS_REC_INFO_SIZE 32*1024

#define CHUNK_TYPE_CONAX6_URI        6
#define CHUNK_TYPE_CONAX6_REC_KEY    2
#define CONAX6_KERC_LEN                128        // measured in bit
#define CONAX6_KERC_NUM_MAX            15

#define CONAX6_URI_IN_GROUP                100
#define CONAX6_URI_GROUP_IN_CHUNK        40
#define  CONAX6_MAX_URI_IN_CHUNK        (CONAX6_URI_IN_GROUP * CONAX6_URI_GROUP_IN_CHUNK)

#define CONAX6_URI_ID_MAX_LEN            64

#define ERROR_URI_NON_CAS9                      2
#define ERROR_HMAC_VERIFY_FAIL    3
#define ERROR_CHUNK_FAIL    4
#define ERROR_COMMON_FAIL    5
#define ERROR_URI_DISKERR                 6
#define RET_URI_SUCCESS        0
enum
{
    CONAX6_CHUNK_MGR_REC  = 0,
    CONAX6_CHUNK_MGR_PLAY = 1,
    CONAX6_CHUNK_MGR_NUM
};

typedef struct
{
    UINT16 year;
    UINT32 month    : 4;
    UINT32 reserved     : 4;
    UINT32 day        : 5;
    UINT32 hour        : 5;
    UINT32 minute    : 6;
    UINT32 sec      : 8;
} detail_time;

typedef struct
{
    //Time stamp
    detail_time dt;
    UINT32    ptm;

    //URI properties
    UINT8 bap_default    : 1;
    UINT8 bap_ecm_mat    : 1;
    UINT8 bap_res1        : 2;
    UINT8 bap_pvr_mat    : 4;   //1:G,2:PG,4:A,8:X,9:XXX

    //export
    UINT8 bex_res1        : 1;
    UINT8 bex_res2        : 3;
    UINT8 bex_ciplus        : 1;
    UINT8 bex_pbda        : 1;
    UINT8 bex_dtcp        : 1;
    UINT8 bex_hndrm        : 1;

    //URI
    UINT16 buri_aps    : 2;
    UINT16 buri_emi    : 2;
    UINT16 buri_ict    : 1;
    UINT16 buri_rct    : 1;
    UINT16 buri_retlimit    : 3;
    UINT16 buri_trickplay    : 3;
    UINT16 buri_mat    : 3;
    UINT16 buri_da    : 1;

    //UINT8   resv1[16];

} conax6_uri_item;     //32-byte align for hash calculation

typedef struct
{
    UINT8    type;            // chunk type indication, CHUNK_TYPE_CAS9_FP
    UINT8    resv0[3];
    UINT32    length;             // total length of this chunk

    UINT16    magic;            // MUST be CAS9_MAGIC_NUMBER
    UINT8    resv1[2];

    UINT32    start_ptm;
    UINT16    uri_num;
    UINT8    resv2[2];

    UINT32    uri_groups[CONAX6_URI_GROUP_IN_CHUNK];        // start ptm for each group
    conax6_uri_item uris[CONAX6_MAX_URI_IN_CHUNK];        // 32-Byte for each

    UINT32      verified_len;                               //Time length in secord
    UINT8       pad1[700 + 608 - 4];
    UINT8       pad2[12];
    UINT8       hash[32];
    UINT8    crc[4];
} conax6_uri_chunk;


typedef struct
{
    conax6_uri_chunk    chunk;
    ID                lock;
    UINT8            ref_cnt;
    UINT16            chunk_idx;
} conax6_uri_chunk_mgr;

#endif


struct record_prog_param
{
    UINT16 mode;             // RECORDING_TIMESHIFT:1, RECORDING_NORMAL:0
    BOOL is_scrambled;       // 0: record as descramble,
    // 1: record as scramble, only active when the program is CA program.
    char folder_name[1024];   // full path of the record or
    UINT8 full_path;         // 1: folder_name is full path.
    UINT8 av_flag;           // 1: TV, 0:radio
    UINT8 lock_flag;         // 1: Locked, 0: unlock
    UINT8 ca_mode;           // 1: scramble program, 0: FTA program
    UINT8 h264_flag;         // type defined in vdec.h: enum video_decoder_type{}
    UINT8 audio_channel;
    UINT8 nim_id;
    UINT8 dmx_id;
    UINT8 live_dmx_id;
    UINT32 channel_id;
    UINT16 cur_audio_pid_sel;        // for multi audio track, indicator current audio track
    struct pvr_pid_info pid_info;    // record PID list.
    char service_name[36];
    UINT16 name_len;
    UINT16 prog_number;
    UINT8 rec_type;                    // 0:TS, 1:PS
    UINT8 append_to_exist_file;        // 0:record as new item,
    // 1: find the record item, if success, append to the file end
    UINT8 continuous_tms;            // 1: enable, 0: disable, only enable when mode is RECORDING_TIMESHIFT
    UINT8 record_whole_tp_data;        // 1: whole tp record, 0: only record the pid_info.
    UINT32 tms_total_size;            // in KBytes, only active while mode is RECORDING_TIMESHIFT
    UINT32 ts_bitrate;                // in bps, current record channel bitrate
    UINT32 record_file_size;        // in KBytes, recomment to 900M, if FAT32 fs almost 4G, NTFS no limit.
    UINT32 ciplus_pvr_enable;        // must be 0xA5A5
    UINT8 ciplus_resv1: 7;
    UINT8 is_reencrypt: 1;
    UINT8 ciplus_resv[2];
    UINT8 rec_special_mode;
    UINT32 partition_id[2];
};

struct playback_stop_param
{
    UINT8 stop_mode;
    UINT8 vpo_mode;
    UINT8 sync;
    UINT8 no_blackscr : 1;
    UINT8 resv1        : 7;
    PVR_HANDLE reopen_handle;        // only enabled when stop_mode is P_STOP_AND_REOPEN
};

struct playback_param
{
    UINT16 index;            // playback index
    char path[1024];        // playback reccord item
    PVR_STATE state;        // playback state
    UINT32 start_time;      // start time
    UINT32 start_pos;       // only when start_mode is P_OPEN_FROM_POS.
    UINT8 speed;            // playback speed
    UINT8 start_mode;       // open mode: enum p_open_config_t
    UINT8 dmx_id;           // playback dmx id
    UINT8 live_dmx_id;      // live play dmx id
    UINT8 preview_mode;        // 1: preview_mode, 0: full screen playback mode.
    UINT8 vpo_closed;       // indicated wheather closed vpo when state trans ;1:vpo closed 0:vpo donnot closed
};

struct pvr_dmx_delay_param
{
    UINT32 delay;    // measured in ms
    UINT8  nim_id;
    UINT8  ts_id;
    UINT8  dmx_id;
    UINT8  nim_type;//to seperate the front end type DVBS=0,DVBT=1,DVBC=2;ISDBT=3,
    BOOL   is_using_dmx2;
};

typedef struct
{
    UINT8  stat_end_grp_num;// how may start_end positions.
    UINT8  resv1[3];
    UINT32  start_end_state;// which start_end positions is active by bit.
    UINT32  start_end[START_END_MAX_NUM][2]; // start end positions.
    UINT32 resv2[20];
} START_END_INFO, *PSTART_END_INFO;

typedef struct
{
    UINT32 index;
    UINT32 start_pos;
    UINT32 end_pos;
} START_END_PARAM, *PSTART_END_PARAM;

typedef struct
{
    char    txti[TXTI_LEN];
    UINT8    data_version;    // the version of recorded data
    UINT8    pvr_version;    // the version of this pvr module
    UINT8    rec_mode;        // 0:Normal record, 1: time-shifting
    UINT8    nim_id;            // the id for nim device
    UINT8    channel_type;    // 0: video channel, 1: radio channel.

    UINT16    r_cache_id;        // the cache id, indicate which cache will be used.
    UINT16    p_cache_id;        // the cache id, indicate which cache will be used.
    UINT16    r_dmx_id;        // the dmx id
    UINT16    p_dmx_id;        // the dmx id

    UINT32    channel_id;        // the channel id in the database,
    // pvr engine not use this value, only as additional information

    UINT8     lock_flag;        // to compatible with old structure
    UINT8   is_h264;        // h264 program indicator
    UINT8    is_scrambled;    // indicate this record is scramble or FTA.
    UINT8    audio;            // bit0~3: audio type(0 --MPEG 1, 1 --MPEG 2); bit4~7: audio channel

    UINT16    prog_number;
    UINT8    ca_mode;         // program is scrambled? 1:yes, 0, FTA.
    UINT8    is_timeshift;

    UINT8   rec_type;        // 0:TS, 1:PS
    UINT8    is_recording;     // this record is recording.
    UINT8     old_item;        // indicator old record item
    UINT8     record_pid_num; // add for dynamic pid

    UINT8 is_append;
    UINT8 continue_tms;
    UINT8 resv: 7;
    UINT8    is_reencrypt: 1;   // 0: FTA, 1: re-encrypt
    UINT8 retention_limit: 6;
    UINT8 copy_control: 2;
    UINT16 record_pids[PVR_MAX_PID_NUM]; // add for dynamic pid
    struct pvr_pid_info pid_info;
    START_END_INFO se_info; // start and end point group info
    struct  PVR_DTM tm;        // The start recording time information for the Recording List
    UINT32 start_ptm;
    UINT32  duration;        // The playback/recording duration for this Recording List.
    UINT32    ts_bitrate;        // bps
    UINT32    size;             // in KBytes, record item total size.
    UINT32 record_file_size;// in Kbytes, one record ts file file.
    UINT8  rec_special_mode;
    UINT8 record_de_encryp_key_mode;
    UINT8 is_not_finished;
} REC_ENG_INFO, *PREC_ENG_INFO; // pvr engine record item info for external

///////////////////////////////////////////////////
// to support continuous tms and append function.
typedef struct
{
    UINT8 is_append;        // record mode is append.
    UINT8 continue_tms;        // record mode is continue-tms.
    UINT8 is_h264;            // h264 program indicator
    UINT8 is_scrambled;        // scramble

    UINT8 audio;            // bit0~3: audio type(0 --MPEG 1, 1 --MPEG 2); bit4~7: audio channel
    UINT8 ca_mode;             // $** or ** prog
    UINT8 rec_type;            // 0:TS, 1:PS
    UINT8 channel_type;        // 0: radio channel, 1: video channel.
    UINT32 ts_bitrate;        // bps
    UINT8 prog_number;
    UINT8 resv0[3];

    struct pvr_pid_info pid_info;
    UINT8  resv[784];
} H_CHAN_INFO, *PH_CHAN_INFO; //total 1KB, it will be write to head file

///////////////////////////////////////////////////

typedef INT8(*notice_callback)(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code);
typedef BOOL (*data_encrypt_callback)(UINT8 *buffer, UINT32 *length, UINT8 type, int64 file_offset); // type: 1 encrypt, 0, descramble //jeremy test
typedef BOOL (*file_format_callback)(const char *path_prex, PREC_ENG_INFO info,
                                     PVR_FFORMAT_TYPE file_type, UINT32 file_idx, char *full_path, UINT32 buff_size);
typedef void (*get_local_time_callback)(struct PVR_DTM *tm);
typedef BOOL (*info_saving_callback)(PVR_HANDLE handle, UINT8 *user_data, UINT32 len);
typedef void (*tsg_play_task_loop_callback)(void);
typedef void (*tsg_play_task_set_packet_num_callback)(UINT32 packet_num);
typedef BOOL (*pfn_prog_id_to_record_param)(UINT32 prog_id, struct record_prog_param *prog_info);
typedef UINT32(*record_size_update_callback)(PVR_HANDLE handle, INT32 size_k);

typedef struct
{
    // common config
    UINT32 cache_addr;       // vob cache address, please set it from a contiunous buffer.
    UINT32 cache_size;       // vob cache size, in bytes, the size is determined by the record and play capability.
    // for HD solution: cache_size = (max_rec_number+max_play_number)*250*47K
    // for SD solution: cache_size = (max_rec_number+max_play_number)*30*47K

    UINT8 max_rec_number;        // recorder number, currently we support 3 recorder.
    UINT8 max_play_number;        // play number, currently we only support 1 player.
    UINT16 prj_mode;                // enum PVR_PROJECT_MODE, Ali different solution using.

    UINT8 continuous_tms_en;    // continuous time-shifting enable.
    UINT8 ac3_decode_support;    // support AC3.
    UINT8 recover_enable;        // enable recover the video callback interrupt when do trick mode.
    UINT8 debug_level;            // refer to PVR_DEBUG_LEVEL, call libc_printf to print some message.

    UINT32 ps_packet_size;        // record as PS format, the size of each packet, 188*16(m36), 188*8(m33)
    UINT16 rec_state_update_freq;// in ms, recomment to 1000ms, the minimal value is 1000ms
    UINT16 ply_state_update_freq;// in ms, recomment to 1000ms, the minimal value is 1000ms
    UINT32 resv1[4];

    notice_callback event_callback;         // notice application callback
    data_encrypt_callback encrypt_callback;    // user can encrypt the recording data, and decord when playing
    file_format_callback name_callback;        // manage the file format name by this callback.
    get_local_time_callback local_time_callback;
    info_saving_callback info_saving;
    tsg_play_task_loop_callback play_task_loop;
    tsg_play_task_set_packet_num_callback play_task_set_packet_num;
    record_size_update_callback rec_size_update_callback;

    // record format config
    char dvr_path_prefix[256];     // like "AliDvr"
    char info_file_name[256];     // like "info.dvr"
    char info_file_name_new[256];     // like "info3.dvr"
    char ts_file_format[256];     // like "dvr"
    char ts_file_format_new[256];    // like "ts"
    char ps_file_format[256];     // like "mpg"
    char test_file1[256];         // like "test_write1.dvr"
    char test_file2[256];         // like "test_write2.dvr"

    // record attribute config
    UINT32 h264_vobu_time_len;    // in ms, scope: 500-2000ms, recommend to 600ms
    UINT32 scramble_vobu_time_len;    //in ms, scope: 500-2000ms,    recommend to 600ms
    UINT32 file_header_save_dur;// in seconds, recomment to 30s, file head save cycle, min duration is 15s.
    UINT32 record_min_len;      // in seconds, recomment to 15s, the record will be deleted if shorter that this limit
    UINT32 tms_time_max_len;    // in seconds, recomment to 2h(7200);
    UINT32 tms_file_min_size;    // in MBytes,  recomment to 10M

    char storeinfo_file_name[256];     // like "info.dvr"
    char datinfo_file_name[256];  //"xxx.dat"
    UINT8 crash_protect;
    UINT32 new_trick_mode;  //Only for PVR3.1, no used anymore in PVR3.2 and later version.
    UINT32 ff_trick_mode_skip; //Only for PVR3.1, no used anymore in PVR3.2 and later version.
    UINT32 trim_record_ptm;
} PVR_CFG, *PPVR_CFG;

typedef struct
{
    PVR_HANDLE handle;        // handle for the record
    UINT8 id;                // record id
    UINT32 start_tm;        // record start time, in ms.
    UINT32 end_tm;            // record end time, in ms.
    UINT32 speed_reference;    // the low speed count for each second.
    UINT32 real_write_speed;// in kbytes/s, real speed of writing data to storage device
    struct _dvr_eng_state state;    //the playback state
    REC_ENG_INFO detail;    // this record detail information.
} REC_INFO, *PREC_INFO;

typedef struct
{
    PVR_HANDLE handle;
    UINT8     id;
    UINT8     use_tsgen;        // player use the ts-generate route.
    UINT32    play_pos;        // store the point, file index and vobu index
    UINT32    ptm;            // playback time,in seconds
    UINT32 speed_reference;    // the low speed count for each second.
    UINT32 real_read_speed; // in kbytes/s, real speed of reading data from storage device
    struct _dvr_eng_state state;    //the playback state
    H_CHAN_INFO play_chan_info; // the detail information for the channel for append function.
    REC_ENG_INFO detail;    // this record detail information.
} PLY_INFO, *PPLY_INFO;

typedef struct
{
    UINT8 tms_continuous; //whether now is using continuous tms mode
    UINT8 tms_cross_prog; //whether crossed programs while continuous tms
    UINT8 tms_last_update_num; //for continuous tms updated Q num (but not calc in VOBU) remeber while change channel
    UINT32 tms_file_min_size_mbytes;    // in MBytes.
    UINT32 tms_ring_buffer_size_mbytes;    // in MBytes.
    UINT32 tms_file_size_mbytes;        // in MBytes.
    UINT32 tms_real_free_size_kbytes;    // in KBytes.
    UINT32 tms_capability;                 // in seconds
    UINT8 tms_file_num;
    UINT8 tms_file_active;
    char *tms_file_name;
    char *tms_path;
} TMS_INFO, *PTMS_INFO;

typedef struct
{
    INT64 pcr_min;
    INT64 pcr_max;
    UINT64 file_time_length;    // time in second
    INT64 file_data_length;    // data in byte
    UINT32 bitrate;
} tsfile_info;

typedef struct
{
    UINT32 ptm;         // in ms
    UINT8  fp_height;
    UINT8  fp_priority; // 0:default, 1:from LPK
    UINT16 fp_duration;    // in 10ms
    UINT16 fp_pos_x;
    UINT16 fp_pos_y;
    UINT8  fp_id[64];
} pvr_finger_info;

typedef struct
{
    UINT32 ptm;         // in ms
    UINT8  fp_data[508];
} pvr_finger_info_ext;

typedef struct
{
    UINT32 ptm;         // in ms
    UINT8  camsg[PVR_CA_MESSAGE_LEN];
} pvr_ca_message;


typedef struct
{
    struct list_head    listpointer;
    UINT32  item_uid;   //unique id
    UINT8   name[256];  //full_path of directory,like "/mnt/uda1/ALIDVBS2/xxxxname xxxx:xx:xx xx:xx:xx"
    time_t  time;
    UINT8   save_time[20];//xxxx-xx-xx xx:xx:xx
    UINT8   channel_type;   //audio : 1, video:0
    UINT8   crypto; //1:cry
    UINT32  record_size;
    UINT32  duration;
    UINT32  valid_recorded_time;
    UINT32  ts_bitrate;
    UINT16  file_num;
    UINT16  first_file_idx;
    UINT8   direction;//0: ascending  1:disacending

} pvr_eng_record_basic_info;

typedef struct
{
    UINT32 ptm;
    UINT32 offset;

} pvr_eng_time_info;

typedef struct
{
    UINT16 file_idx;
    pvr_eng_time_info *time_info;
    UINT32 time_info_num;
} pvr_eng_rec_head_info;


typedef struct
{
    FILE *p_file;  //FILE *: ts data file's handle
    char *rec_path;
    UINT8   direction;//0: ascending  1:disacending
    UINT16 total_file_nums;
    UINT16 last_file_idx;
    UINT16 cur_file_idx;
    UINT32 cur_file_size;
    UINT32 file_cur_read_offset;
    UINT32 file_next_read_offset;
    pvr_eng_rec_head_info rec_cur_head_info;
    pvr_eng_rec_head_info rec_next_head_info;

} pvr_eng_rec_info;


int pvr_eng_get_rec_basic_info(const char *item_path, pvr_eng_record_basic_info *node);
int pvr_eng_get_head_info(const char *rec_name, UINT16 idx, pvr_eng_rec_head_info *p_head_info);


typedef enum
{
    // PVR Engine Record IO
    // parameter            // param1,                 param2
    PVR_ERIO_PAUSE = 0,        // 0,                    0
    PVR_ERIO_RESUME,        // 0,                     0
    PVR_EPIO_RESTART,        // 0:start, 1:end,      0
    PVR_ERIO_TMS2REC,        // UINT32 *time,         0
    PVR_ERIO_GET_TIME,        // UINT32 *time,        0,
    PVR_ERIO_GET_INFO,        // PREC_INFO *info,        0,
    PVR_ERIO_GET_TMS_INFO,    // PTMS_INFO *info,        0,
    PVR_ERIO_CHG_PID,        // UINT16 pid_num,        UINT16 *pid_list
    PVR_ERIO_CHG_MODE,        // 0,                    0
    PVR_EPIO_SET_TMS_PATH,    // UINT8* name            0
    PVR_ERIO_ADD_PID,                 //UINT16  pid_num,         UINT16* pids
    PVR_ERIO_CLEAR_TMS,
    PVR_ERIO_GET_REC_CNT,
    PVR_ERIO_GET_MS_TIME,
    PVR_ERIO_SET_CAS9_MAT_RATING,
    PVR_ERIO_SET_CAS9_FINGER_PRINT,
    PVR_ERIO_SET_CAS9_FINGER_PRINT_EXT,
    PVR_ERIO_GET_TMS_START_MS,

    // PVR Engine Play IO
    PVR_EPIO_TIMESEARCH = 50,// UINT32 ptm,         0
    PVR_EPIO_JUMP,            // INT32 ptm,            0
    PVR_EPIO_SEEK,            // UINT32 pos,            0

    PVR_EPIO_SET_STARTEND,    // START_END_PARAM*,    0
    PVR_EPIO_CLR_STARTEND,    // UINT32 index,        0

    PVR_EPIO_SWITCH_A_PID,    // UINT16 pid,             0
    PVR_EPIO_SWITCH_A_CHAN,    // UINT8 channel,        0
    PVR_EPIO_LOCK_SWITCH,    // UINT32 lock_en,      0
    PVR_EPIO_GET_POS,        // UINT32 *pos,         0
    PVR_EPIO_GET_POS_TIME,    // UINT32 *time,        0
    PVR_EPIO_GET_TIME,        // UINT32 *time,        0
    PVR_EPIO_GET_INFO,        // PPLY_INFO *info,     0
    PVR_EPIO_GET_CUR_APID,
    PVR_EPIO_CHANGE_PRE_MODE,
    PVR_EPIO_SWITCH_A_PIDL,    // 0,    UINT16 *pid_list
    PVR_EPIO_RECOVER_TO_LIVE,
    PVR_EPIO_GET_MS_TIME,
    PVR_EPIO_GET_CAS9_MAT_RATING,
    PVR_EPIO_GET_CAS9_FINGER_PRINT,
    PVR_EPIO_GET_CAS9_FINGER_PRINT_EXT,
    PVR_EPIO_SET_CAS9_LAST_PLAY_PTM,
    PVR_EPIO_GET_CAS9_LAST_PLAY_PTM,
    PVR_MGRIO_SAVE_KEY_INFO,
    PVR_MGRIO_GET_KEY_INFO,
    PVR_ERIO_SET_CAS9_CA_MESSAGE,  //for CHUNK_TYPE_CAS9_CAMSG
    PVR_EPIO_GET_CAS9_CA_MESSAGE,  //for CHUNK_TYPE_CAS9_CAMSG
    PVR_CHECK_BITRATE,
    PVR_GET_BUF_VALID,
    PVR_EPIO_GET_REC_SPCIAL_MODE,
    PVR_EPIO_GET_PLY_SPCIAL_MODE,
    PVR_EPIO_GET_EMM_PID,
    PVR_EPIO_GET_ECM_PID,
    PVR_EIO_TOTAL = 100,
} PVR_ENG_IO, *PPVR_ENG_IO;


typedef struct
{
    UINT32   key_index;
    UINT32   key_len;
    UINT8    key_data[GEN_CA_KEY_LEN_MAX];
    UINT8    key_type;
    UINT32   ptm;
} gen_ca_key_info;

struct gen_ca_key_info_data
{
    gen_ca_key_info   key_info_pre;
    gen_ca_key_info   key_info_next;
};

struct gen_ca_crypto_info
{
    UINT32 gen_ca_key_cnt;
    UINT32 key_len;
    UINT8  key_type;
    UINT8  crypto_mode;
    UINT8  sub_device_id;
};

#ifdef __cplusplus
extern"C" {
#endif

//##: PVR module initial and cleanup functions
extern RET_CODE     pvr_eng_attach(PPVR_CFG ini_param);
extern RET_CODE    pvr_eng_get_cfg(PPVR_CFG *ppcfg);
extern RET_CODE     pvr_eng_detach();

//## PVR record API
extern PVR_HANDLE    pvr_eng_r_open(struct record_prog_param *param);
extern UINT8        pvr_eng_r_close(PVR_HANDLE *handle, struct record_stop_param *stop_param);
extern RET_CODE    pvr_eng_ioctl(PVR_HANDLE handle, UINT32 cmd_type, UINT32 param1, UINT32 param2);

//##: PVR playback API
extern PVR_HANDLE     pvr_eng_p_open(struct playback_param *param);
extern BOOL        pvr_eng_p_close(PVR_HANDLE *handle, struct playback_stop_param *stop_param);
extern RET_CODE     pvr_eng_p_play_mode(PVR_HANDLE handle, UINT8 new_state, UINT32 param);

//##: PVR addition API
extern BOOL     pvr_eng_get_user_data(PVR_HANDLE handle, char *path, void *buffer, UINT32 *len);
extern BOOL     pvr_eng_set_user_data(PVR_HANDLE handle, char *path, void *buffer, UINT32 len);
extern BOOL     pvr_eng_get_record_info(PVR_HANDLE handle, char *path, PREC_ENG_INFO info);
extern  BOOL     pvr_eng_set_record_info(PVR_HANDLE handle, char *path, PREC_ENG_INFO info);
extern BOOL    pvr_dmx_delay_start_ext(struct pvr_dmx_delay_param *param,
                                       struct record_prog_param *prog_info);
extern  void     pvr_dmx_delay_stop(void);
extern   void    pvr_dmx_delay_stop_ext(struct pvr_dmx_delay_param param);
extern  void    pvr_dmx_delay_pause(BOOL b_pause);
extern   BOOL     pvr_dmx_delay_start(UINT8 dmx_id, UINT8 nim_id, UINT32 delay,
                                      struct record_prog_param *prog_info);
extern   BOOL  pvr_dmx_delay_set_front(UINT8 i_tsa, UINT8 i_tsb);
extern   BOOL pvr_eng_is_our_file(char *name, PREC_ENG_INFO rec_eng_info);
extern   RET_CODE pvr_eng_copy(struct playback_param *param, struct copy_param cp_param);
extern   RET_CODE pvr_eng_parse_ts_file(char *ts_file_name);
extern   int pvr_eng_parse_ts_file_dir(char *dir_path);
extern   BOOL pvr_eng_get_user_data_with_type(char *path, void *buf_user, UINT32 *len, UINT32 item_type);
extern   BOOL pvr_eng_set_user_data_and_info(char *path, void *buffer,
                                             PREC_ENG_INFO info, UINT32 len, UINT32 item_type);
extern   UINT8 get_current_play_key_mode(void);
extern   BOOL pvr_eng_check_chunks(char *path, PREC_ENG_INFO rec_eng_info);
extern   UINT32 _pvr_data_check_tms_capability(UINT32 ts_bitrate, UINT32 tms_total_size_k, PTMS_INFO tms_info);
extern   BOOL pvr_eng_set_header_key(char *dir_path, UINT8 *buf, UINT32 len);
extern  BOOL pvr_eng_set_header_key(char *dir_path, UINT8 *buf, UINT32 len);
extern  BOOL pvr_eng_get_cur_play_file_idx(PVR_HANDLE handle, char *rec_dir, UINT16 *file_idx);
extern  BOOL pvr_eng_get_header_priv(PVR_HANDLE handle, char *rec_dir, UINT16 file_idx, UINT8 *buf, UINT32 *len);
extern  BOOL pvr_eng_get_cur_rec_file_idx(PVR_HANDLE handle, char *rec_dir, UINT16 *file_idx);
extern  BOOL pvr_eng_set_header_priv(PVR_HANDLE handle, char *rec_dir, UINT16 file_idx, UINT8 *buf, UINT32 *len);
INT32 _pvr_r_request(PVR_HANDLE handle, UINT8 **addr, INT32 length, INT32 *indicator);
BOOL _pvr_r_update(PVR_HANDLE handle, UINT32 size, UINT16 offset);
INT32 _pvr_p_request(PVR_HANDLE handle, UINT8 **addr, INT32 length, INT32 *indicator);
PVR_HANDLE pvr_eng_get_last_rec_hnd(void);
void printf_pid_info(struct pvr_pid_info *info);
BOOL pvr_eng_get_cas9_key_info(PVR_HANDLE handle, pvr_crypto_general_param *param);
BOOL pvr_eng_set_cas9_play_key_pos(PVR_HANDLE handle, UINT32 key_pos, UINT32 first_key_pos);
BOOL pvr_eng_get_header_key(char *dir_path, UINT8 *buf, UINT32 len);
void pvr_eng_get_real_speed(UINT32 *read_speed, UINT32 *write0_speed, UINT32 *write1_speed, UINT32 *write2_speed);
BOOL pvr_eng_delay_start(struct pvr_dmx_delay_param *param, struct record_prog_param *prog_info);
BOOL pvr_eng_delay_stop(struct pvr_dmx_delay_param *param);
void pvr_eng_delay_pause(BOOL b_pause);
PVR_HANDLE pvr_eng_get_free_recoder_handle(UINT16 mode);
UINT32 pvr_eng_get_share_buff(UINT32 size_k, UINT32 *addr);
void pvr_eng_set_share_buff_free(UINT32 addr);
UINT8 pvr_eng_get_share_count();
PVR_HANDLE pvr_eng_get_free_player_handle();
UINT32 pvr_eng_get_data_skip_type(void);
void pvr_eng_set_disk_read_speed(UINT32 read_speed);
UINT32 pvr_eng_get_disk_read_speed(void);


FILE *_fopen(const char *path, const char *opt);
INT32 _fclose(FILE *fd);
ssize_t _fread(UINT8 *buffer, INT32 size, FILE *fd);
ssize_t _fwrite(UINT8 *buffer, INT32 size, FILE *fd);
off_t _fseek(FILE *fd, off_t offset, int whence);
off_t _ftell(FILE *file);
INT32 _fsync(char *dev);
INT32 _rm(const char *path);
#ifndef PVR_FS_API
DIR *_opendir(const char *path);
BOOL _readdir(DIR *dir, f_dirent *dent_item);
INT32 _rewinddir(DIR *dir);
INT32 _closedir(DIR *dir);
#endif
INT32 _mkdir(const char *path);
INT32 _rmdir(const char *path);
INT32 _copydir(const char *src_path, const char *dest_path, off_t offset);
INT32 _fsync_root(const char *dev);
char *_strindex(char *str, char ch, unsigned int index);
INT32 _emptydir(const char *path);

#ifdef __cplusplus
}
#endif


// the sample of pvr engine
#ifdef PVR_SAMPLE_CODE
//1. initialize flow

INT8 pvr_notice_callback(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
}

void init_demo()
{
    PVR_CFG ini_param;
    MEMSET(ini_param, 0, sizeof(PPVR_CFG));
    ini_param.cache_addr = 0xa4000000;
    ini_param.cache_size = ((47 * 1024) * 280 * 3 + 0x1000); // for HD 2 writer and 1 reader
    //ini_param.cache_size = ((47*1024)*30*3+0x1000); // for SD 2 writer and 1 reader
    ini_param.max_rec_number = 2;
    ini_param.max_play_number = 1;
    ini_param.prj_mode = PVR_DVBS2;
    ini_param.continuous_tms_en = 0;
    ini_param.ac3_decode_support = 1;
    ini_param.pvr_name_in_unicode = 0;
    ini_param.debug_level = PVR_DEBUG_NONE;
    ini_param.ps_packet_size = 188 * 16;
    ini_param.rec_state_update_freq = 1000;
    ini_param.ply_state_update_freq = 1000;
    ini_param.event_callback = pvr_notice_callback;
    ini_param.encrypt_callback = NULL;
    ini_param.name_callback = NULL;    // use defualt file format rule.
    strncpy(ini_param.dvr_path_prefix, "AliDvrS2", 256 - 1);
    strncpy(ini_param.info_file_name, "info.dvr", 255);
    strncpy(ini_param.info_file_name_new, "info3.dvr", 255);
    strncpy(ini_param.ts_file_format, "dvr", 255);
    strncpy(ini_param.ps_file_format, "mpg", 255);
    strncpy(ini_param.test_file1, "test_write1.dvr", 255);
    strncpy(ini_param.test_file2, "test_write2.dvr", 255);
    strncpy(ini_param.storeinfo_file_name, "storeinfo.dvr", 255);

    strncpy(ini_param.datinfo_file_name, "000.dat", 255);
    ini_param.h264_vobu_time_len = 600;    // in ms, scope: 500-2000ms, recommend to 600ms
    ini_param.scramble_vobu_time_len = 600;    //in ms, scope: 500-2000ms,    recommend to 600ms
    ini_param.record_file_size = 900;    // in MBytes, recomment to 900M, if FAT32 fs < 3G, NTFS no limit.
    ini_param.file_header_save_dur = 30;// in seconds, recomment to 30s, file head save cycle, min duration is 15s.
    ini_param.record_min_len = 15; // in seconds,recomment to 15s,the record will be deleted if shorter that this limit
    ini_param.tms_time_max_len = 7200;    // in seconds, recomment to 2h(7200);
    ini_param.tms_file_min_size = 10;    // in MBytes,  recomment to 10M
    pvr_eng_attach(&ini_param);
}
//2. record flow
void record_demo(UINT32 chan_idx)
{
    PVR_HANDLE hdl;
    UINT32 record_time;
    BOOL sync = TRUE;
    P_NODE p_node;
    get_prog_at(chan_idx, &p_node);
    struct record_prog_param rec_param;
    MEMSET(rec_param, 0, sizeof(struct record_prog_param));
    rec_param.mode = RECORDING_NORMAL;                    // RECORDING_TIMESHIFT:1, RECORDING_NORMAL:0
    // 0: record as descramble, 1: record as scramble, only active when the program is CA program.
    rec_param.is_scrambled = 0;
    strncpy(rec_param.folder_name, "/mnt/uda1/AliDVBS2/CCTV1_20090303", 1023);     // full path of the record or
    rec_param.full_path = 1;                // 1: folder_name is full path.
    rec_param.av_flag = p_node.av_flag;                  // 1: TV, 0:radio
    rec_param.lock_flag = p_node.lock_flag;                // 1: Locked, 0: unlock
    rec_param.ca_mode = p_node.ca_mode;                    // 1: scramble program, 0: FTA program
    rec_param.h264_flag = p_node.h264_flag;                // 1: h.264 program, 0:MPEG2 program.
    rec_param.audio_channel = p_node.audio_channel;            // 0: left, 1, right, 2 stereo
    rec_param.nim_id = 0;                    // for the dual tuner solution, point one demod.
    rec_param.dmx_id = 1;                    // use dmx1 to record
    rec_param.live_dmx_id = 0;                // use dmx0 to do live play
    rec_param.channel_id = chan_idx;
    rec_param.cur_audio_pid_sel = p_node.cur_audio;        // for multi audio track, indicator current audio track
    rec_param.pid_info.video_pid = p_node.video_pid;
    rec_param.pid_info.audio_pid = p_node.audio_pid;
    rec_param.pid_info.pcr_pid = p_node.pcr_pid;
    rec_param.pid_info.total_num = 3;
    MEMCPY(rec_param.service_name, p_node.service_name, p_node.name_len);
    rec_param.name_len = p_node.name_len;
    rec_param.prog_number = p_node.prog_number;
    rec_param.rec_type = 0;                // record as TS.
    // 0:record as new item, 1: find the record item, if success, append to the file end
    rec_param.append_to_exist_file = 0;
    rec_param.continuous_tms = 0;            // 1: enable, 0: disable, only enable when mode is RECORDING_TIMESHIFT
    rec_param.record_whole_tp_data = 0;        // 1: whole tp record, 0: only record the pid_info.
    rec_param.tms_space_limit_size = 0;    // only active while mode is RECORDING_TIMESHIFT
    hdl = pvr_eng_r_open(&rec_param);
    record_time = pvr_r_get_time(hdl);
    pvr_eng_r_close(&hdl, sync);
}

//3. playback flow
void record_playback_demo(void)
{
    PVR_HANDLE hdl;
    UINT16 rl_idx;
    PVR_STATE state;
    UINT8 speed, vpo_onoff_flag, resume_play_flag;
    BOOL sync = TRUE;
    UINT32 play_time, search_time;
    INT32 jump_time;
    UINT32 start_time;
    struct playback_param ply_param;
    MEMSET(ply_param, 0, sizeof(struct playback_param));
    ply_param.path[1024];        // playback reccord item
    ply_param.state = NV_PLAY;        // playback state
    ply_param.start_time = 0;      // start time
    ply_param.start_pos = 0;// bit22-bit31: file index,bit0-bit21:vobu index, only when start_mode is P_OPEN_FROM_POS.
    ply_param.speed = 1;            // playback speed
    ply_param.start_mode = P_OPEN_DEFAULT;       // open mode: enum p_open_config_t
    ply_param.dmx_id = 2;           // playback dmx id
    ply_param.live_dmx_id = 0;      // live play dmx id
    hdl = pvr_eng_p_open(&ply_param);
    pvr_p_pause(hdl);
    pvr_p_play(hdl);
    speed = pvr_p_get_speed(hdl);
    play_time = pvr_p_get_time(hdl);
    pvr_p_revslow(hdl, speed);
    pvr_p_slow(hdl, speed);
    pvr_p_pause(hdl);
    pvr_p_step(hdl);
    pvr_p_fast_backward(hdl, speed);
    pvr_p_fast_forward(hdl, speed);
    search_time = 0;
    pvr_eng_p_timesearch(hdl, search_time);
    jump_time = -20; //seconds
    pvr_eng_p_jump(hdl, jump_time);
    pvr_p_stop(hdl);
    resume_play_flag = 1;
    struct playback_stop_param stop_param;
    stop_param.stop_mode = P_STOPPED_ONLY;
    stop_param.vpo_mode = 0; // for tv program.
    stop_param.sync = TRUE;
    pvr_eng_p_close(&hdl, &stop_param);
}

#endif
#endif //__LIB_PVR_ENG_H__
