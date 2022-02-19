/****************************************************************************
*
*  ALi (Shanghai) Corporation, All Rights Reserved. 2003 Copyright (C)
*
*  File: db_3l.h
*
*  Description: The head file of program database
*
*  History:
*      Date        Author         Version   Comment
*      ====        ======         =======   =======
*  1.  2004.07.04  David Wang     0.1.000   Initial
*  2.  2005.03.24  Zhengdao Li    0.1.001   Add marco for SERVICE_PROVIDER name option field.
*  3.  2005.04.21  Zhengdao Li    0.1.002   Add support for program sort by dl-frequency by default.
*  4.  2006.01.18  Joe Zhou   0.1.003   Modify P_NODE for 2M -T solution.
*  5.  2006.02.20  Zhengdao Li    0.1.004   Modify according to Wing Chen.
****************************************************************************/
#ifndef __LIB_DB_3L_T_H__
#define __LIB_DB_3L_T_H__

#include <types.h>
#include <sys_config.h>


//#if( SYS_PROJECT_FE == PROJECT_FE_DVBT )
#define MAX_BAND_COUNT                  10
#define MAX_COUNTRY                     20
#define MAX_BAND_PARAMETERS             4
#define HIERARCHY

typedef struct band_param
{
    UINT32 start_freq;
    UINT32 end_freq;
    UINT32 bandwidth;
    UINT32 band_type;
}band_param;

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2)
#else
band_param u_country_param[MAX_BAND_COUNT];
#endif

///////////////////////////////////////////

#ifndef SERVICE_PROVIDER_NAME_OPTION
#define SERVICE_PROVIDER_NAME_OPTION    0
#endif

#if 0
#ifndef _DB3L_DEBUG_
#define _DB3L_DEBUG_
#endif
#endif

#define DB_DEBUG_OPTION            0
#if (DB_DEBUG_OPTION>0)
#define DB_PRINTF       libc_printf
#define DVBT_PRINTF     libc_printf
#else
#define DB_PRINTF(...)      do{}while(0)
#define DVBT_PRINTF(...)        do{}while(0)
#endif


#define TYPE_PROG_NODE              0
#define TYPE_TP_NODE                1
#define TYPE_SAT_NODE               2
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2)
#define TYPE_SYS_NODE               3
#ifdef GROUP_NAME_MANAGER_SUPPORT
#define TYPE_GROUP_NODE             4
#define MAX_TABLE_NUM               5
#else
#define TYPE_CI_DATA_NODE           4
#define MAX_TABLE_NUM               4
#endif

#else
#define TYPE_CI_DATA_NODE           3
#define MAX_TABLE_NUM               3
#endif

#ifdef SM_OTA
#define INVALID_SAT                 0xff
#endif

#define MAX_SERVICE_NAME_LENGTH     40//17 //new or change add by yuj
#define MAX_SATELLITE_NAME_LENGTH   17

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2)
#define MAX_AUDIO_CNT               10
#else
#define MAX_AUDIO_CNT               10//EXPANDED_MPEG_CNT // hbchen 2006 05 03
#define MAX_AC3_CNT                 10//         EXPANDED_AC3_CNT
#endif
//#define MAX_AUDIO_CNT             3
#define DEFAULT_MAX_AUDIO_CNT       10//3

#define DB_VER_NUM                  1
#define DEFAULT_VIEW_RULE           NULL

#define INVALID_POS_NUM             0xFFFF
#define DB_LCN_INVALID              0xFFFF



#define PROG_DEFAULT_VIEW_ID        "PRGV"
#define PROG_DEFAULT_TABLE_ID       "PT01"

#define TP_DEFAULT_VIEW_ID          "TPDV"
#define TP_DEFAULT_TABLE_ID         "TT01"

#define SAT_DEFAULT_VIEW_ID         "SATV"
#define SAT_DEFAULT_TABLE_ID        "ST01"

#define CI_DEFAULT_VIEW_ID          "CIDV"
#define CI_DEFAULT_TABLE_ID         "CI01"


/* define return value */
#define NODE_POOL_FULL              0x0FF00001
#define STORE_SPACE_FULL            0x0FF00002
#define BACKUP_DATA_FAIL            0x0FF00003
#define UPDATE_DATA_FAIL            0x0FF00004
#define NODE_ALREADY_EXIST          0x0FF00005

#define PROG_VIDEO_MODE             0x01
#define PROG_AUDIO_MODE             0x00

#if 0
/* create program view flag */
#define CREATE_VIEW_SINGLE_SAT      0x01
#define CREATE_VIEW_ALL_SAT         0x02
#define CREATE_VIEW_FAV_GROUP       0x03
#define CREATE_VIEW_SINGLE_CH       0x04
#define CREATE_VIEW_ALL_CH          0x05
#define CREATE_VIEW_PSEARCH         0x06
#else
/* create program view flag */
enum
{
    CREATE_VIEW_SINGLE_SAT          = 0x01,
    CREATE_VIEW_ALL_SAT             = 0x02,
    CREATE_VIEW_FAV_GROUP           = 0x03,
    CREATE_VIEW_SINGLE_CH           = 0x04,
    CREATE_VIEW_ALL_CH              = 0x05,
    CREATE_VIEW_PSEARCH             = 0x06, /* single sallete, all program */
    CREATE_VIEW_ALL_SAT_AV          = 0x07,
    CREATE_VIEW_SINGLE_SAT_AV       = 0x08,
    CREATE_VIEW_FAV_GROUP_AV        = 0x09,
};
#endif
/* for cal program num */
#define SAT_PROG_NUM                0x01
#define FAV_PROG_NUM                0x02
#define CH_PROG_NUM                 0x04

/* flag for sort program */
enum
{
    PROG_LOCK_SORT                  = 0x01,
    PROG_LOCK_SORT_EXT              = 0x02,

    PROG_FTA_SORT                   = 0x03,
    PROG_FTA_SORT_EXT               = 0x04,

    PROG_NAME_SORT                  = 0x05,
    PROG_NAME_SORT_EXT              = 0x06,

    PROG_DEFAULT_SORT               = 0x07,
    PROG_DEFAULT_SORT_EXT           = 0x08,

    PROG_LOGICAL_NUM_SORT           = 0x09,

    PROG_TPLIST_SORT                = 0x0a,
    PROG_TPLIST_SORT_EXT            = 0x0b,

    PORG_DEFAULT_SORT_THEN_FTA      = 0x0c,
    PROG_NUMBER_SORT                = 0x0d,

    PROG_LCN_SORT                   = 0x0e,

};


enum
{
    TP_DEFAULT_SORT                 = 0x15,
    TP_DEFAULT_SORT_EXT             = 0x16,
};

enum
{
    DB3L_IOCTL_SELECT_PNODE_BUFFER  = 0xDB300001,
};

enum
{
    DB3L_USE_STATIC_ARRAY           = 0x00000001,
    DB3L_USE_FRAME_BUFFER           = 0x00000002,
};

#define PROG_NAME_SORT_A_Z          0x00
#define PROG_NAME_SORT_Z_A          0x01

/* audio channel */
#ifdef  DEFAULT_AUDIO_CH_STEREO

#define AUDIO_CH_STEREO             0x00
#define AUDIO_CH_L                  0x01
#define AUDIO_CH_R                  0x02
#define AUDIO_CH_MONO               0x03
#else
#define AUDIO_CH_L                  0x00
#define AUDIO_CH_R                  0x01
#define AUDIO_CH_STEREO             0x02
#define AUDIO_CH_MONO               0x03
#endif

#define AUDIO_DEFAULT_VOLUME        13
#define DEFAULT_DOMESTIC_PROG       0x01
#define DEFAULT_ALL_PROG            0x02
#define DEFAULT_SAT_TP_ONLY         0x03

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2)
#define ERR_EXIST                   0x000e000e
#endif
#define PRE_SET_TP_FLAG             0x01

#define PROG_ADD_REPLACE_OLD        0x00
#define PROG_ADD_DIRECTLY           0x01
#define PROG_ADD_PRESET_REPLACE_NEW 0x02
#define PROG_ADD_REPLACE_NONPRESET  0x03

#define SAT_DEL_ALL                 0x01
#define SAT_DEL_CHILDREN            0x02

#define DB_ENTER_MUTEX()            osal_semaphore_capture(db_access_sema,OSAL_WAIT_FOREVER_TIME)
#define DB_RELEASE_MUTEX()          osal_semaphore_release(db_access_sema)


#ifndef DB_USE_UNICODE_STRING
#define DB_DEFAULT_NAME             "No Name"
#define DB_DEFAULT_PROV_NAME        "Unknown PP"
#define DB_DEFAULT_NAME_LEN         8
#define DB_DEFAULT_PROV_LEN         11

#define DB_STRCMP                   STRCMP
#define DB_STRCPY                   STRCPY
#define DB_STRLEN                   STRLEN
typedef UINT8 DB_ETYPE;
#else
typedef UINT16 DB_ETYPE;
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE && SYS_SDRAM_SIZE == 2)
#define DB_DEFAULT_NAME             "N\0o\0 \0N\0a\0m\0e\0\0"
#define DB_DEFAULT_PROV_NAME        "U\0n\0k\0n\0o\0w\0n\0 \0P\0P\0\0"
#else
#define DB_DEFAULT_NAME             "\0N\0o\0 \0N\0a\0m\0e\0"
#define DB_DEFAULT_PROV_NAME        "\0U\0n\0k\0n\0o\0w\0n\0 \0P\0P\0"
#endif
#define DB_DEFAULT_NAME_LEN         16
#define DB_DEFAULT_PROV_LEN         22


INT32 DB_STRCMP(UINT16 *s, UINT16 *t);
UINT16 *DB_STRCPY(UINT16 *dest, UINT16 *src);
INT32 DB_STRLEN(UINT16 *s);
#endif /* DB_USE_UNICODE_STRING */

/*
 * name     : LNB_CLASS_xxxx <enum>
 * description  : the LNB class enums.
 */
enum
{
    LNB_CLASS_PREDEFINED            = 0x00, /* predefined parameter     */
    LNB_CLASS_USERDEFINE            = 0x40  /* user adjustable parameter    */
};


union c_param
{
    struct
    {
        UINT16 cmd;
        UINT16 tag;
    } decode;
    UINT32 encode;
};


struct t_info
{
    UINT32 value;
    UINT16 id;
    UINT16 next;
};

struct p_info
{
    UINT16 rec_idx;
    UINT16 next;
};



/*
 * name     : LNB_CTRL_xxxx <enum>
 * description  : LNB control method enums.
 */
enum
{
    LNB_CTRL_STD                = 0x00, /* standard non dual-LNB    */
    LNB_CTRL_POL                = 0x10, /* dual LNB controlled by polar */
    LNB_CTRL_22K                = 0x20, /* dual LNB controlled by 22k   */
    LNB_CTRL_RESERVED           = 0x30  /* reserved control method  */
};

typedef struct prog_node
{
    /*0 4B */
    UINT32 tp_id                :16;/* MASK: 0xFFFF0000 */
    UINT32 video_pid            :13;
    UINT32 av_flag              : 1;
    UINT32 level                : 2;
    /*1 4B*/
    UINT32 prog_number          :16;
    UINT32 service_type         : 8;
    UINT32   lcn_true           :1;
    UINT32  ttxsub_flag         :1;
    UINT32 audio_channel        : 2;
    UINT32  cur_audio           : 4;    //reserved but redundant for DVB-T, SN // 2->4 by hbchen

    UINT8 fav_byte[0];
    /*2 4B*/
    UINT32 fav_grp0             : 1;
    UINT32 fav_grp1             : 1;
    UINT32 fav_grp2             : 1;
    UINT32 fav_grp3             : 1;
    UINT32 fav_grp4             : 1;
    UINT32 fav_grp5             : 1;
    UINT32 fav_grp6             : 1;
    UINT32 fav_grp7             : 1;
    UINT32 pmt_pid              :13;
    UINT32 pmt_version          : 3;
    UINT32 user_modified_flag   : 1;
    UINT32 preset_flag          : 2;
    UINT32 audio_volume         : 5;

    /*3 4B*/
    UINT32 pcr_pid              :13;
    UINT32 subtitle_pid         :13;
#ifdef H264_SUPPORT
    UINT32 audio_count          : 4;
    UINT32 h264_flag            : 1;
#else
    UINT32 audio_count          : 5;
#endif
    UINT32 lock_flag            : 1;

    /*4 4B*/
    UINT32 sat_id               :16;
    UINT32 teletext_pid         :13;
    UINT32 skip_flag            : 1;
    UINT32 ca_mode              : 1;
    UINT32 audio_select         : 1;

    UINT16 audio_pid[MAX_AUDIO_CNT];
    UINT16 audio_lang[MAX_AUDIO_CNT];

    UINT8 service_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];
#if (SERVICE_PROVIDER_NAME_OPTION>0)
    UINT8 service_provider_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];
#endif
    UINT32  audio_count_dolby   : 2;
    UINT32  cur_audio_dolby     : 3; //combine cur_audio as an only one flag, SN for DVB-T
    UINT32  mheg5_exist         : 1; // 3-> 1 by hbchen // fan: it's just useful when _MHEG5_SUPPORT_
    UINT32  current_index       :16; //like default_index and LCN, record the current moved or unmoved index
//  UINT32  true_lcn            : 1; // 1: the LCN value is from bitstream, 0: the LCN value is given by default_index
    UINT32  reserved_1          : 7;
    UINT32   eit_pf_flag        : 1;
    UINT32 provider_lock        : 1;
    UINT32  eit_sch_flag        : 1;

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE >= 8)
    UINT16  audio_pid_dolby[MAX_AC3_CNT];
    UINT16  audio_lang_dolby[MAX_AC3_CNT];//the 80th byte or the 120th byte
#endif
    UINT32  default_index       :16;//for returning to default sorting
    UINT32  LCN                 :16;//for LCN
#ifdef  DTTM_DB
    UINT8   dttm_sert_next_state   : 8;//0:new, 1:current, 2:delete
    UINT8   dttm_slt_status       : 8;//0:available, 1:favourite, 2:hidden
    UINT8   dttm_sert_state       : 8;//0:new, 4:current
    UINT8   reserved_2          : 8;
    UINT8  audio_type[MAX_AUDIO_CNT];
    UINT16  reserved_3;

#endif
}P_NODE;

typedef struct default_prog_node
{
    /*0 4B */
    UINT32 tp_id                :16;/* MASK: 0xFFFF0000 */
    UINT32 video_pid            :13;
    UINT32 av_flag              : 1;
    UINT32 level                : 2;
    /*1 4B*/
    UINT32 prog_number          :16;
    UINT32 service_type         : 8;
    UINT32 audio_count          : 2;
    UINT32 audio_channel        : 2;
    UINT32 eit_sch_flag         : 1;
    UINT32 cur_audio            : 2;
    UINT32 provider_lock        : 1;

    /*2 4B*/
    UINT8 fav_byte[0];
    UINT32 fav_grp0             : 1;
    UINT32 fav_grp1             : 1;
    UINT32 fav_grp2             : 1;
    UINT32 fav_grp3             : 1;
    UINT32 fav_grp4             : 1;
    UINT32 fav_grp5             : 1;
    UINT32 fav_grp6             : 1;
    UINT32 fav_grp7             : 1;
    UINT32 ci_data_id           :16;    /* each bit indicate a CA System */
    UINT32 lock_flag            : 1;
    UINT32 preset_flag          : 2;
    UINT32 audio_volume         : 5;
    /*3 4B*/
    UINT32 pcr_pid              :13;
    UINT32 subtitle_pid         :13;
    UINT32 pmt_version          : 5;    /* this value is not countable, since it will rewind */
    UINT32 eit_pf_flag          : 1;

    UINT32 sat_id               :16;
    UINT32 teletext_pid         :13;
    UINT32 skip_flag            : 1;
    UINT32 ca_mode              : 1;
    UINT32 audio_select         : 1;

    UINT16 audio_pid[DEFAULT_MAX_AUDIO_CNT];
    UINT16 audio_lang[DEFAULT_MAX_AUDIO_CNT];
    UINT8 service_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];
    UINT8 service_provider_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];
}P_DEFAULT_NODE;

typedef struct tp_node
{
    /* 0 4B */
    UINT32 tp_id                :16;
#ifdef DYNAMIC_ADD_PROG
#ifdef HIERARCHY
    UINT32  priority            : 2;
    UINT32  hier                : 2;

    UINT32  reserved            : 6;
#else
    UINT32  reserved            :10;
#endif
    UINT32    del_prog_flag     : 1;
#else
#ifdef HIERARCHY
    UINT32  priority            : 2;
    UINT32  hier                : 2;
    UINT32  reserved            : 7;
#else
    UINT32  reserved            :11;
#endif
#endif
    UINT32  inverse             : 1;    /*used for channel change*/
    UINT32  band_type           : 1;    /*0: VHF, 1: UHF, for DVB-T only*/

    UINT32 preset_flag          : 2;
    UINT32 big5_indicator       : 1; /* for some TAIWAN channels */

    /* 1 4B */
    UINT32 sat_id               :16;
    UINT32 usage_status         : 1;
    UINT32 nit_pid              :13;
    UINT32 pol                  : 2;

    /* 2 4B */
    UINT32 frq                  :24;    //Added for freq offset
    INT8 offset_step            : 8;

    /* 3 4B */
    UINT32 sym                  :28;
    UINT32 fec_inner            : 4;

    /* 4 4B */
    UINT32 t_s_id               :16;
    UINT32 network_id           :16;


    /* 5 4B */
    UINT32  guard_interval      : 8;     /*0x4: 1/4, 0x8: 1/8, 0x10: 1/16, 0x20:1/32*/
    UINT32  FFT                 : 8; /*0x2 : 2k, 0x8 : 8k*/
    UINT32  modulation          : 8; /*0x4 : QPSK, 0x10 : 16 QAM, 0x40 : 64 QAM*/
    UINT32  bandwidth           : 8; /**/

#ifdef _MHEG5_SUPPORT_
    /* 6 4B */
    UINT32 net_id               :16;
    UINT32  reserved2           :16;
#endif

}T_NODE;

typedef struct sat_node
{
    /* 0 4B */
    UINT32 sat_id               :16;
    UINT32 k22                  : 1;
    UINT32 di_seq_c_type          : 3;/* 1/4,1/8,1/16 */
    UINT32 di_seq_c_port          : 4;
    UINT32 selected_flag        : 1;
    UINT32 v12                  : 1;
    UINT32  reserved_1          : 6;


    /* 1 4B */
    UINT32  lnb_power           : 1;

    /*0: not DiSEqC LNB
      1:Universal Local freq DiSEqC LNB 
      2:DiSEqC LNB Hi local freq
      3:DiSEqC LNB lo local freq*/
    UINT32  lnb_type            : 7;
    UINT32  lnb_high            :16;
    UINT32 di_seq_c11_type        : 3;/* 1/4,1/8,1/16 */
    UINT32 di_seq_c11_port        : 4;
    UINT32  reserved_2          : 1;

    /* 2 4B */
    UINT32  positioner_type     : 3;    /*0 no positioner 1 positioner support*/
    UINT32  pol                 : 2;    /* 0: auto,1: H,2: V */
    UINT32  toneburst           : 3;    /* 0: off, 1: A, 2: B */
    UINT32  lnb_low             :16;
    UINT32  reserved_3          : 8;

    /* 3 4B */
    UINT32  position            : 8;
    UINT32  sat_orbit           :24;
    UINT8 sat_name[2*(MAX_SATELLITE_NAME_LENGTH+1)];

}S_NODE;

#ifdef GROUP_NAME_MANAGER_SUPPORT
#define MAX_GROUP_NAME_LEN      16
#define MAX_GROUP_SUPPORT       8

typedef struct group_node
{
    UINT16 group_name[MAX_GROUP_NAME_LEN];
}G_NODE;


#endif

#define MAX_CI_DATA_LEN         128
typedef struct ci_node
{
    UINT32 ci_data_id           :16;
    UINT32 reserved             :16;
    UINT8  ci_data_buff[MAX_CI_DATA_LEN];
}CI_NODE;

struct feild_map
{
    INT8 f_name[32];
    INT8 f_mask[32];
};

#if (SERVICE_PROVIDER_NAME_OPTION>0)
/* struct for provider lock function */
typedef struct provider_info
{
    UINT8 lock_flag;
    UINT16 audio_cnt;
    UINT16 video_cnt;
    UINT8 service_provider_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];
    struct provider_info *next;
}PROV_INFO;
#endif

enum
{
    DB_EVT_UPDATE_START         = 0x01,
    DB_EVT_UPDATE_END           = 0x02,
};

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*backup_buff_callback)(void);
typedef INT32 (*update_data_callback)(void);

typedef INT32 (*db_event_t)(UINT32 type, UINT32 param);

INT32 _init_db();
INT32 init_db();
INT32 _clear_db();
INT32 clear_db();

INT32 add_node(UINT8 n_type,void *node);
void set_prog_del_flag(UINT16 pos,UINT8 flag);
UINT8 get_prog_del_flag(UINT16 pos);

INT32 del_prog_at(UINT16 pos);
INT32 del_prog_by_tp(UINT32 tp_id, UINT8 prog_attr);
INT32 del_tp_by_id(UINT16 tp_id);
INT32 del_sat_by_id(UINT16 sat_id);
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2)
INT32 del_sat_at(UINT16 pos);
#else
INT32 del_sat_at(UINT16 pos,UINT8 del_flag);
#endif
/**
 * name     : clear_unuse_tp
 * description  : delete all tp which does not associated any other program.this function have
                   O(TP number* program number) time complexity,if there are many tp and program,call this fucntion
                   should cost a lots time,avoid to call this function in this situation.
 * parameter    : none
 * return value :INT32 if SUCCESS this function complete successful.otherwise, return the reason of fail.
 */
UINT32 clear_unuse_tp(void);
/**
 * name     : del_prog_with_tp_at
 * description  : this function will delete the program node at pos,at the same time,
                  it will delete the program'tp if NO any other program associated .
                  this function have O(TP number + program number) time complexity,you can call this
                  function like del_prog_at
 * parameter    : 1
 *  IN  UINT16 pos:the post of the program should delete.
 * return value :INT32 if SUCCESS this function complete successful.otherwise, return the reason of fail.
 */
INT32 del_prog_with_tp_at(UINT16 pos);
/**
 * name     : discard_data_changed
 * description  : discard database chaanged,such as delete,modify,add.
 * parameter    : 2
 *  IN  UINT8 n_type:   database table type.
 *  IN  UINT8 reason:   discard which type of changed,delete,modify or add.
 * return value : void
 */
void discard_data_changed(UINT8 n_type,UINT8 reason);


INT32 _modify_prog(UINT16 pos,P_NODE *node);
INT32 modify_prog(UINT16 pos,P_NODE *node);
INT32 modify_tp(UINT16 tp_id,T_NODE *node);
INT32 modify_sat(UINT16 sat_id,S_NODE *node);

INT32 _get_prog_at(UINT16 pos,P_NODE *node);
INT32 get_prog_at(UINT16 pos,P_NODE *node);
UINT16 _get_prog_pos(P_NODE *p_node);
UINT16 get_prog_pos(P_NODE *p_node);
INT32 get_find_prog_pos(P_NODE *p_node);

INT32 get_tp_at(UINT16 sat_id,UINT16 pos,T_NODE *node);
INT32 _get_sat_at(UINT16 pos,S_NODE *node);
INT32 get_sat_at(UINT16 pos,S_NODE *node);
INT32 get_tp_by_id(UINT16 tp_id,T_NODE *node);
INT32 get_sat_by_id(UINT16 sat_id,S_NODE *node);

UINT16 get_prog_num(UINT8 av_flag);
UINT32 _get_node_num(UINT8 n_type,INT8 *reserved);
UINT32 get_node_num(UINT8 n_type,INT8 *rule);

UINT32 get_selected_sat_num();

/* for DB3 only */
UINT32 get_selected_sat_list(UINT16 *ids);

INT32 get_selected_sat(UINT32 pos,S_NODE *s_node);
UINT32 get_tp_num_sat(UINT16 sat_id);
INT32 get_prog_num_sat(UINT16 sat_pos, UINT16 *v_cnt,UINT16 *a_cnt);
UINT16 get_prog_num_fav(UINT8 fav_group, UINT8 av_flag);
INT32 _get_specific_prog_num(UINT8 prog_flag,UINT16 *sat_v_cnt,UINT16 *sat_a_cnt,UINT16 *fav_v_cnt,UINT16 *fav_a_cnt);
INT32 get_specific_prog_num(UINT8 prog_flag,UINT16 *sat_v_cnt,UINT16 *sat_a_cnt,UINT16 *fav_v_cnt,UINT16 *fav_a_cnt);

//INT32 get_1st_node(UINT8 n_type,INT8 *rule,UINT16 r_len,void *node,UINT32 node_len);
//INT32 get_next_node(UINT8 n_type,INT8 *rule,UINT16 r_len,void *node,UINT32 node_len);
//INT32 get_cur_node(UINT8 n_type,void *node,UINT32 node_len);

INT8 get_av_mode();
void _set_av_mode(INT8 av_mode);
void set_av_mode(INT8 av_mode);

INT32 alloc_provider_info();
INT32 free_provider_info();
INT32 get_provider_at(UINT16 p_pos,UINT16 *tv_cnt,UINT16 *radio_cnt,UINT16 *l_flag,INT8 *p_name);
INT32 set_provider_at(UINT16 p_pos,UINT16 *l_flag);
UINT16 get_provider_num();
UINT16 get_provider_pos(UINT16 prog_pos);


UINT16 check_node_modified(UINT8 n_type);
INT32 move_prog(UINT16 d_pos,UINT16 s_pos);
INT32 node_restore(UINT8 n_type);

void sort_prog_node(UINT8 sort_flag);
INT32 _set_bkbuffer_single_sector(UINT8 *bk_buff,UINT32 buff_len);

INT32 _check_storage_space(UINT8 n_type);
INT32 check_storage_space(UINT8 n_type);


/**
 * name     : sort_tp_node
 * description  : sorting on the tranponder table.
 * parameter    : 2
 *  IN  UINT8 sort_flag:    the sort method
 *  IN  INT32 parameter:    reserved for future use.
 * return value : void
 */
void sort_tp_node(UINT8 sort_flag, INT32 parameter);

INT32 recreate_prog_view(INT8 create_flag,UINT16 id);

INT32 _tmp_buff_node(UINT8 n_type,void *node);
INT32 cal_expr_val(UINT8 n_type,INT8 *expr,UINT16 pos);
INT32 _update_data(UINT8 n_type);
INT32 update_data(UINT8 n_type);
void db_reg_callback(backup_buff_callback callback);
INT32 set_default_value(UINT8 set_mode);
INT32 db_reg_update_event(db_event_t callback);
INT32 update_stream_info(P_NODE *prev, P_NODE *next, INT32 *pos);
#if 0
INT32 db_register_update_callback(update_data_callback callback);
INT32 db_unregister_update_callback();
#endif


#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2)
band_param *get_default_bandparam(UINT8 index);
#else
void get_default_bandparam(UINT8 index, band_param *buf);
#endif

/*
 * name     : get_LNB_class
 * description  : return the LNB class belonging.
 * parameter    : 1
 *  IN  UINT32 type
 * return value : UINT32
 *  LNB_CLASS_PREDEFINED:   the LNB belong to predefined class.
 *  LNB_CLASS_USERDEFINE:   the LNB belong to user defined class.
 */
static inline UINT32 get_lnb_class(UINT32 type)
{
    return (type&LNB_CLASS_USERDEFINE);
}

/*
 * name     : get_LNB_ctrl
 * description  : return the LNB controlling method
 * parameter    : 1
 *  IN  UINT32 type
 * return value : UINT32
 *  LNB_CTRL_STD:   it is not a dual-LNB.
 *  LNB_CTRL_POL:   the LNB is selected by polarity.
 *  LNB_CTRL_22K:   the LNB is selected by 22k signal.
 *  LNB_CTRL_RESERVED:  reserved.
 */
static inline UINT32 get_lnb_ctrl(UINT32 type)
{
    return (type&LNB_CTRL_RESERVED);
}

/*
 * name     : get_LNB_index
 * description  : return the LNB index inside the class and control type.
 * parameter    : 1
 *  IN  UINT32 type
 * return value : UINT32
 *  0~15    : the LNB index.
 */
static inline UINT32 get_lnb_index(UINT32 type)
{
    return (type&0x0F);
}

#ifdef __cplusplus
}
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && SYS_SDRAM_SIZE == 2)
void load_favor_info(UINT8 *buff, UINT32 buff_size);
void write_favor_info(UINT8 *buff);
INT32 recreate_view_favored_prog(INT32 av_mode);
UINT16 get_prog_total(void);
#endif

void find_max_default_index();//SN 20050712 for default index
void find_min_invalid_lcn_num();//051128-cmchen
void tp_onid_sort(void);
void prog_sid_sort(void);

#if (SYS_SDRAM_SIZE>2)
INT32 db3l_ioctl(UINT32 cmd, UINT32 parameter);
#endif
#if (SYS_PROJECT_FE != PROJECT_FE_DVBT || SYS_SDRAM_SIZE != 2)

UINT16 get_node_during_scan(UINT16 pos,P_NODE *p_node);
#endif

#endif /* __LIB_DB_3L_T_H__ */

