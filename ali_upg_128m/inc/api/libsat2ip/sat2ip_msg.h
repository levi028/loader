/** @file       sat2ip_msg.h
 *  @brief      Define datatypes that SAT2IP message communication module used, and declare APIs.
 *
 *  @copyright  ALi Corporation, All Rights Reserved.
 */

#ifndef _SAT2IP_MSG_H_
#define _SAT2IP_MSG_H_

#include <basic_types.h>

#ifdef SOCKET
#undef SOCKET
#endif
#define SOCKET              int

#ifdef INVALID_SOCKET
#undef INVALID_SOCKET
#endif
#define INVALID_SOCKET      (-1)

/** Sat2ip message listen port. */
#define SERVER_LISTEN_PORT          6000
/** Heartbeat timeout interval, Unit: second. */
#define HEARTBEAT_MAX_TIME          40
/** Message keyword string max length. */
#define MSG_STRING_LEN              128
/** Define Max message client support. */
#define MAX_MSG_CLIENT_SUPPORT      64

/** Define message keywords. */
#define SAT2IP_MSG_HEARTBEAT_KEYWORD                "heartbeat"
#define SAT2IP_MSG_FOLLOW_TV_KEYWORD                "follow_tv"
#define SAT2IP_MSG_SERVER_CLOSE_STREAM_KEYWORD      "server_close"
#define SAT2IP_MSG_STREAM_COUNT_KEYWORD             "stream_count"
#define SAT2IP_MSG_PVR_DIR_VALID_KEYWORD            "pvrdir_valid"
#define SAT2IP_MSG_PVR_DIR_INVALID_KEYWORD          "pvrdir_invalid"
#define SAT2IP_MSG_PVR_DIR_UPDATE_KEYWORD           "pvrdir_update"
#define SAT2IP_MSG_PVR_STREAMING_ITEM_DEL_KEYWORD   "pvritem_deleted"
#define SAT2IP_MSG_PVR_USB_SPEEDLOW_KEYWORD         "pvrusb_speedlow"
#define SAT2IP_MSG_USER_BEGIN_KEYWORD               "usermsg_begin"

typedef enum SAT2IP_MSG_ID
{
    /** Client -> Server, Use to keep alive to disconnect client in some case client no response. */
    SAT2IP_MSG_HEARTBEAT = 0,
    
    /** Client -> Server, Client want to follow STB current playing program. */
    SAT2IP_MSG_FOLLOW_TV,
    
    /** Server -> Client, server close the sat2ip stream. */
    SAT2IP_MSG_SERVER_CLOSE_STREAM,

    /** Server -> Client, SAT2IP+PVR2IP network streaming COUNT. */
    SAT2IP_MSG_STREAM_COUNT,
    
    /** Server -> Client, when one or more PVR directory sharing. */
    SAT2IP_MSG_PVR_DIR_VALID,

    /** Server -> Client, when no PVR directory sharing. */
    SAT2IP_MSG_PVR_DIR_INVALID,

    /** Server -> Client, when PVR directory change, the direcotry's record item number change. */
    SAT2IP_MSG_PVR_DIR_UPDATE,

    /** Server -> Client, when the PVR record item which streaming to the client was deleted. */
    SAT2IP_MSG_PVR_STREAMING_ITEM_DEL,

    /** Server -> Client, when read data from USB too low. maybe USB bandwidth not enough. */
    SAT2IP_MSG_PVR_USB_SPEEDLOW,
    
    /** if SDK user want to extend new message, please add msg type after this MSG */
    SAT2IP_MSG_USER_BEGIN,

    /** MAX of enum. */
    SAT2IP_MSG_UNDEF,
}SAT2IP_MSG_ID;

/** Define Sat2IP message callback function prototype, APP layer can register callback to message module. */
typedef RET_CODE (*SAT2IP_MSG_CALLBACK)(INT32 socket_id,INT32 msgid,UINT8 *keyword,UINT8 *msg_desc);

typedef enum COMM_CLIENT_STREAM_TYPE
{
    /** Mark the streaming is SAT2IP stream. */
    STREAM_TYPE_SAT2IP = 0,
    /** Mark the streaming is PVR2IP stream. */
    STREAM_TYPE_PVR2IP,
    /* Max of enum limit. */
    STREAM_TYPE_UNDEF,
} COMM_CLIENT_STREAM_TYPE;

typedef enum COMM_CLIENT_PROTO_TYPE
{
    /** Client use RTSP protocol playing stream. */
    CLIENT_PROTO_RTSP = 0,
    /** Clietn use HTTP protocol playing stream. */
    CLIENT_PROTO_HTTP,
    /** Max of enum limit. */
    CLIENT_PROTO_UNDEF,
} COMM_CLIENT_PROTO_TYPE;

/**
 *  @brief      Define structure to management sat2ip message client information.
 */
typedef struct sat2ip_comm_cilent
{
    /** Sat2ip message client IP address. */
    UINT32                  client_ip;
    /** Current playing stream's unique id. */
    UINT32                  stream_uid;
    /** Current playing stream type(SAT2IP/PVR2IP). */
    COMM_CLIENT_STREAM_TYPE stream_type;
    /** Current streaming network protocol type(RTSP/HTTP). */
    COMM_CLIENT_PROTO_TYPE  proto_type;
}sat2ip_comm_cilent;


/**
 *  @brief      Sat2IP server & client communication channel module initialize.
 *  @details    After initialize, all default message have been registered, user message need SDK user register again.
 *  @param[in]  msg_cb Message callback, when server receive message from client, will call this callback.
 *  @return     TRUE when success; FALSE if failture.
 */
BOOL sat2ip_comm_module_init(SAT2IP_MSG_CALLBACK msg_cb);

/**
 *  @brief      Uninitialize communication module.
 *  @return     TRUE if uninitialize success; FALSE if failture.
 */
BOOL sat2ip_comm_module_uninit(void);

/**
 *  @brief      Register message to communicate with client
 *  @details    when message module initialize, an set of default message already register,
 *              if SDK user want to add new message, need call this API to add into list.
 *  @param[in]  msg_id Message ID.
 *  @param[in]  keyword Message keyword string.
 *  @param[in]  p_cb Message process callback, when message come up from client, will call this callback.
 *  @return     TRUE if success; FALSE if failture.
 */
BOOL sat2ip_msg_register(SAT2IP_MSG_ID msg_id, UINT8 *keyword, SAT2IP_MSG_CALLBACK p_cb);

/**
 *  @brief      Unregister message from list, module will free relate resource.
 *  @param[in]  msg_id Message ID.
 *  @return     TRUE if sucess; FALSE if failture.
 */
BOOL sat2ip_msg_unregister(SAT2IP_MSG_ID msg_id);

/**
 *  @brief      Send message to client which identified by socket_id.
 *  @param[in]  socket_id The socket that after accept, use to communication with client.
 *  @param[in]  msg_id Message ID.
 *  @param[in]  msg_desc Message description, you can set to NULL if no description.
 *  @param[in]  msg_desc_len Message description string length, set to zero if no description string.
 *  @return     TRUE if send success; FALSE if send failture.
 */
BOOL sat2ip_comm_sendmsg_socketid(SOCKET socket_id, SAT2IP_MSG_ID msg_id, UINT8*msg_desc, UINT16 msg_desc_len);

/**
 *  @brief      Send message to SAT2IP client by SAT2IP stream id.
 *  @param[in]  stream_id SAT2IP stream id.
 *  @param[in]  msg_id Message ID.
 *  @param[in]  msg_desc Message description, you can set to NULL if no description.
 *  @param[in]  msg_desc_len Message description length, set to ZERO when no description string.
 *  @return     TRUE if success; FALSE if failture.
 */
BOOL sat2ip_comm_sendmsg_streamid(INT32 stream_id, SAT2IP_MSG_ID msg_id, UINT8*msg_desc, UINT16 msg_desc_len);

/**
 *  @brief      Send message to all client that connecting to server.
 *  @param[in]  msg_id Message ID.
 *  @param[in]  msg_desc Message description string.
 *  @param[in]  msg_desc_len Message description string length.
 *  @return     TRUE if success; FALSE if failture.
 */
BOOL sat2ip_comm_sendmsg_all(SAT2IP_MSG_ID msg_id, UINT8*msg_desc, UINT16 msg_desc_len);


/**
 *  @brief      Get current connected to server's client count.
 *  @return     The count of clients.
 */
UINT32 sat2ip_comm_get_client_count(void);

/**
 *  @brief      Get specified index client's information.
 *  @param[in]  idx Index from ZERO to sat2ip_comm_get_client_count.
 *  @param[out] info Client's information.
 *  @return     TRUE if success; FALSE if failture.
 */
BOOL sat2ip_comm_get_client_info(UINT32 idx, sat2ip_comm_cilent *info);

/**
 *  @brief      Get client's information by ip address.
 *  @param[in]  ip IP address of the client.
 *  @param[out] info The client's information.
 *  @return     TRUE if success; FALSE if failture.
 */
BOOL sat2ip_comm_get_client_info_by_ip(UINT32 ip, sat2ip_comm_cilent *info);

/**
 *  @brief      Add stream information to client when client play stream, add by RTSP server or HTTP server.
 *  @param[in]  ip Client's IP address.
 *  @param[in]  stype Stream type.
 *  @param[in]  ptype Protocol type.
 *  @return     TRUE if success; FALSE if failture.
 */
BOOL sat2ip_comm_stream_add(UINT32 ip, UINT32 uid, COMM_CLIENT_STREAM_TYPE stype, COMM_CLIENT_PROTO_TYPE ptype);

/**
 *  @brief      Delete stream information from client node when client stop play, call by RTSP server or HTTP server.
 *  @param[in]  ip Client's IP address.
 */
void sat2ip_comm_stream_del(UINT32 ip);

#endif /*_SAT2IP_MSG_H_*/

