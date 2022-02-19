/** @file       libprovider.h
 *  @brief      Define datatypes that SAT2IP media provider module used, and declare APIs.
 *
 *  @copyright  ALi Corporation, All Rights Reserved.
 */

#ifndef LIBPROVIDER_H
#define LIBPROVIDER_H

#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libfs2/stdio.h>
#include <api/libdb/db_return_value.h>
#include <api/libdb/db_config.h>
#include <api/libdb/db_node_combo.h>
#include <api/libdb/db_interface.h>
#include <api/libsat2ip/sat2ip.h>
#include <hld/dsc/dsc.h>

/** MAX PID number per one request. */
#define MAX_PID_NUM                         48
/** Max video pid number support per one request. */
#define VIDEO_MAX_PID   4

#ifdef MAX_CA_AUDIO_CNT
#define AUDIO_MAX_PID   MAX_CA_AUDIO_CNT
#else
/** Max audio pid number support per one request. */
#define AUDIO_MAX_PID   16
#endif
/** Max other pid number support per one request. */
#define OTHER_MAX_PID   16

/** Define invlaid crypto key position. */
#define PRV_INVALID_CRYPTO_KEY_POS          0xff
/** Define invalid crypto stream id. */
#define PRV_INVALID_CRYPTO_STREAM_ID        0xff
/** Define invalid crypto stream handle. */
#define PRV_INVALID_CRYPTO_STREAM_HANDLE    0xffffffff
/** Define invalid crypto mode. */
#define PRV_INVALID_CRYPTO_MODE             0xffffffff

/**
 *  @brief      Define stream state in media provider.
 *  @details    Server request play one stream by media provider, it is async mode request, provider set following
 *              state when in playing flow.
 */
typedef enum
{
    /** The stream already finish. */
    MEDIA_PROVIDER_STREAM_INFO_STATE_DEAD = 0,
    /** Media provider is configurating the playing stream. */
    MEDIA_PROVIDER_STREAM_INFO_STATE_GOON,
    /** Media provider play stream OK. */
    MEDIA_PROVIDER_STREAM_INFO_STATE_OK,
}STREAM_STATE_TYPE;

/**
 *  @brief      Define Media provider's command, use with API media_provider_io_cmd.
 *  @details    These command use by server, application layer no need these.
 */
typedef enum
{
    /** Start request and play one stream. media_provider_io_cmd param is pointer of struct sat2ip_stream_info,
     * affix_param is pointer of CA_CRYPT_INFO*/
    REQUEST_STREAM_START = 0,
    /** Update exist stream information, media_provider_io_cmd param is pointer of struct sat2ip_stream_info,
     * affix_param is pointer of CA_CRYPT_INFO*/
    REQUEST_STREAM_UPDATE,
    /** Stop streaming exist stream, only need stream_id, no other parameter need. */
    REQUEST_STREAM_STOP, 
    /** Read stream data from media provider. ### Currently no implement this command, read data use API:
     * media_provider_rpid_read_ring_buffer instead. */
    REQUEST_STREAM_DATA_READ,
    /** Check current play action's resutl/status, just need stream_id, other parameters no need.
     * return 0 is OK, others is failure. */
    REQUEST_CHECK_STREAM_STATUS,
    /** No implementation. */
    REQUEST_CHECK_PARAMETER_VALID,
    /** No need anymore, ########should be deleted. */
    REQUEST_CHECK_GET_PROGRAM_STREAM_FILE_PATH,
    /** No need anymore, ########should be deleted. */
    REQUEST_CHECK_GET_PID_STREAM_FILE_PATH,
    /** Command use to request all channel list in database, need provide param: PIPE_RW type function pointer. */
    REQUEST_SERVER_DB_INFOR,
    /** Stop generate and send out channel list. */
    REQUEST_SERVER_DB_ABORT,
    /** Get provider current status, include state: STREAM_STATE_TYPE, and return code: RETURN_REQUEST.
     * param is pointer of STREAM_STATE_TYPE var, affix_param is pointer of RETURN_REQUEST var. */
    REQUEST_CHECK_PROVIDER_STATUS,
}REQ_STREAM_CMD;


/**
 *  @brief      Define the structure to manage CA information from client request.
 */
typedef struct CA_CRYPT_INFO
{
    INT32   vid_num;    /**< Video pid count in vid_pids array. */
    INT32   aud_num;    /**< Audio pid count in aud_pids array. */
    INT32   ttx_num;    /**< Teletext pid count in ttx_pids array. */
    INT32   subt_num;   /**< Subtitle pid count in subt_pids array. */
    UINT16  vid_pids[VIDEO_MAX_PID];    /**< Video pid array from client request QUERY. */
    UINT16  aud_pids[AUDIO_MAX_PID];    /**< Audio pid array from client request QUERY. */
    UINT16  ttx_pids[OTHER_MAX_PID];    /**< Teletext pid array from client request QUERY. */
    UINT16  subt_pids[OTHER_MAX_PID];   /**< Subtitle pid array from client request QUERY. */
    UINT16  pmt_pid;    /**< Program's PMT pid value from client request QUERY. */
    UINT16  prog_number;/**< Program number(service id) from client request QUERY. */
    UINT8   ca_mode;    /**< CA mode flag from client request QUERY. */
}CA_CRYPT_INFO;

/**
 *  @brief      Define the structure to manage information for recrypto.
 */
typedef struct PRO_ENCRYPT_INFO
{
    CA_CRYPT_INFO   ca_info;    /**< Request stream's CA information. */
    UINT32          crypt_stream_id;    /**< Crypto engine used stream id. */
    UINT32          crypto_stream_hnd;  /**< Crypto engine assigned stream handle for stream. */
    UINT32          crypto_key_pos;     /**< Crypto stream key pos used by provider. */
    DEEN_CONFIG     pvr_reencrypt_config;   /**< Crypto engine configuration parameter. */
    INT32           state;                  /**< 0: un-initialize,1:initiallize free,2:used. */
}PRO_ENCRYPT_INFO;

/**
 *  @brief      Sat2IP stream information structure, store all information about sat2ip stream.
 *  @details    This structure use by server and media provider, some information provide by server,
 *              and some information set by media provider.
 */
struct sat2ip_stream_info
{
    STREAM_RECORD_TYPE  record_type;        /**< Server set, the stream filter type. */
    struct sat2ip_sat   sat_src;            /**< Server set, Satellite information from request. */
    struct sat2ip_tp    tp;                 /**< Server set, TP information from request. */
    INT32               pid_num;            /**< Server set, total pid count from request. */
    UINT16              pids[MAX_PID_NUM];  /**< Server set, PID list from request. */
    BOOL                is_scramble;        /**< Server set, Is program scamble flag. */

    UINT32              pvr_rec_handle;     /**< Provider set, handle of sat2ip stream filter. */
    UINT32              ts_route_id;        /**< Provider set, the id of TS ROUTE using. */
    INT32               stream_id_inter;    /**< Provider set, The same to stream id. */
    INT32               state;              /**< Provider set, value of STREAM_STATE_TYPE. */
    UINT8               ret_file_dir[1024]; /**< Provider set, ####### should be deleted. */
    UINT32              para_valid;         /**< Provider set, no used, #######should be deleted. */
    struct dmx_device   *dmx_dev;
    PRO_ENCRYPT_INFO    *crypt_info;
    BOOL                failure;            /**< Provider set, no used, ####### should be deleted. */
    INT32               type;               /**< Provider set, no used, ####### should be deleted. */
    RETURN_REQUEST      ret_err_code;       /**< Provider set, return code of command execute result. */
    void                *debug_info;        /**< Reversed for debug. */
    void                *ringbuf;           /**< Provider set, The ring buffer that store stream data from DMX. */
    void                *priv;              /**< Reversed. */
    INT32               socket_fd;          /**< Server set, Client socket id, but no use anymore. */
};

/**
 *  @brief      Define the event type of PVR directory update.
 *  @details    When PVR directory has something change, it should notify PVR2IP module, to update shared PVR list,
 *              here define the update event types.
 */
typedef enum
{
    /** One new PVR record finished in shared PVR directory. */
    PVR2IP_DIR_UPDATE_ADD = 0,
    /** Some PVR records have been deleted in shared PVR directory. */
    PVR2IP_DIR_UPDATE_DELETE,
    /** PVR record has been renamed in shared PVR directory. */
    PVR2IP_DIR_UPDATE_RENAME,
    /** Max type number, don't use it. */
    PVR2IP_DIR_UPDATE_UNDEF,
}PVR2IP_DIR_UPDATE_TYPE;

/**
 *  @brief      PVR record information for PVR2IP.
 */
typedef struct _pvr2ip_record_info
{
    UINT32      record_size;    /**< Record total size(Byte). */
    UINT32      duration;       /**< Record's duration. */
    UINT32      ts_bitrate;     /**< Record's bitrate. */
}pvr2ip_record_info;

/** Sat2IP stream information structure. */
typedef struct sat2ip_stream_info  S2IP_M_INFO;
/** Pointer type of Sat2IP stream information structure*/
typedef struct sat2ip_stream_info* PS2IP_M_INFO;

/** Prototype of channel list write out function. */
typedef int (*PIPE_RW) (INT32 id,unsigned char * buffer_in,unsigned int siz);
/** Prototype of application layer callback which register to media provider. */
typedef void (*AP_CALLBACK)(UINT32 event_type,UINT32 para1,UINT32 para2,BOOL sync);
/**< Prototype of request ring buffer function. */
typedef UINT32 (*REQ_BUFFER)(UINT32 req_size,UINT32 *addr);
/**< Prototype of free ring buffer function. */
typedef  void  (*FREE_BUFFER)(UINT32 addr);

/**
 *  @brief      Media provider module initialize API, it should be the first called API of media provider.
 *  @param[in]  encrypt_enable Is support content encrypto.
 */
void media_provider_init(BOOL encrypt_enable);

/**
 *  @brief      Media provider module uninitialize API.
 */
void media_provider_uninit(void);

/**
 *  @brief      Media provider ring buffer setting API.
 *  @details    Media provider need allocate ring buffer for every stream, use to recieve ts data from DMX.
 *              This use to set total ring buffer to media provider, and configurate how many stream support,
 *              every stream's ring buffer size is length/buf_num.
 *  @param[in]  ring_buffer The total ring buffer address.
 *  @param[in]  length The total ring buffer length.
 *  @param[in]  buf_num The max sat2ip stream support.
 *  @param[in]  use_extern Please always set TRUE.
 *  @return     void.
 */
void media_provider_buffer_init(UINT8 *ring_buffer,INT32 length,INT8 buf_num,BOOL use_extern);

/**
 *  @brief      Register application layer callback to media provider.
 *  @details    The event from media provider can reference SAT2IP_ACTION_TYPE.
 *  @param[in]  callback The application layer callback.
 *  @return     void.
 */
void media_provider_register_ap_callback(AP_CALLBACK callback);

/**
 *  @brief      Get playing stream's state.
 *  @param[in]  stream_id The stream's ID.
 *  @return     reference STREAM_STATE_TYPE.
 *  @see        STREAM_STATE_TYPE
 */
INT32 media_provider_get_stream_state(INT32 stream_id);

/**
 *  @brief      Media provider command api, call by server.
 *  @param[in]  stream_id The operate stream id.
 *  @param[in]  cmd The command going to execute.
 *  @param[in]  param The first parameter for command.
 *  @param[in]  affix_param The option param for command.
 *  @return     0 when success; other value failure.
 *  @see        REQ_STREAM_CMD
 */
INT32 media_provider_io_cmd(INT32 stream_id,REQ_STREAM_CMD cmd,UINT32 param,UINT32 affix_param);

/**
 *  @brief      Read ts stread data from media provider's ring buffer.
 *  @param[in]  stream_id The stream's id.
 *  @param[out] buffer The buffer carry ts stream out.
 *  @param[in]  size The buffer's size.
 *  @return     0 success; other value failure.
 */
INT32 media_provider_rpid_read_ring_buffer(INT32 stream_id,UINT8 *buffer,INT32 size);

/**
 *  @brief      Start configurate DMX filter out stream, call by application layer after configurate CA module if need.
 *  @param[in]  stm_info Request stream information from client.
 *  @return     0 success; other failure.
 */
INT32 media_provider_record_pid_start(struct sat2ip_stream_info *stm_info);

/**
 *  @brief      Stop DMX filter out stream.
 *  @details    Current implementation call by media provider internal when stop play.
 *  @param[in]  stm_info Playing stream information.
 *  @return     0 success; other failure.
 */
INT32 media_provider_record_pid_stop(struct sat2ip_stream_info *stm_info);

/**
 *  @brief      Set the frontend number current STB have, application layer set.
 *  @param[in]  fe_count The frontend number.
 *  @return     void.
 */
void media_provider_set_fe_count(INT32 fe_count);

/**
 *  @brief      Get the frontend number STB have.
 *  @return     The number of frontend.
 */
INT32 media_provider_get_fe_count(void);

/**
 *  @brief      Get stream's signal quality.
 *  @param[in]  stream_id The checking stream id.
 *  @param[out] quality The signal quality of stream.
 *  @return     0 success; other failure.
 */
INT32 media_provider_get_fe_snr(INT32 stream_id,UINT32 *quality);

/**
 *  @brief      Check the specifid stream lock signal or not.
 *  @param[in]  stream_id The checking stream's id.
 *  @param[out] lock Is signal lock: 0 unlock; 1 locked.
 *  @return     0 success; other failure.
 */
INT32 media_provider_get_fe_lock(INT32 stream_id,UINT8 *lock);

/**
 *  @brief      Get the specified stream's agc level.
 *  @param[in]  stream_id The checking stream's id.
 *  @param[out] level The agc level value.
 *  @return     0 success; other failure.
 */
INT32 media_provider_get_fe_agc(INT32 stream_id,UINT32 *level);

/**
 *  @brief      Get the specified stream's bitrate.
 *  @param[in]  stream_id The checking stream's id.
 *  @return     The stream's bitrate value.
 */
UINT32 media_provider_get_stream_bitrate(UINT32 stream_id);

/**
 *  @brief      Retrieve the specified stream's information.
 *  @param[in]  stream_id The stream's id.
 *  @return     The stream information's pointer.
 */
struct sat2ip_stream_info* media_provider_get_info_by_id(INT32 stream_id);

/**
 *  @brief      Retrieve the specified ts route's stream information.
 *  @param[in]  route_id The stream using ts route's id.
 *  @return     The stream information's pointer.
 */
struct sat2ip_stream_info*media_provider_get_info_by_route_id(UINT32 route_id);

/**
 *  @brief      Translate the return code to string.
 *  @param[in]  ret_code The media provider's request return code.
 *  @return     The pointer to corresponding string.
 *  @see        RETURN_REQUEST
 */
UINT8 *media_provider_retid_to_str(RETURN_REQUEST ret_code);

/** Define the handle of record that open by PVR2IP. */
typedef UINT32 PVR2IP_HANDLE;
/** Define the invalid handle value of PVR2IP_HANDLE. */
#define PVR2IP_INVALID_HANDLE 0xFFFFFFFF

/**
 *  @brief      Initialize PVR Provider module.
 *  @details    Client can try to get pvr list and play after initialize. Application layer call this API when
 *              system startup, or UI setting enable PVR2IP feature.
 *  @return     TRUE when init success; FALSE when failure.
 */
BOOL media_provider_pvr_init(void);

/**
 *  @brief      Disable PVR Provider module, after call this API, client can not download pvr list any more.
 *  @return     TRUE when clean up success; FALSE when clean up failure.
 */
BOOL media_provider_pvr_clean_up(void);

/**
 *  @brief      Check is PVR provider module is enable or not.
 *  @return     TRUE module enable; FALSE module disable.
 */
BOOL media_provider_pvr_is_enable(void);

/**
 *  @brief      Add PVR direcotry that going to be shared.
 *  @details    After add path, only ALI PVR record item can be shared, other media file won't be share to client.
 *  @param[in]  dir_path full path of the directory going to share. e.g. /mnt/uda1/ALIDVRS2
 *  @return     RET_SUCCESS when success; RET_FAILURE when failure.
 */
RET_CODE media_provider_pvr_add_dir(const char *dir_path);

/**
 *  @brief      Delete shared PVR directory.
 *  @param[in]  dir_path full path of the PVR directory. e.g. /mnt/uda1/ALIDVRS2
 *  @return     RET_SUCCESS when success; RET_FAILURE when directory not exist in list or other reason.
 */
RET_CODE media_provider_pvr_del_dir(const char *dir_path);

/**
 *  @brief      Notify pvr provider to update shared pvr direcotry information.
 *  @details    When STB finish new record or delete record item, should call this API to update the list.
 *  @param[in]  dir_path full path of the PVR directory. e.g. /mnt/uda1/ALIDVRS2
 *  @param[in]  type ADD, DEL, REM
 *  @return     RET_SUCCESS when update success; RET_FAILURE when failure.
 */
RET_CODE media_provider_pvr_update_dir(const char *dir_path, PVR2IP_DIR_UPDATE_TYPE type);

/**
 *  @brief      Add one record item to share list.
 *  @param[in]  rec_path Full path of the record item.
 *  @return     RET_SUCCESS if success; RET_FAILTURE if failture.
 */
RET_CODE media_provider_pvr_add_rec(const char *rec_path);

/**
 *  @brief      Delete one record item from share list.
 *  @param[in]  rec_path Full path of the record item.
 *  @return     RET_SUCCESS if success; RET_FAILTURE if failture.
 */
RET_CODE media_provider_pvr_del_rec(const char *rec_path);

/**
 *  @brief      This API use to check is there any record item in the specified directory streaming.
 *  @param[in]  dir_path full path of the PVR directory. e.g. /mnt/uda1/ALIDVRS2
 *  @return     TRUE there are some record item is streaming; FALSE no recording is streaming.
 */
BOOL media_provider_pvr_dir_is_streaming(const char *dir_path);

/**
 *  @brief      This API use to check is the specified record item is streaming to client.
 *  @param[in]  rec_path full path of the record item(record item's directory path).
 *  @return     TRUE the record item is streaming to client; FALSE the record item is not streaming.
 */
BOOL media_provider_pvr_rec_is_streaming(const char *rec_path);

/**
 *  @brief      Get current how many PVR directory shared.
 *  @return     The number of direcotry sharing.
 */
UINT32 media_provider_pvr_get_dir_count(void);

/**
 *  @brief      This API use to get directory name that in share list by index.
 *  @param[in]  index The index in share list, from 0 to count that get from API media_provider_pvr_get_dir_count.
 *  @param[out] dir_path The buffer to get direcotry path name.
 *  @param[in]  dir_path_len The buffer length.
 *  @return     RET_SUCCESS get directory success; RET_FAILURE get directory failure.
 */
RET_CODE media_provider_pvr_get_dir_name(UINT32 index, char *dir_path, UINT32 dir_path_len);

/**
 *  @brief      This API use to get and send out PVR list, call by HTTP server.
 *  @param[in]  id The send out fifo id pass from HTTP server.
 *  @param[in]  send_out_cb The function the send data out, pass from HTTP server.
 *  @return     RET_SUCCESS get pvr list success; RET_FAILURE get pvr list failure.
 */
RET_CODE media_provider_pvr_get_list(INT32 id, PIPE_RW send_out_cb);

/**
 *  @brief      This API use open record item that going to streaming.
 *  @details    This API use open record item that going to streaming, call by server. server
 *              can read data after open success.
 *  @param[in]  record_uid Every record item will be allocate one unique id when client get pvr list.
 *  @return     Valid handle when open success; Invalid handle value when open failure.
 */
PVR2IP_HANDLE media_provider_pvr_record_open(UINT32 record_uid);

/**
 *  @brief      Close record item that streaming. server can not read data after close.
 *  @param[in]  handle The return handle when call media_provider_pvr_record_open
 *  @return     TRUE close success; FALSE close failure.
 */
BOOL media_provider_pvr_record_close(PVR2IP_HANDLE handle);

/**
 *  @brief      Read record item data. Call by server.
 *  @param[in]  handle The return handle when call media_provider_pvr_record_open
 *  @param[out] buf The buffer use to fill reading data.
 *  @param[in]  size The buffer size.
 *  @return     Data size read success.
 */
UINT32 media_provider_pvr_record_read(PVR2IP_HANDLE handle,UINT8 *buf,UINT32 size);

/**
 *  @brief      Seek to the specified time.
 *  @param[in]  handle The return handle when call media_provider_pvr_record_open
 *  @param[in]  second The time going to seek to.
 *  @return     TRUE seek success; FALSE seek failure.
 */
BOOL media_provider_pvr_time_seek(PVR2IP_HANDLE handle, UINT32 second);

/**
 *  @brief      Get specified record item's information from pvr provider.
 *  @param[in]  uid The unique id of record item.
 *  @param[out] info Record's information.
 *  @return     TRUE when get success; FALSE when get failure.
 */
BOOL media_provider_pvr_get_rec_info(UINT32 uid, pvr2ip_record_info *info);

/**
 *  @brief      Get private key.
 *  @param[out] buf Output key buffer.
 *  @param[in]  buf_len Buffer length.
 *  @return     TRUE get success; FALSE get key failure.
 */
BOOL media_provider_pvr_get_user_key(UINT8 *buf, UINT32 buf_len);

/**
 *  @brief      Get specified PVR record item's bitrate.
 *  @param[in]  uid PVR2IP record item's unique ID.
 *  @return     Bitrate.
 */
UINT32 media_provider_pvr_get_stream_bitrate(UINT32 uid);

/**
 *  @brief      Check is there any sharing items.
 *  @return     TRUE if have some items; FALSE no share item.
 */
BOOL media_provider_pvr_has_share_item(void);

/**
 *  @brief      Configurate malloc and free buffer function.
 *  @param[in]  req_fun Function callback that request ring buffer.
 *  @param[in]  free_fun Function callback that free ring buffer.
 *  @return     void
 */
void media_provider_register_buffer_function(REQ_BUFFER req_fun,FREE_BUFFER free_fun);

/* media_provider_crypt_config_dsc_parse_res
 * 
 * configurate decrypt informaton.
 * @dmx_id      ts stream use which dmx.
 * @dec_dev     device of use decrypt 
 * @decrypt_mode    CSA/DES/AES,it defined dsc.h
 * @decrypt_stream_id       stream ID for current device.CA library using for
                            decrypt
 * return void
 */

INT32 media_provider_crypt_config_dsc_parse_res(UINT32 dmx_id,void *dec_dev,
    UINT32 decrypt_mode,
    UINT32 decrypt_stream_id);

#endif
