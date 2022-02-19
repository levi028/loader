/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
      disclosed to unauthorized individual.
*    File: db_node_combo.h
*
*    Description: declare the structure of node and functions interface on
      combo solution, like node packer, unpacker, compare etc...
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _DB_NODE_COMBO_H_
#define _DB_NODE_COMBO_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include <api/libsi/si_config.h>
#ifdef DB_SUPPORT_HMAC
#include <api/librsa/rsa_verify.h>
#endif

#define S_NODE_FIX_LEN          24
#ifdef PLSN_SUPPORT
#define T_NODE_FIX_LEN          40
#else
#define T_NODE_FIX_LEN          36
#endif
#define P_NODE_FIX_LEN          48//44

//add by wenhao
#define DB_MAX_OPTIMIZE
#define COMBO_P_NODE_MAX_FIX_LEN    36
#define COMBO_P_NODE_MIN_FIX_LEN    16
#define LCN_ENABLE_FLAG_OFFSET        4+COMBO_P_NODE_MIN_FIX_LEN
#define SERVICE_ATTRIBUTE_ENABLE_FLAG_OFFSET LCN_ENABLE_FLAG_OFFSET+8
#define User_ORDER_SORT_FLAG_OFFSET SERVICE_ATTRIBUTE_ENABLE_FLAG_OFFSET+4
#define FIX_POS_FLAG_OFFSET User_ORDER_SORT_FLAG_OFFSET+4
#define SUPPORT_FRANCE_HD_FLAG_OFFSET 4+COMBO_P_NODE_MAX_FIX_LEN
#ifdef DB_MAX_OPTIMIZE
    #define COMBO_P_NODE_FIX_LEN    COMBO_P_NODE_MIN_FIX_LEN
#else
    #define COMBO_P_NODE_FIX_LEN    COMBO_P_NODE_MAX_FIX_LEN
#endif
//add end



#define MAX_CAS_CNT             4
#define MAX_AUDIO_CNT           20
#define MAX_SERVICE_NAME_LENGTH 17
#define MAX_SATELLITE_NAME_LENGTH 17
#define MAX_BAND_COUNT          10

#define AUDIO_DEFAULT_VOLUME    40
#define AUDIO_CH_L              0x00
#define AUDIO_CH_R              0x01
#define AUDIO_CH_STEREO         0x02
#define AUDIO_CH_MONO           0x03

#define INVALID_POS_NUM         0xFFFF
#define INVALID_POLAR   		0xFF

#define FRQ_EDG 2
#define SYM_EDG 19

typedef struct band_param
{
    UINT32  start_freq;
    UINT32  end_freq;
    UINT32  bandwidth;
    UINT32  band_type;

    //for dvbc
    UINT32  freq_step;
    UINT8   start_ch_no;
    UINT8   end_ch_no;
    UINT8   show_ch_no;
    UINT8   show_ch_prefix;
}band_param;

enum
{
    LNB_CTRL_STD                = 0x00, /* standard non dual-LNB    */
    LNB_CTRL_POL                = 0x10, /* dual LNB controlled by polar */
    LNB_CTRL_22K                = 0x20, /* dual LNB controlled by 22k   */
    LNB_CTRL_UNICABLE           = 0x30, /* dual LNB controlled from unicable */
    LNB_CTRL_RESERVED           = 0x70  /* reserved control method  */
};

typedef struct antena_t
{
    UINT8 lnb_power         : 1;
    UINT8 lnb_type          : 7;

    UINT8 pol               : 2;
    UINT8 k22               : 1;
    UINT8 v12               : 1;
    UINT8 toneburst         : 2;
    UINT8 unicable_pos      : 1;
    UINT8 reserve_1         : 1;            //2

    UINT16 lnb_low;
    UINT16 lnb_high;                    //4

    UINT8 di_seq_c_type       : 4;
    UINT8 di_seq_c_port       : 4;

    UINT8 di_seq_c11_type     : 4;
    UINT8 di_seq_c11_port     : 4;

    UINT16 positioner_type  : 3;
    UINT16 position         : 8;
    UINT16 reserve_2        : 5;        //4
}ANTENA;

typedef struct sat
{
    UINT16 sat_id;
    UINT16 reserve_id;              //4

    UINT16 sat_orbit;               //2

    UINT16 selected_flag    : 1;
    UINT16 tuner1_valid     : 1;
    UINT16 tuner2_valid     : 1;
    UINT16 sat2ip_flag      : 1;     //2
    UINT16 reserve_1        :12;     //2

    //tuner1 antenna setting*****
    UINT8 lnb_power         : 1;
    UINT8 lnb_type          : 7;

    UINT8 pol               : 2;
    UINT8 k22               : 1;
    UINT8 v12               : 1;
    UINT8 toneburst         : 2;
    UINT8 unicable_pos      : 1;
    UINT8 reserve_2         : 1;       //2

    UINT16 lnb_low;
    UINT16 lnb_high;                    //4

    UINT8 di_seq_c_type       : 4;
    UINT8 di_seq_c_port       : 4;

    UINT8 di_seq_c11_type     : 4;
    UINT8 di_seq_c11_port     : 4;

    UINT16 positioner_type  : 3;
    UINT16 position         : 8;
    UINT16 reserve_3        : 5;        //4
    //*************************

    ANTENA tuner2_antena;               //10

    UINT16 name_len;
    UINT8 sat_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];

    UINT16 reference_chan1;
    UINT16 reference_chan2;

#ifdef DB_SUPPORT_HMAC
    UINT8 v_hmac[HMAC_OUT_LENGTH];
#endif

}S_NODE;

typedef struct tp
{
    UINT16 sat_id;
    DB_TP_ID tp_id;

    UINT32 frq;
    UINT32 sym;

    UINT16 pol                  : 2;
    UINT16 fec_inner            : 4;  //T: 0:1/2, 1:2/3, 2:3/4, 3:5/6, 4:7/8  //T2: 5:3/5, 6:4/5 //0xF:unknown
    UINT16 universal_22k_option : 1;
    UINT16 big5_indicator       : 1;
    UINT16 ft_type              : 2;  //front end type,check lib_frontend.h
    UINT16 inverse              : 1;
    UINT16 band_type            : 1;
    UINT16 priority             : 2;  //for DVB-T HIERARCHY mode
    UINT16 hier                 : 2;

    UINT16 preset_flag          : 2;
    UINT16 usage_status         : 1;
    UINT16 nit_pid              :13;

    UINT32 t_s_id               :16;
    UINT32 network_id           :16;  //original_network_id

    UINT32 net_id               :16;  //network_id
    UINT32 t2_signal            : 1;  //0:DVB-T signal, 1:DVB-T2 signal. (when ft_type==FRONTEND_TYPE_T)
    UINT32 plp_index            : 8;
    UINT32 t2_profile			: 2;
    UINT32 reserved             : 5;

    UINT32 sdt_version          : 8;
    UINT32 t2_system_id         :16;
    UINT32 plp_id               : 8;

    //T2: 128:1/128, (19+128):19/128, 19:19/256, //0xFF:unknown
    UINT32 guard_interval       : 8;    //4: 1/4, 8: 1/8, 16: 1/16, 32:1/32
    UINT32 FFT                  : 8;    //2:2k, 8:8k //T2: 1:1k, 4:4k, 16:16k, 32:32k, //0xFF:unknown

     //T2: (64+1):256 QAM, //0xFF:unknown
    UINT32 modulation           : 8;    //2:DQPSK 4:QPSK, 16:16 QAM, 64:64 QAM
    UINT32 bandwidth            : 8;

    //SUPPORT_TP_QUALITY
    UINT32 intensity            :16;
    UINT32 quality              : 8;
    #ifdef AUTO_UPDATE_TPINFO_SUPPORT
    UINT32 crc_h8               : 8;
    #else
    UINT32 reserved4            : 8;
    #endif

    UINT32 remote_control_key_id: 8;
    #ifdef AUTO_UPDATE_TPINFO_SUPPORT
    UINT32 crc_t24              :24;
    #else
    UINT32 reserved5            :24;
    #endif
#ifdef PLSN_SUPPORT
    UINT32 pls_num;
#endif
#ifdef DB_SUPPORT_HMAC
    UINT8 v_hmac[HMAC_OUT_LENGTH];
#endif    
}T_NODE;

typedef struct program
{
    UINT16 sat_id;                          //2
    DB_TP_ID tp_id;                         //4

    UINT32 prog_id;                         //4

    //prog feature
    UINT32 level                : 2;
    UINT32 preset_flag          : 2;
    UINT32 av_flag              : 1;
    UINT32 ca_mode              : 1;
    UINT32 video_pid            :13;
    UINT32 pcr_pid              :13;        //4

    UINT32 prog_number          :16;
    UINT32 pmt_pid              :13;
    UINT32 tuner1_valid         : 1;
    UINT32 tuner2_valid         : 1;
    UINT32 h264_flag            : 1;        //4

    UINT32 fav_group[0];
    UINT8 fav_grp0              : 1;
    UINT8 fav_grp1              : 1;
    UINT8 fav_grp2              : 1;
    UINT8 fav_grp3              : 1;
    UINT8 fav_grp4              : 1;
    UINT8 fav_grp5              : 1;
    UINT8 fav_grp6              : 1;
    UINT8 fav_grp7              : 1;
    UINT8 fav_group_byte2;
    UINT8 fav_group_byte3;
    UINT8 fav_group_byte4;                  //4

    UINT16 pmt_version          : 5;
    UINT16 service_type         : 8;
    UINT16 audio_channel        : 2;
    UINT16 audio_select         : 1;        //2

    UINT16 user_modified_flag   : 1;
    UINT16 lock_flag            : 1;
    UINT16 skip_flag            : 1;
    UINT16 audio_volume         : 8;
    UINT16 mpeg4_flag           : 1;
    UINT16 shunning_protected   : 1;
	UINT16 video_type   		: 2; // 1 - H265
    UINT16 reserve_2            : 1;

//#if(defined( _MHEG5_SUPPORT_) ||defined(_LCN_ENABLE_) ||defined(_MHEG5_V20_ENABLE_))
    UINT32 mheg5_exist          : 1;
    UINT32 orig_lcn             :16;
    UINT32 reserve_3            :14;
    UINT32 lcn_true             : 1;
    UINT32 LCN                  :16;
    UINT32 default_index        :16; //for returning to default sorting
//#endif

//#ifdef _SERVICE_ATTRIBUTE_ENABLE_
    UINT32  num_sel_flag        : 1;
    UINT32  visible_flag        : 1;
    UINT32  reserved_4          : 30;       //4
//#endif

//#ifdef User_order_sort
    UINT32 user_order;                      //4
//#endif

    UINT32 provider_lock        : 1;
    UINT32 subtitle_pid         :13;
    UINT32 teletext_pid         :13;
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    UINT32 cur_audio            : 5;    /* any new fixed param shall added before cur_audio!!! */
                                            //4
//#ifdef SUPPORT_FRANCE_HD
    UINT32 hd_lcn_ture          : 1;
    UINT32 hd_lcn               :16;
    UINT32 reserve6             :15;        //4
//#endif
    UINT16 nvod_sid;
    UINT16 nvod_tpid;
    UINT16 bouquet_id;
    UINT16 logical_channel_num;

//#ifdef DB_CAS_SUPPORT
    UINT32 cas_count;                       //4
    UINT16 cas_sysid[MAX_CAS_CNT];          //8
//#endif

    UINT32 audio_count;                     //4
    UINT16 audio_pid[MAX_AUDIO_CNT];        //40
    UINT16 audio_lang[MAX_AUDIO_CNT];       //40
//#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
    UINT8 audio_type[MAX_AUDIO_CNT];        //20
//#endif
    UINT32 name_len;                        //4
    UINT8 service_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];            //36

    UINT32 provider_name_len;                                       //4
    UINT8 service_provider_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];   //36
#ifdef _INVW_JUICE

#if(defined(SUPPORT_NETWORK_NAME))
    UINT16 network_provider_name_len;
    UINT8 network_provider_name[2*(MAX_SERVICE_NAME_LENGTH + 1)];
#endif
#if(defined(SUPPORT_DEFAULT_AUTHORITY))
    UINT16 default_authority_len;
    UINT8 default_authority[2*(MAX_SERVICE_NAME_LENGTH + 1)];
#endif

#endif
//#ifdef RAPS_SUPPORT
    UINT16  internal_number;
    UINT16  unique_number;
    UINT16  crypt_type; //
    UINT16  pmc_pid;                                                //8
    UINT32  genre               : 8;
    UINT32  flag1               : 8;
    UINT32  flag2               : 8;
#ifdef POLAND_SPEC_SUPPORT
    UINT32  deleted                     : 1;
    UINT32  user_changed_service_name   : 1;   //service_name had been changed by user.
    UINT32  user_changed_lcn            : 1;   //LCN had been changed by user.
    UINT32  hidden_flag                 : 1;
    UINT32  reserved5                   : 4;

#else
    UINT32      reserved5               : 8;
#endif//4
    UINT8 audio_com_tag[MAX_AUDIO_CNT];                             //20

    UINT8 pnode_type;    // For flaging font-end type. Internal use.

#ifdef DB_SUPPORT_HMAC
    UINT8 v_hmac[HMAC_OUT_LENGTH];
#endif    
}P_NODE;

typedef UINT32 KEY_VALUE;

struct sort_t_info
{
    UINT32 value;
    DB_TP_ID id;
    UINT16 next_pos;
};

struct sort_p_info
{
    UINT16 node_pos;
    UINT16 next_pos;
};

enum
{
    DYNAMIC_PG_INITED   = 0x00,
    DYNAMIC_PG_STEAM_UPDATED = 0x01,
    DYNAMIC_PG_USER_MODIFIED = 0x02,
};

struct dynamic_prog_back
{
    UINT8 status;
    P_NODE prog;
};

#ifdef DB_SUPPORT_HMAC
enum
{
    MSG_HMAC_INIT = 1,
    MSG_HMAC_SAVENEW,
    MSG_HMAC_GETSAVE,
    MSG_HMAC_RESET,
    MSG_HMAC_NODE_FAILED,
    MSG_HMAC_HEAD_FAILED,
};
#endif

INT32 get_tp_at(UINT16 sat_id, UINT16 pos, T_NODE *node);
UINT16 get_tp_num_sat(UINT16 sat_id);
INT32 del_tp_on_sat(UINT16 sat_id);

INT32 get_sat_at(UINT16 pos, UINT16 select_mode, S_NODE *node);
INT32 get_sat_by_id(UINT16 sat_id, S_NODE *node);
UINT16 get_sat_num(UINT16 select_mode);
INT32 del_sat_by_id(UINT16 sat_id);
INT32 del_sat_by_pos(UINT16 pos);
INT32 recreate_sat_view(UINT16 create_mode, UINT32 param);
INT32 modify_sat(UINT16 sat_id, S_NODE *node);
UINT32 get_selected_sat_num();

INT32 get_prog_at(UINT16 pos, P_NODE *node);
INT32 del_prog_at(UINT16 pos);
INT32 get_specific_prog_num(UINT8 prog_flag,UINT16 *sat_v_cnt,UINT16 *sat_a_cnt,UINT16 *fav_v_cnt,UINT16 *fav_a_cnt);
INT32 move_prog(UINT16 d_pos, UINT16 s_pos);

BOOL db_same_node_checker(UINT8 n_type, void *old_node, void *new_node);
BOOL db_same_tpnode_checker(UINT8 n_type, void *old_node, void *new_node);

BOOL check_node_modified(UINT8 n_type);
INT32 move_prog(UINT16 d_pos,UINT16 s_pos);
INT32 update_stream_info(P_NODE *prev, P_NODE *next, INT32 *pos);

void get_default_bandparam(UINT8 index, band_param *buf);
UINT16 get_tp_num(UINT16 select_mode,UINT32 param);

INT32 node_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len);
INT32 node_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len,UINT8 *unpack_node, UINT32 unpack_len);
INT32 old_node_packer(UINT8 node_type, UINT8*src_node, UINT32 src_len,UINT8*packed_node,UINT32 *packed_len);
INT32 old_node_unpacker(UINT8 node_type,UINT8 *packed_node,UINT32 packed_len, UINT8 *unpack_node, UINT32 unpack_len);

INT32 recreate_sat_view_ext(UINT16 create_mode,UINT32 param,DB_VIEW *dest_view);

#ifdef DB_SUPPORT_HMAC
typedef INT32 (*db_notice_callback)(UINT32 msg_type, UINT32 msg_param);
void register_db_callback(db_notice_callback cb);
#endif

#ifdef __cplusplus
 }
#endif


#endif

